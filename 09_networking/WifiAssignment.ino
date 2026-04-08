#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "MAKERSPACE";
const char* password = "12345678";

const String dogEndpoint = "https://dogapi.dog/api/v1/facts?number=1";
const String backendEndpoint = "http://192.168.0.163:5050/generate-haiku";

WebServer server(80);

String currentFact = "Click “Get Dog Fact” to begin!";
String currentHaiku = "";

void fetchDogFact() {
  HTTPClient http;
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.begin(dogEndpoint);

  int code = http.GET();

  if (code > 0) {
    String payload = http.getString();
    Serial.println("Dog fact response:");
    Serial.println(payload);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      const char* fact = doc["facts"][0];
      if (fact) {
        currentFact = String(fact);
        currentHaiku = "";  // clear old haiku when new fact arrives
      } else {
        currentFact = "No dog fact found.";
      }
    } else {
      Serial.print("Dog fact JSON parse failed: ");
      Serial.println(error.c_str());
      currentFact = "Failed to parse dog fact.";
    }
  } else {
    Serial.print("Dog fact request failed, code: ");
    Serial.println(code);
    currentFact = "Failed to fetch dog fact.";
  }

  http.end();
}

void generateHaiku() {
  HTTPClient http;
  http.begin(backendEndpoint);
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument req(1024);
  req["fact"] = currentFact;

  String body;
  serializeJson(req, body);

  int code = http.POST(body);

  if (code > 0) {
    String payload = http.getString();
    Serial.println("Haiku response:");
    Serial.println(payload);

    DynamicJsonDocument resp(4096);
    DeserializationError error = deserializeJson(resp, payload);

    if (!error) {
      const char* haiku = resp["haiku"];
      if (haiku) {
        currentHaiku = String(haiku);
      } else {
        currentHaiku = "No haiku returned.";
      }
    } else {
      Serial.print("Haiku JSON parse failed: ");
      Serial.println(error.c_str());
      currentHaiku = "Failed to parse haiku.";
    }
  } else {
    Serial.print("Haiku request failed, code: ");
    Serial.println(code);
    currentHaiku = "Failed to generate haiku.";
  }

  http.end();
}

void handleGetFact() {
  fetchDogFact();

  DynamicJsonDocument doc(2048);
  doc["fact"] = currentFact;
  doc["haiku"] = currentHaiku;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleGenerateHaiku() {
  generateHaiku();

  DynamicJsonDocument doc(4096);
  doc["fact"] = currentFact;
  doc["haiku"] = currentHaiku;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Dog Fact + Haiku Machine</title>
<style>
  body {
    margin: 0;
    font-family: "Trebuchet MS", Arial, sans-serif;
    background: linear-gradient(135deg, #fff4e6, #ffe9f3, #eef7ff);
    color: #333;
    text-align: center;
    padding: 24px;
  }

  .container {
    max-width: 850px;
    margin: 0 auto;
  }

  .card {
    background: rgba(255,255,255,0.92);
    border-radius: 24px;
    padding: 28px;
    box-shadow: 0 10px 30px rgba(0,0,0,0.10);
  }

  h1 {
    margin-top: 0;
    font-size: 2rem;
    color: #5a3d2b;
  }

  .subtitle {
    color: #7a6a5c;
    margin-bottom: 24px;
    font-size: 1.05rem;
  }

  .fact-box {
    background: #fffaf0;
    border: 3px dashed #ffb86b;
    border-radius: 20px;
    padding: 22px;
    margin: 20px 0;
  }

  .fact-label {
    font-size: 0.95rem;
    font-weight: bold;
    color: #c26a00;
    margin-bottom: 10px;
    text-transform: uppercase;
    letter-spacing: 1px;
  }

  .fact {
    font-size: 1.35rem;
    line-height: 1.6;
    color: #4a4037;
  }

  .haiku-box {
    background: #f8f4ff;
    border: 3px solid #d7c3ff;
    border-radius: 20px;
    padding: 22px;
    margin-top: 24px;
  }

  .haiku-label {
    font-size: 0.95rem;
    font-weight: bold;
    color: #6a43c1;
    margin-bottom: 12px;
    text-transform: uppercase;
    letter-spacing: 1px;
  }

  .haiku {
    white-space: pre-wrap;
    font-family: Georgia, serif;
    font-size: 1.25rem;
    line-height: 1.8;
    color: #43345f;
    min-height: 80px;
  }

  .buttons {
    display: flex;
    gap: 14px;
    justify-content: center;
    flex-wrap: wrap;
    margin-top: 22px;
  }

  button {
    border: none;
    border-radius: 999px;
    padding: 14px 22px;
    font-size: 1rem;
    font-weight: bold;
    cursor: pointer;
    transition: transform 0.15s ease, box-shadow 0.15s ease, opacity 0.15s ease;
    box-shadow: 0 6px 14px rgba(0,0,0,0.10);
  }

  button:hover {
    transform: translateY(-2px);
  }

  button:active {
    transform: translateY(0);
  }

  .fact-btn {
    background: #ffb347;
    color: white;
  }

  .haiku-btn {
    background: #9b7bff;
    color: white;
  }

  .status {
    margin-top: 18px;
    color: #666;
    min-height: 24px;
    font-size: 0.95rem;
  }

  .paw {
    font-size: 1.4rem;
  }
</style>
</head>
<body>
  <div class="container">
    <div class="card">
      <h1>🐶 Dog Fact + Haiku Machine 🐾</h1>
      <div class="subtitle">Fetch a fun dog fact, then turn it into a tiny poem.</div>

      <div class="fact-box">
        <div class="fact-label">Fresh Dog Fact</div>
        <div id="fact" class="fact">FACT_TEXT</div>
      </div>

      <div class="buttons">
        <button class="fact-btn" onclick="getFact()">🐕 Get Dog Fact</button>
        <button class="haiku-btn" onclick="makeHaiku()">✨ Make Haiku</button>
      </div>

      <div id="status" class="status"></div>

      <div class="haiku-box">
        <div class="haiku-label">Dog Fact Haiku</div>
        <div id="haiku" class="haiku">HAIKU_TEXT</div>
      </div>
    </div>
  </div>

<script>
function getFact() {
  document.getElementById("status").innerText = "Fetching a delightful dog fact...";
  fetch("/get-fact")
    .then(res => res.json())
    .then(data => {
      document.getElementById("fact").innerText = data.fact;
      document.getElementById("haiku").innerText = data.haiku || "";
      document.getElementById("status").innerText = "Dog fact acquired! 🐾";
    })
    .catch(err => {
      document.getElementById("status").innerText = "Could not fetch dog fact.";
    });
}

function makeHaiku() {
  document.getElementById("status").innerText = "Turning fact into poetry...";
  fetch("/make-haiku")
    .then(res => res.json())
    .then(data => {
      document.getElementById("fact").innerText = data.fact;
      document.getElementById("haiku").innerText = data.haiku;
      document.getElementById("status").innerText = "Haiku complete ✨";
    })
    .catch(err => {
      document.getElementById("status").innerText = "Could not generate haiku.";
    });
}
</script>
</body>
</html>
)rawliteral";

  html.replace("FACT_TEXT", currentFact);
  html.replace("HAIKU_TEXT", currentHaiku);

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("Connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/get-fact", handleGetFact);
  server.on("/make-haiku", handleGenerateHaiku);

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}