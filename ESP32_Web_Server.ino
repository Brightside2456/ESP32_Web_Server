#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Network Credentials
const char* ssid = "Bright";
const char* password = "12345678";
#define echopin 27
#define trigpin 26
int receivedValue = 0;

int distance; // Declare the distance variable

int getDistance() {
  digitalWrite(trigpin, LOW);
  delay(10);
  digitalWrite(trigpin, HIGH);
  delay(10);
  digitalWrite(trigpin, LOW);
  int vals = pulseIn(echopin, HIGH);
  int distance = (vals * 0.034) / 2;
  return distance;
}

// Create an AsyncWebServer (Create the webserver object) object on port 80
AsyncWebServer server(80);
// Create the event source with name /events
AsyncEventSource events("/events");

String processor(const String& var) {
  distance = getDistance();
  if (var == "SPRAY") {
    return String(distance);
  }
  else if (var == "BATTERY") {
    return String(distance);
  }
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

// Declare the HTML content as a PROGMEM (program memory) variable
const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
<html>
<head>
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-9ndCyUaIbzAi2FUVXJi0CjmCapSmO7SnpJef0486qhLnuZ2cdeRhO02iuK6FUUVM" crossorigin="anonymous">
  <link href="/bootstrap.min.css" rel="stylesheet" integrity="sha384-9ndCyUaIbzAi2FUVXJi0CjmCapSmO7SnpJef0486qhLnuZ2cdeRhO02iuK6FUUVM" crossorigin="anonymous">
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
</head>
<body>
  <div class="modal modal-sheet position-static d-block bg-body-secondary p-4 py-md-5 align-items-center" tabindex="-1" role="dialog" id="modalSheet">
    <div class="modal-title text-center">
      <h1 class="display-5 fw-bold text-body-emphasis text-center">Air Refresher</h1>
    </div>  
    <div class="modal-dialog row" role="document">
      <div class="modal-dialog col" role="document">
        <div class="modal-content rounded-4 shadow">
          <div class="modal-header border-bottom-0">
            <h1 class="modal-title text-centre fs-5 text-center">Spray Percentage</h1>
          </div>
          <div class="modal-footer flex-column align-items-stretch w-100 gap-2 pb-3 border-top-0">
            <h1 class="modal-title text-centre fs-5 text-center"><span id="spray">%SPRAY%</span>&percnt;</h1>
          </div>
        </div>
      </div>
      <div class="modal-dialog col" role="document">
        <div class="modal-content rounded-4 shadow">
          <div class="modal-header border-bottom-0">
            <h1 class="modal-title text-centre fs-5 text-center">Battery Percentage</h1>
          </div>
          <div class="modal-footer flex-column align-items-stretch w-100 gap-2 pb-3 border-top-0">
            <h1 class="modal-title text-centre fs-5 text-center"><span id="battery">%BATTERY%</span>&percnt;</h1>
          </div>
        </div>
      </div>
      <div class="modal-dialog" role="document">
        <div class="modal-content rounded-4 shadow">
          <div class="modal-header border-bottom-0">
            <h1 class="modal-title text-centre fs-5 text-center">Spray Interval</h1>
          </div>
          <div class="modal-footer flex-column align-items-stretch w-100 gap-2 pb-3 border-top-0">
            <div class="row mb-3 text-center">
              <div class="col-sm-4 themed-grid-col">
                <button type="button" class="btn btn-lg btn-primary" onclick="setValue('10 min')">10 min</button>
              </div>
              <div class="col-sm-4 themed-grid-col">
                <button type="button" class="btn btn-lg btn-primary" onclick="setValue('30 min')">30 min</button>
              </div>
              <div class="col-sm-4 themed-grid-col">
                <button type="button" class="btn btn-lg btn-primary" onclick="setValue('60 min')">60 min</button>
              </div>
            </div>
            <div class="row mb-3 text-center">
              <div class="col-sm-8 themed-grid-col">
                <div class="form-floating mb-3">
                  <input class="form-control rounded-3" id="floatingInput" placeholder="spray time">
                  <label for="floatingInput">Spray Time</label>
                </div>
              </div>
              <div class="col-sm-4 themed-grid-col">
                <button type="submit" class="btn btn-st btn-primary" onclick="saveValue()">Save</button>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="modal-dialog" role="document">
        <div class="modal-content rounded-4 shadow">
          <div class="modal-footer flex-column align-items-stretch w-100 gap-2 pb-3 border-top-0">
            <button type="button" class="btn btn-lg btn-primary">Reset WiFi credentials</button>
            <script>
              var selectedValue = ""; // Variable to store the selected value

              function setValue(value) {
                document.getElementById("floatingInput").value = value;
                selectedValue = value; // Store the selected value in the variable
                alert("Selected value: " + value);
              }

              function saveValue() {
                var inputValue = document.getElementById("floatingInput").value;
                alert("Value saved: " + inputValue + " | Selected value: " + selectedValue);
              }
            </script>
          </div>
        </div>
      </div>
    </div>
  </div>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  initWiFi();

server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
  if (request->hasParam("value")) {
    AsyncWebParameter* param = request->getParam("value");
    if (param->isGet()) { // Change isPost() to isGet()
      receivedValue = param->value().toInt();
      Serial.print("Received value: ");
      Serial.println(receivedValue);
    }
  }
  request->send(SPIFFS, "/index.html", "text/html");
});

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });

  server.addHandler(&events);
  server.begin();

  unsigned long lastTime = 0;
  unsigned long timerDelay = 10000;
  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);
  digitalWrite(echopin, LOW);
}

void loop() {
  distance = getDistance();
  Serial.printf("Distance = %d cm\n", distance);
  Serial.printf("Recieved Value = %d\n", receivedValue);
  // Send Events to the Web Server with the Sensor Readings
  events.send("ping", NULL, millis());
  events.send(String(distance).c_str(), "temperature", millis());
}
