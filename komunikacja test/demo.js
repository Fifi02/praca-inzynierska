const { createClient } = supabase;

const db = createClient(
  'https://pbuavcvywholupjuhxmn.supabase.co', // Twój projekt Supabase URL
  'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InBidWF2Y3Z5d2hvbHVwanVoeG1uIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjE1Njg5NTgsImV4cCI6MjA3NzE0NDk1OH0.WyTiMk1T2rkSTYXarnBmyAAg3guDtB_Uu4K-Pr53-mc' // Twój klucz anon
);

// --- Funkcja zapisu danych MQTT do bazy ---
async function insertMQTTData(topic, payload) {
  try {
    const { data, error } = await db
      .from("temperatura")  // nazwa tabeli
      .insert([
        {
          stC: payload,
          created_at: new Date() // jeśli masz kolumnę timestamp
        }
      ]);

    if (error) {
      console.error("❌ Supabase insert error:", error);
      document.getElementById("messages").innerHTML += `<span style="color:red;">Błąd zapisu do bazy: ${error.message}</span><br>`;
    } else {
      console.log("✅ Dane zapisane:", data);
      document.getElementById("messages").innerHTML += `<span style="color:green;">Zapisano do bazy: ${topic} → ${payload}</span><br>`;
    }
  } catch (err) {
    console.error("Unexpected error:", err);
    document.getElementById("messages").innerHTML += `<span style="color:red;">Błąd: ${err.message}</span><br>`;
  }
}

// --- MQTT konfiguracja i funkcje ---

function startConnect() {
  // Losowy client ID
  clientID = "clientID-" + parseInt(Math.random() * 100);

  // Parametry połączenia
  host = document.getElementById("host").value;   
  port = document.getElementById("port").value;  
  userId = document.getElementById("username").value;  
  password = document.getElementById("password").value;  

  // Komunikat statusu
  document.getElementById("messages").innerHTML += `<span>Łączenie z ${host}:${port}</span><br>`;
  document.getElementById("messages").innerHTML += `<span>Client ID: ${clientID}</span><br>`;

  // Utworzenie klienta MQTT
  client = new Paho.MQTT.Client(host, Number(port), clientID);

  // Callbacki
  client.onConnectionLost = onConnectionLost;
  client.onMessageArrived = onMessageArrived;

  // Połączenie z brokerem
  client.connect({
    onSuccess: onConnect,
    userName: userId,
    password: password,
    useSSL: true // SSL wymagany przy portach 8883 / 8884
  });
}

function onConnect() {
  topic = document.getElementById("topic_s").value;
  document.getElementById("messages").innerHTML += `<span>Subskrypcja tematu: ${topic}</span><br>`;
  client.subscribe(topic);
}

function onConnectionLost(responseObject) {
  document.getElementById("messages").innerHTML += `<span style="color:red;">❌ Połączenie utracone.</span><br>`;
  if (responseObject != 0) {
    document.getElementById("messages").innerHTML += `<span>ERROR: ${responseObject.errorMessage}</span><br>`;
  }
}

function onMessageArrived(message) {
  console.log("Wiadomość:", message.payloadString);
  document.getElementById("messages").innerHTML += `<span>📩 Temat: ${message.destinationName} | Wiadomość: ${message.payloadString}</span><br>`;
  insertMQTTData(message.destinationName, message.payloadString);
}

function startDisconnect() {
  client.disconnect();
  document.getElementById("messages").innerHTML += "<span>🔌 Rozłączono.</span><br>";
}

function publishMessage() {
  msg = document.getElementById("Message").value;
  topic = document.getElementById("topic_p").value;

  Message = new Paho.MQTT.Message(msg);
  Message.destinationName = topic;

  client.send(Message);
  document.getElementById("messages").innerHTML += `<span>📤 Wysłano wiadomość do ${topic}</span><br>`;
}