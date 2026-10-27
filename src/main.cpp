#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>      // Enables http://stu-gate.local
#include "web_page.h"

// --- PRODUCT HOTSPOT NAME (Only visible when unconfigured) ---
const char* AP_SSID = "STU-SmartGate";
const char* AP_PASS = "stu123456";

// --- FACTORY DEFAULT CREDENTIALS (Restored on 5-sec button hold) ---
const char* DEFAULT_ADMIN_USER = "admin";
const char* DEFAULT_ADMIN_PASS = "admin123";
const char* DEFAULT_GATE_PIN   = "1234";

// --- HARDWARE PIN ASSIGNMENTS ---
#define RELAY_SINGLE_PIN  18   // 1 GATE Relay
#define RELAY_DUAL_PIN    19   // 2 GATE Relay
#define GATE_SENSOR_PIN   5   // Magnetic Reed / Limit Sensor (GPIO 21 to GND)
#define RESET_BTN_PIN     0    // Physical Reset Button (GPIO 0 to GND - also onboard BOOT btn)
#define STATUS_LED_PIN    2    // Onboard Blue LED for visual 5-sec reset feedback

#define RELAY_ACTIVE_LOW  false

// When sponge blocks sensor -> Sensor pulls line to GND (LOW) -> Door is CLOSED
#define SENSOR_CLOSED_VAL LOW  // Flip to HIGH if Open/Closed reads inverted

WebServer server(80);
Preferences prefs;

String savedSsid      = "";
String savedPass      = "";
String currentGatePin = "";
String adminUser      = "";
String adminPass      = "";

// Physical Reset Button Tracking
unsigned long btnPressStartTime = 0;
bool isBtnPressed = false;

void pulseRelay(int pin) {
  digitalWrite(pin, RELAY_ACTIVE_LOW ? LOW : HIGH);
  delay(500);
  digitalWrite(pin, RELAY_ACTIVE_LOW ? HIGH : LOW);
}

// Loads saved configuration or falls back to factory defaults
void loadConfiguration() { 
  savedSsid      = prefs.getString("ssid", "");
  savedPass      = prefs.getString("pass", "");
  currentGatePin = prefs.getString("gatepin", DEFAULT_GATE_PIN);
  adminUser      = prefs.getString("admuser", DEFAULT_ADMIN_USER);
  adminPass      = prefs.getString("admpass", DEFAULT_ADMIN_PASS);
}

// Wipes everything back to factory defaults (admin / admin123, PIN 1234, no Wi-Fi)
void executeFactoryReset() {
  Serial.println("\n[!] 5-SECOND HOLD DETECTED: EXECUTING FACTORY RESET [!]");

  // Rapidly blink onboard LED to alert user that factory reset succeeded
  for (int i = 0; i < 10; i++) {
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    delay(100);
  }
  digitalWrite(STATUS_LED_PIN, LOW);

  prefs.clear(); // Wipes Wi-Fi, Gate PIN, and Admin credentials from NVS Flash
  WiFi.disconnect(false, true);
  delay(500);
  ESP.restart(); // Reboot clean into "STU-SmartGate" AP setup mode
}

// Checks if physical button is held down continuously for 5 seconds
void checkFactoryResetButton() {
  if (digitalRead(RESET_BTN_PIN) == LOW) { // Button pressed (Active LOW)
    if (!isBtnPressed) {
      isBtnPressed = true;
      btnPressStartTime = millis();
      Serial.println("Reset button pressed... Hold for 5 seconds to wipe.");
    } else if (millis() - btnPressStartTime >= 5000) {
      executeFactoryReset();
    }
  } else {
    if (isBtnPressed) {
      Serial.println("Reset button released before 5 seconds. Cancelled.");
      isBtnPressed = false;
    }
  }
}

// Wi-Fi Event Callback: Tells us EXACTLY why the router is dropping the ESP32
void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.printf("\n[Wi-Fi Event] Disconnected! Reason Code: %d\n", info.wifi_sta_disconnected.reason);
      if (info.wifi_sta_disconnected.reason == 201) {
        Serial.println(">>> REASON 201: NO_AP_FOUND (Router is on Channel 12/13 or out of range!)");
      } else if (info.wifi_sta_disconnected.reason == 202 || info.wifi_sta_disconnected.reason == 15) {
        Serial.println(">>> REASON 202/15: AUTH_FAIL / 4-WAY HANDSHAKE TIMEOUT (Router rejected password or WPA mode)");
      } else if (info.wifi_sta_disconnected.reason == 2) {
        Serial.println(">>> REASON 2: AUTH_EXPIRE (Router kicked ESP32)");
      }
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("\n[Wi-Fi Event] Connected to AP! Waiting for IP address...");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("\n[Wi-Fi Event] Got IP: ");
      Serial.println(WiFi.localIP());
      break;
    default:
      break;
  }
}

