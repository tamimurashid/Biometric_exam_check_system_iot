// Full crash-proof ESP32 fingerprint access system with Google Apps Script + Sheet.Best
#include <Wire.h> // i2c connection 
#include <LiquidCrystal_I2C.h> // lcd display 
#include <Adafruit_Fingerprint.h> 
#include <HardwareSerial.h>
#include <WiFiClientSecure.h> // https 
#include <ArduinoJson.h>
#include <WiFi.h>

// Pins
#define RXD2 17
#define TXD2 16
#define BUZZER 5

HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// WiFi and cloud config
const char* ssid = "Reindeer";
const char* password = "200120022003";

const String deviceRoom = "PG12"; // This device's room
const char* host = "script.google.com";
const int httpsPort = 443;
const String server_id = "AKfycby1d8dDi74HWhSRHVeziIv2-sf-Bsa-y3369Cdt_nspXXPcQqkoJhDj_MTSkTpFQDFU";

const char* sheetHost = "sheet.best";
const char* sheetUUID = "4145c95d-615c-4dcd-9504-f88353d873f5"; // REPLACE with your real UUID

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Wire.begin(21, 22);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System init...");

  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);

  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor connected.");
    lcd.setCursor(0, 1);
    lcd.print("Sensor Found");
  } else {
    Serial.println("Sensor error!");
    lcd.setCursor(0, 1);
    lcd.print("Sensor Error");
    while (true);
  }

  delay(2000);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Status");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString());
  Serial.println("WiFi connected!");
  delay(2000);
}

void loop() {
  int result = getFingerprintID();

  if (result == FINGERPRINT_OK) {
    char hexID[10];
    sprintf(hexID, "%04X", finger.fingerID);
    String uid = "ID_" + String(hexID);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Checking...");
    Serial.println("UID: " + uid);

    sendToGoogle(uid);
  } 

  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan fingerprint");
  lcd.setCursor(0, 1);
  lcd.print("to access room");
}


int getFingerprintID() {
  if (finger.getImage() != FINGERPRINT_OK) return -1;
  if (finger.image2Tz() != FINGERPRINT_OK) return -1;
  if (finger.fingerSearch() == FINGERPRINT_OK) return FINGERPRINT_OK;
  return FINGERPRINT_NOTFOUND;
}

void sendToGoogle(String uid) {
  WiFiClientSecure googleClient;
  googleClient.setInsecure();

  if (!googleClient.connect(host, httpsPort)) {
    Serial.println("Google Script Conn Fail");
    return;
  }

  String url = "/macros/s/" + server_id + "/exec";
  String payload = "{\"user_id\":\"" + uid + "\",\"room_name\":\"" + deviceRoom + "\"}";

  googleClient.println("POST " + url + " HTTP/1.1");
  googleClient.println("Host: " + String(host));
  googleClient.println("Content-Type: application/json");
  googleClient.print("Content-Length: ");
  googleClient.println(payload.length());
  googleClient.println("Connection: close");
  googleClient.println();
  googleClient.println(payload);
  delay(1500);
  googleClient.stop();

  fetchResponseFromSheetBest(uid);
}

void fetchResponseFromSheetBest(String uid) {
  WiFiClientSecure sheetClient;
  sheetClient.setInsecure();

  String url = "/api/sheets/" + String(sheetUUID) + "?user_id=" + uid;

  if (!sheetClient.connect(sheetHost, 443)) {
    Serial.println("Sheet.best Conn Fail");
    return;
  }

  sheetClient.println("GET " + url + " HTTP/1.1");
  sheetClient.println("Host: " + String(sheetHost));
  sheetClient.println("Connection: close");
  sheetClient.println();

  String response;
  while (sheetClient.connected()) {
    String line = sheetClient.readStringUntil('\n');
    if (line == "\r") break;
  }

  while (sheetClient.available()) {
    response += sheetClient.readString();
  }
  sheetClient.stop();

  Serial.println("Sheet Response:");
  Serial.println(response);

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    lcdError("JSON Parse Err");
    return;
  }

  // Find correct match manually (safety against partial filter failure)
  bool found = false;
  for (JsonObject obj : doc.as<JsonArray>()) {
    String id = obj["user_id"];
    id.trim();

    if (id.equalsIgnoreCase(uid)) {
      String room = obj["Assigned Room"];
      String name = obj["Full Name"];
      room.trim();

      if (room.equalsIgnoreCase(deviceRoom)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Granted");
        lcd.setCursor(0, 1);
        lcd.print(name);
        successFeedback();
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Wrong Room!");
        lcd.setCursor(0, 1);
        lcd.print("Room: " + room);
        failFeedback();
      }

      found = true;
      break;
    }
  }

  if (!found) {
    lcdError("User Not Found");
  }
}


void lcdError(String msg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Denied");
  lcd.setCursor(0, 1);
  lcd.print(msg);
  failFeedback();
}

void successFeedback() {
  tone(BUZZER, 1000, 150); delay(150);
  tone(BUZZER, 1500, 150); delay(150);
  noTone(BUZZER);
}

void failFeedback() {
  tone(BUZZER, 400, 300); delay(300);
  noTone(BUZZER);
}
