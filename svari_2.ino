#include "WiFi.h"
#include "ESPAsyncWebSrv.h"
#include <PubSubClient.h>
#include <PubSubClient.h>

#define BUTTON_PIN 21 

const char* ssid = "MZVF";
const char* password = "S@!3g7kW";
const char* mqtt_server = "192.168.1.32";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

int lastState = LOW; 
int currentState;     
int randNumber;

AsyncWebServer server(80);

String svars() {
  float t = randNumber;
//  Serial.println(t);
  return String(t);
  }

String processor(const String& var){  
  var == "TEMPERATURE";  
  return String();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1" charset="UTF-8">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
 button {
        background-color: rgb(180, 219, 187);
        font-size: 40px;
        padding: 15px 32px;
        border-radius: 15px;
    }    
  </style>
</head>

<body>
    <h2>makets Svari</h2>
    <p>
       
      <span class="dht-labels">Svars:</span> 
      <span id="temperature">%TEMPERATURE%</span>
      <span class="dht-labels">grami</span>
    </p>

    <button id="myButton" onclick="fetch('/buttonPressed')">Reģistrēt</button>
  </body>

  <script>
    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
        document.getElementById("temperature").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "/temperature", true);
    xhttp.send();
    }, 1000 ) ;
  </script>

  <script>  
    var button = document.getElementById("myButton"); 
    button.addEventListener("click", function() {  
        alert("Svars reģstrēts!");
        client.publish("esp32/temperature", tempString);
        randNumber = 0;   
    });

</script>

    </html>)rawliteral";

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP); //simulators

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
}
 Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883); //mqtt clients
  client.setCallback(callback);


//web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", svars().c_str());
  });

  server.on("/buttonPressed", HTTP_GET, [](AsyncWebServerRequest *request){
  Serial.println("button");
  request->send(200, "text/plain", "Button was pressed");
  client.publish("esp32/temperature", svars().c_str());
  randNumber = 0;  
  });

  server.begin();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void buttonPress() {
  currentState = digitalRead(BUTTON_PIN);
  
  if (lastState == HIGH && currentState == LOW)
    randNumber = random(10, 40);
  else if (lastState == LOW && currentState == HIGH)
    Serial.println("The button is released");
  lastState = currentState;
}

void loop() {
buttonPress();

if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