// Boots network: If Wi-Fi is saved & connects, "STU-SmartGate" AP is completely shut off
void startNetwork() {
  savedSsid = prefs.getString("ssid", "");
  savedPass = prefs.getString("pass", "");

  WiFi.onEvent(onWiFiEvent);

  if (savedSsid.length() > 0) {
    Serial.println("\n=========================================");
    Serial.print("Connecting to Home Wi-Fi: ");
    Serial.println(savedSsid);
    Serial.println("=========================================");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);
    delay(200);

    WiFi.setSleep(false);

    // --- STATIC IP CONFIGURATION (Bypasses DHCP lock) ---
    // If your Unifi router IP is 192.168.1.1, change all "0" below to "1"
    IPAddress local_IP(192, 168, 0, 222);   // Fixed Static IP for your gate
    IPAddress gateway(192, 168, 0, 1);       // Your Unifi router gateway IP
    IPAddress subnet(255, 255, 255, 0);      // Standard Subnet Mask
    IPAddress primaryDNS(8, 8, 8, 8);        // Google DNS
    IPAddress secondaryDNS(1, 1, 1, 1);      // Cloudflare DNS

    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("[Wi-Fi Config] Static IP configuration failed!");
    } else {
      Serial.println("[Wi-Fi Config] Static IP applied: 192.168.0.222");
    }

    // Begin Wi-Fi connection with static parameters
    WiFi.begin(savedSsid.c_str(), savedPass.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
      delay(400);
      Serial.print(".");
      checkFactoryResetButton();
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n\n=========================================");
      Serial.println("[SUCCESS] Connected to Home Router!");
      Serial.print("Gate Web Dashboard: http://");
      Serial.println(WiFi.localIP());

      if (MDNS.begin("stu-gate")) {
        Serial.println("Gate Local URL    : http://stu-gate.local");
      }
      Serial.println("=========================================");
      return;
    }

    Serial.println("\n[FAIL] Reverting to Setup Hotspot...");
  }

  // Setup Hotspot Fallback
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.println("\n=========================================");
  Serial.println("SETUP HOTSPOT ACTIVE");
  Serial.println("1. Connect to Wi-Fi: " + String(AP_SSID));
  Serial.print("2. Open Browser to : http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("=========================================");
}

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleWifiStatus() {
  bool conn = (WiFi.status() == WL_CONNECTED);
  String json = "{\"connected\":" + String(conn ? "true" : "false") +
                ",\"ssid\":\"" + (conn ? WiFi.SSID() : "") + "\"}";
  server.send(200, "application/json", json);
}
// --- SCAN NEARBY 2.4GHz NETWORKS ---
void handleScanWifi() {
  Serial.println("[Wi-Fi Scanner] Scanning airwaves...");
  int n = WiFi.scanNetworks();
  
  String json = "[";
  for (int i = 0; i < n; ++i) {
    if (i > 0) json += ",";
    json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  json += "]";
  WiFi.scanDelete(); // Free scan memory
  
  server.send(200, "application/json", json);
}

// --- DIAGNOSTIC CONNECTION HANDLER ---
void handleSaveWifi() {
  String targetSsid = "";
  String targetPass = "";

  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    int sIdx = body.indexOf("\"ssid\":\"");
    if (sIdx != -1) {
      int sEnd = body.indexOf("\"", sIdx + 8);
      targetSsid = body.substring(sIdx + 8, sEnd);
    }
    int pIdx = body.indexOf("\"pass\":\"");
    if (pIdx != -1) {
      int pEnd = body.indexOf("\"", pIdx + 8);
      targetPass = body.substring(pIdx + 8, pEnd);
    }
  } else {
    targetSsid = server.arg("ssid");
    targetPass = server.arg("pass");
  }

  if (targetSsid.length() == 0) {
    server.send(400, "text/plain", "Missing SSID");
    return;
  }

  Serial.println("\n-------------------------------------------");
  Serial.print("[Wi-Fi Setup] Saving SSID: ");
  Serial.println(targetSsid);
  Serial.print("[Wi-Fi Setup] Saving Password Length: ");
  Serial.println(targetPass.length());
  Serial.println("-------------------------------------------");

  // Save permanently to flash
  prefs.putString("ssid", targetSsid);
  prefs.putString("pass", targetPass);

  // Send success response back to phone
  server.send(200, "text/plain", "SAVED");

  // Give the HTTP response 1 second to reach the phone, then reboot cleanly
  delay(1000);
  Serial.println("[Wi-Fi Setup] Restarting ESP32 into Station Mode...");
  ESP.restart();
}

