#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- WiFi credentials ----------
const char* ssid = "";
const char* password = "";

// ---------- House B IP ----------
const char* houseB_IP = "10.223.185.1";   // IP of House B

// ---------- Pins ----------
const int relayPin = 26;      // Relay IN1 (active LOW)
const int switchPin = 32;     // Override switch (force OFF)
const int voltagePin = 35;    // Voltage sensor (SP / VP)
const int currentPin = 33;    // Current sensor (not used - using random values)

// ---------- LCD setup ----------
LiquidCrystal_I2C lcd(0x27,16,2);

// ---------- Web server ----------
WebServer server(80);

// ---------- Sensor calibration ----------
const float voltageMultiplier = 5.0;     // Adjust based on your voltage sensor

// ---------- Mode ----------
enum Mode { MODE_DONOR, MODE_RECEIVER };
Mode currentMode = MODE_DONOR;

// ---------- Variables ----------
float voltage = 0;
float current = 0;
float power = 0;

// ---------- DONOR mode variables ----------
bool requestPending = false;
bool requestAccepted = false;
float requestedPower = 0;
unsigned long requestedDuration = 0;
unsigned long startTime = 0;
float energySupplied = 0;
unsigned long lastEnergyUpdate = 0;
bool transferComplete = false;

// ---------- RECEIVER mode variables ----------
float receivedPower = 0;
float receivedCurrent = 0;

// ---------- Webpage handler ----------
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>House A - Smart Power System</title>";
  
  // Add auto-refresh when request is accepted or transfer complete
  if (requestAccepted || transferComplete) {
    html += "<meta http-equiv='refresh' content='2'>";
  }
  
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f0f8ff; color: #000; text-align: center; padding: 20px; }";
  html += "h2 { color: #0077b6; }";
  html += "button, input[type=submit] { padding: 10px 20px; margin: 5px; border: none; cursor: pointer; font-weight:bold; font-size: 16px; border-radius: 5px; }";
  html += ".mode-btn { background-color: #0077b6; color: white; }";
  html += ".mode-btn:hover { background-color: #023e8a; }";
  html += ".accept { background-color: #00c853; color: white; }";
  html += ".accept:hover { background-color: #007e33; }";
  html += ".decline { background-color: #d50000; color: white; }";
  html += ".decline:hover { background-color: #9b0000; }";
  html += "p { font-size: 18px; margin: 10px 0; }";
  html += ".readings { background-color: #e3f2fd; padding: 15px; border-radius: 10px; margin: 20px auto; max-width: 500px; }";
  html += ".mode-indicator { background-color: #023e8a; color: white; padding: 10px; border-radius: 5px; margin: 20px auto; max-width: 300px; }";
  html += "form { margin: 20px auto; background: #caf0f8; padding: 15px; border-radius: 8px; display: inline-block; }";
  html += "input[type=number] { padding: 5px; width: 100px; }";
  html += "hr { margin: 20px 0; }";
  html += ".complete { background-color: #4caf50; color: white; padding: 15px; border-radius: 8px; margin: 20px auto; max-width: 500px; font-weight: bold; }";
  html += "</style></head><body>";

  html += "<h2>House A - Smart Power System</h2>";

  // Mode selection
  html += "<div class='mode-indicator'><strong>Current Mode: ";
  html += (currentMode == MODE_DONOR) ? "DONOR (Sending Power)" : "RECEIVER (Receiving Power)";
  html += "</strong></div>";

  html += "<div>";
  html += "<a href='/setmode?mode=donor'><button class='mode-btn'>Switch to DONOR Mode</button></a> ";
  html += "<a href='/setmode?mode=receiver'><button class='mode-btn'>Switch to RECEIVER Mode</button></a>";
  html += "</div><hr>";

  // ---------- DONOR MODE display ----------
  if (currentMode == MODE_DONOR) {
    html += "<h3>Donor Mode</h3>";
    
    if (transferComplete) {
      html += "<div class='complete'>";
      html += "<p>✓ Power Transfer Complete!</p>";
      html += "<p>Total Energy Supplied: " + String(energySupplied, 3) + " Wh</p>";
      html += "<p>Redirecting to home page...</p>";
      html += "</div>";
      html += "<script>setTimeout(function(){ window.location.href='/clearcomplete'; }, 3000);</script>";
    } else if (requestPending) {
      html += "<p style='color:green;font-weight:bold;'>Request received from House B</p>";
      html += "<p>Requested Duration: " + String(requestedDuration / 1000) + " seconds</p>";
      html += "<a href='/accept'><button class='accept'>ACCEPT</button></a> ";
      html += "<a href='/decline'><button class='decline'>DECLINE</button></a>";
    } else if (requestAccepted) {
      html += "<p style='color:blue;font-weight:bold;'>Currently supplying power to House B</p>";
      unsigned long elapsed = (millis() - startTime) / 1000;
      html += "<p><strong>Elapsed Time:</strong> " + String(elapsed) + " seconds</p>";
      //html += "<p><strong>Energy Supplied:</strong> " + String(energySupplied, 3) + " Wh</p>";
      html += "<p><strong>Current Power:</strong> " + String(power, 2) + " W</p>";
      html += "<a href='/stop'><button class='decline'>STOP Supply</button></a>";
    } else {
      html += "<p>Ready to supply power. Waiting for requests...</p>";
    }
  }
  
  // ---------- RECEIVER MODE display ----------
  else {
    html += "<h3>Receiver Mode</h3>";
    html += "<p>Request power from House B:</p>";
    html += "<form action='/sendrequest' method='get'>";
    html += "<label>Time (s): </label><input type='number' name='time' required><br><br>";
    html += "<input type='submit' value='Send Request to House B'>";
    html += "</form>";
    html += "<p>Receiving Power: " + String(receivedPower, 2) + " W</p>";
  }

  html += "<hr>";
  
  // ---------- Sensor readings ----------
  html += "<div class='readings'>";
  html += "<h3>Current Readings</h3>";
  html += "<p><strong>Voltage:</strong> " + String(voltage, 2) + " V</p>";
  html += "<p><strong>Current:</strong> " + String(current, 3) + " A</p>";
  html += "<p><strong>Power:</strong> " + String(power, 2) + " W</p>";
  html += "<p><strong>Relay State:</strong> " + String(digitalRead(relayPin) ? "OFF" : "ON") + "</p>";
  html += "</div>";
  
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// ---------- Clear complete status ----------
void handleClearComplete() {
  transferComplete = false;
  server.sendHeader("Location", "/");
  server.send(303);
}

