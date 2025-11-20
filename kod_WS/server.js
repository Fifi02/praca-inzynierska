const WebSocket = require('ws');
const mysql = require('mysql2/promise');
const http = require('http');

(async () => {
  const db = await mysql.createConnection({
    host: "localhost",
    user: "root",
    password: "Dupa1234$",
    database: "iot_data"
  });

  // ————————————————————————————————————
  // HTTP SERVER (API + CORS)
  // ————————————————————————————————————
  const server = http.createServer(async (req, res) => {
    // CORS headers for all requests
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type");

    if (req.method === "OPTIONS") {
      res.writeHead(200);
      return res.end();
    }

    if (req.url === "/logs") {
      try {
        const [rows] = await db.execute(
          "SELECT * FROM logs ORDER BY id DESC LIMIT 50"
        );
        res.writeHead(200, { "Content-Type": "application/json" });
        return res.end(JSON.stringify(rows));
      } catch (err) {
        res.writeHead(500, { "Content-Type": "application/json" });
        return res.end(
          JSON.stringify({ error: "DB error", details: err.toString() })
        );
      }
    }

    res.writeHead(404, { "Content-Type": "text/plain" });
    res.end("Not Found");
  });

  server.listen(8081, () => {
    console.log("HTTP API server running on http://0.0.0.0:8081/logs");
  });

  // ————————————————————————————————————
  // WEBSOCKET SERVER
  // ————————————————————————————————————
  const wss = new WebSocket.Server({ port: 8080, host: '0.0.0.0' });
  console.log("WebSocket server running on ws://0.0.0.0:8080");

  // jeden ESP
  let espSocket = null;

  // zestaw stron WWW
  let webClients = new Set();

  // UWAGA: dodajemy drugi parametr: req
  wss.on('connection', (ws, req) => {
    const path = req.url || '/';
    console.log(`Nowe połączenie WebSocket, path = ${path}`);

    // ——————————————————————————
    // KLIENT ESP32
    // ——————————————————————————
    if (path === '/esp') {
      // jeśli już jest jakieś ESP podłączone, możemy stare rozłączyć
      if (espSocket && espSocket.readyState === WebSocket.OPEN) {
        console.log("⚠️ Nowe ESP – poprzednie zostanie rozłączone");
        espSocket.close(1000, 'New ESP connected');
      }

      espSocket = ws;
      console.log("📡 ESP32 connected!");

      ws.on('close', () => {
        console.log("📡 ESP32 disconnected");
        if (espSocket === ws) {
          espSocket = null;
        }
      });
    }

    // ——————————————————————————
    // KLIENT WWW
    // ——————————————————————————
    else if (path === '/web') {
      webClients.add(ws);
      console.log("🖥️ WWW client connected!");

      ws.on('close', () => {
        console.log("🖥️ WWW client disconnected");
        webClients.delete(ws);
      });
    }

    // ——————————————————————————
    // Nieznany typ klienta
    // ——————————————————————————
    else {
      console.log("❌ Nieznany typ klienta, zamykam połączenie");
      ws.close(1008, 'Unknown client type');
      return;
    }

    // ——————————————————————————
    // Obsługa wiadomości
    // ——————————————————————————
    ws.on('message', async (message) => {
      const value = message.toString();
      console.log("Received:", value, "from path:", path);

      // Zapis do bazy – nie zależy skąd przyszło
      try {
        await db.execute(
          "INSERT INTO logs(value) VALUES(?)",
          [value]
        );
      } catch (e) {
        console.error("DB error:", e);
      }

      // 1. Wiadomość od ESP → wyślij do WWW
      if (path === '/esp') {
        const payload = JSON.stringify({
          type: "new_value",
          value: value
        });

        for (let client of webClients) {
          if (client.readyState === WebSocket.OPEN) {
            client.send(payload);
          }
        }
      }

      // 2. Wiadomość od WWW → wyślij do ESP
      else if (path === '/web') {
        if (espSocket && espSocket.readyState === WebSocket.OPEN) {
          espSocket.send(value);
        } else {
          ws.send(JSON.stringify({
            type: "error",
            error: "ESP not connected"
          }));
        }
      }
    });
  });

})(); // IIFE