void handleLogoutWifi() {
  prefs.remove("ssid");
  prefs.remove("pass");
  savedSsid = "";
  savedPass = "";

  server.send(200, "text/plain", "Logged Out");
  delay(800);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS); // Re-broadcast "STU-SmartGate" setup network
  Serial.println("Wi-Fi logged out. 'STU-SmartGate' hotspot is now visible.");
}

void handleVerifyPin() {
  if (WiFi.status() != WL_CONNECTED) {
    server.send(403, "text/plain", "Connect Wi-Fi First");
    return;
  }
  if (server.arg("pin") == currentGatePin) {
    server.send(200, "text/plain", "OK");
  } else {
    server.send(401, "text/plain", "Wrong PIN");
  }
}

void handleStatus() {
  bool closed = (digitalRead(GATE_SENSOR_PIN) == SENSOR_CLOSED_VAL);
  String json = String("{\"closed\":") + (closed ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

void handleTrigger() {
  if (WiFi.status() != WL_CONNECTED || server.arg("pin") != currentGatePin) {
    server.send(401, "text/plain", "Unauthorized");
    return;
  }
  String gate = server.arg("gate");
  if (gate == "single") {
    pulseRelay(RELAY_SINGLE_PIN);
    server.send(200, "text/plain", "1 Gate Pulsed");
  } else if (gate == "dual") {
    pulseRelay(RELAY_DUAL_PIN);
    server.send(200, "text/plain", "2 Gate Pulsed");
  }
}

void handleAdminLogin() {
  if (server.arg("user") == adminUser && server.arg("pass") == adminPass) {
    server.send(200, "text/plain", "OK");
  } else {
    server.send(401, "text/plain", "Denied");
  }
}

void handleSetConfig() {
  if (server.arg("user") != adminUser || server.arg("pass") != adminPass) {
    server.send(401, "text/plain", "Unauthorized");
    return;
  }

  String np    = server.arg("newpin");
  String nuser = server.arg("newuser");
  String npass = server.arg("newpass");

  if (np.length() >= 4) {
    currentGatePin = np;
    prefs.putString("gatepin", np);
  }
  if (nuser.length() > 0) {
    adminUser = nuser;
    prefs.putString("admuser", nuser);
  }
  if (npass.length() > 0) {
    adminPass = npass;
    prefs.putString("admpass", npass);
  }

  server.send(200, "text/plain", "Saved");
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_SINGLE_PIN, OUTPUT);
  pinMode(RELAY_DUAL_PIN, OUTPUT);
  digitalWrite(RELAY_SINGLE_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  digitalWrite(RELAY_DUAL_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);

  pinMode(GATE_SENSOR_PIN, INPUT_PULLUP);
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  prefs.begin("stu-prod", false);
  loadConfiguration();
  startNetwork();

  server.enableCORS(true);
  server.on("/", HTTP_GET, handleRoot);
  server.on("/wifi-status", HTTP_GET, handleWifiStatus);
  server.on("/save-wifi", HTTP_POST, handleSaveWifi);
  server.on("/logout-wifi", HTTP_POST, handleLogoutWifi);
  server.on("/verify-pin", HTTP_POST, handleVerifyPin);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/trigger", HTTP_POST, handleTrigger);
  server.on("/admin-login", HTTP_POST, handleAdminLogin);
  server.on("/scan-wifi", HTTP_GET, handleScanWifi);
  server.on("/set-config", HTTP_POST, handleSetConfig);

  server.begin();
}

void loop() {
  server.handleClient();
  checkFactoryResetButton();
}