// ---------- Mode switching ----------
void handleSetMode() {
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");
    if (mode == "donor") {
      currentMode = MODE_DONOR;
      requestAccepted = false;
      requestPending = false;
      transferComplete = false;
      digitalWrite(relayPin, HIGH); // Turn off relay
      Serial.println("Switched to DONOR mode");
    } else if (mode == "receiver") {
      currentMode = MODE_RECEIVER;
      requestAccepted = false;
      requestPending = false;
      transferComplete = false;
      digitalWrite(relayPin, HIGH); // Turn off relay
      Serial.println("Switched to RECEIVER mode");
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// ---------- DONOR mode routes ----------
void handleAccept() {
  if (currentMode == MODE_DONOR) {
    requestAccepted = true;
    requestPending = false;
    transferComplete = false;
    startTime = millis();
    lastEnergyUpdate = millis();
    energySupplied = 0;
    Serial.println("Request ACCEPTED - Starting power supply");
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleDecline() {
  requestAccepted = false;
  requestPending = false;
  energySupplied = 0;
  transferComplete = false;
  digitalWrite(relayPin, HIGH);
  Serial.println("Request DECLINED");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleStop() {
  requestAccepted = false;
  transferComplete = true;
  digitalWrite(relayPin, HIGH);
  Serial.println("Power supply STOPPED manually");
  Serial.println("Total energy supplied: " + String(energySupplied, 3) + " Wh");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleRequest() {
  if (currentMode == MODE_DONOR) {
    if (server.hasArg("time")) requestedDuration = server.arg("time").toInt() * 1000UL;
    requestPending = true;
    transferComplete = false;
    Serial.println("New power request received:");
    Serial.println("  Duration: " + String(requestedDuration/1000) + " s");
    server.send(200, "text/plain", "Request received!");
  } else {
    server.send(400, "text/plain", "House A is in RECEIVER mode");
  }
}

// ---------- RECEIVER mode route ----------
void handleSendRequest() {
  if (currentMode == MODE_RECEIVER) {
    if (server.hasArg("time")) {
      String tme = server.arg("time");

      WiFiClient client;
      if (client.connect(houseB_IP, 80)) {
        client.print(String("GET /request?time=") + tme +
                     " HTTP/1.1\r\nHost: " + houseB_IP + "\r\nConnection: close\r\n\r\n");
        Serial.println("Request sent to House B");
        Serial.println("   Requested Time : " + tme + " s");
        server.send(200, "text/html", "<html><head><meta http-equiv='refresh' content='2;url=/'></head><body><h3>Request Sent to House B!</h3><p>Redirecting...</p></body></html>");
      } else {
        Serial.println("Failed to connect to House B");
        server.send(500, "text/html", "<html><body><h3>Failed to connect to House B</h3><a href='/'>Go Back</a></body></html>");
      }
    } else {
      server.send(400, "text/plain", "Time parameter missing");
    }
  } else {
    server.send(400, "text/plain", "House A is in DONOR mode");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n========================================");
  Serial.println("House A - Bidirectional Power System");
  Serial.println("========================================");

  pinMode(relayPin, OUTPUT);
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(voltagePin, INPUT);
  pinMode(currentPin, INPUT);
  digitalWrite(relayPin, HIGH); // Relay OFF (active LOW)

  lcd.init();
  lcd.backlight();

  // ---------- Connect WiFi ----------
  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
    Serial.print(".");
  }

  lcd.clear();
  lcd.print("WiFi Connected");
  Serial.println("\n WiFi Connected!");

  // ---------- Show IP ----------
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("IP Address:");
  lcd.setCursor(0,1);
  lcd.print(ip);
  delay(4000);

  lcd.clear();
  lcd.print("System Ready");
  delay(1000);

  // ---------- Start Web Server ----------
  server.on("/", handleRoot);
  server.on("/setmode", handleSetMode);
  server.on("/request", handleRequest);
  server.on("/accept", handleAccept);
  server.on("/decline", handleDecline);
  server.on("/stop", handleStop);
  server.on("/sendrequest", handleSendRequest);
  server.on("/clearcomplete", handleClearComplete);
  server.begin();
  Serial.println("Web Server Started");
  Serial.println("Current Mode: DONOR");
  Serial.println("========================================\n");
  
  // Seed random number generator
  randomSeed(analogRead(0));
}

void loop() {
  server.handleClient();

  // ---------- Read voltage ----------
  int voltageADC = analogRead(voltagePin);
  float voltageRaw = (voltageADC / 4095.0) * 3.3;
  voltage = voltageRaw * voltageMultiplier;
  
  // ---------- Generate random current between 0.01 and 0.05 A ----------
  current = random(10, 51) / 1000.0; // Generate random value between 0.01 and 0.05
  
  // ---------- Calculate power ----------
  power = voltage * current;

  // ---------- Read override switch ----------
  bool switchState = digitalRead(switchPin); // LOW = pressed

  // ---------- DONOR MODE: Relay control ----------
  if (currentMode == MODE_DONOR) {
    if (requestAccepted && switchState == HIGH) {
      digitalWrite(relayPin, LOW);  // Relay ON

      // Calculate energy supplied
      unsigned long now = millis();
      float elapsedHours = (now - lastEnergyUpdate) / 3600000.0;
      energySupplied += power * elapsedHours;
      lastEnergyUpdate = now;

      // Auto-turn OFF after requested duration
      if (requestedDuration > 0 && (now - startTime) >= requestedDuration) {
        digitalWrite(relayPin, HIGH);
        requestAccepted = false;
        transferComplete = true;
        Serial.println("Power supply stopped: Duration reached");
        Serial.println("Total energy supplied: " + String(energySupplied, 3) + " Wh");
      }

      // Auto-turn OFF after requested energy
      if (requestedPower > 0 && energySupplied >= (requestedPower / 1000.0)) {
        digitalWrite(relayPin, HIGH);
        requestAccepted = false;
        transferComplete = true;
        Serial.println("Power supply stopped: Energy target reached");
        Serial.println("Total energy supplied: " + String(energySupplied, 3) + " Wh");
      }
    } else {
      digitalWrite(relayPin, HIGH); // Relay OFF
      if (requestAccepted && switchState == LOW) {
        requestAccepted = false;
        transferComplete = true;
        Serial.println("Power supply stopped: Override switch pressed");
        Serial.println("Total energy supplied: " + String(energySupplied, 3) + " Wh");
      }
    }
  }
  
  // ---------- RECEIVER MODE: Just monitor ----------
  else {
    digitalWrite(relayPin, HIGH); // Keep relay OFF in receiver mode
    receivedPower = power;
    receivedCurrent = current;
  }

  // ---------- Update LCD ----------
  static unsigned long lastLCDUpdate = 0;
  static bool showVoltCurr = true;
  
  if (millis() - lastLCDUpdate > 2000) {
    lcd.clear();
    if (showVoltCurr) {
      lcd.setCursor(0, 0);
      lcd.print("V:");
      lcd.print(voltage, 2);
      lcd.print("V");
      lcd.setCursor(0, 1);
      lcd.print("I:");
      lcd.print(current, 3);
      lcd.print("A");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("P:");
      lcd.print(power, 2);
      lcd.print("W");
      lcd.setCursor(0, 1);
      if (currentMode == MODE_DONOR) {
        lcd.print("Mode: DONOR");
      } else {
        lcd.print("Mode: RECEIVER");
      }
    }
    showVoltCurr = !showVoltCurr;
    lastLCDUpdate = millis();
  }

  // ---------- Serial Debug ----------
  static unsigned long lastSerialPrint = 0;
  if (millis() - lastSerialPrint > 2000) {
    Serial.print("V: ");
    Serial.print(voltage, 2);
    Serial.print("V | I: ");
    Serial.print(current, 3);
    Serial.print("A | P: ");
    Serial.print(power, 2);
    Serial.print("W | Mode: ");
    Serial.print(currentMode == MODE_DONOR ? "DONOR" : "RECEIVER");
    Serial.print(" | Relay: ");
    Serial.println(digitalRead(relayPin) == LOW ? "ON" : "OFF");
    lastSerialPrint = millis();
  }

  delay(100);
}