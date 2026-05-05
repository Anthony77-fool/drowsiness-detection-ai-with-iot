#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HardwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// --- OLED CONFIGURATION ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- MP3 CONFIGURATION ---
// IMPORTANT: ESP32 RX_PIN connects to DFPlayer TX | TX_PIN connects to 1k resistor -> DFPlayer RX
#define RX_PIN 26 
#define TX_PIN 27 
HardwareSerial mySerial(2); // Using UART2
DFRobotDFPlayerMini myDFPlayer;

// --- WiFi Credentials ---
const char* ssid = "Gafi3"; 
const char* password = "12345678";

// --- STATIC IP CONFIGURATION ---
IPAddress local_IP(192, 168, 43, 100); 
IPAddress gateway(192, 168, 43, 1);    
IPAddress subnet(255, 255, 255, 0); 

// --- Pin Definitions ---
const int IN1 = 18; 
const int IN2 = 19;
const int IN3 = 25; 
const int IN4 = 33; 

const int RED_LED = 13;
const int YEL_LED = 12;
const int GRN_LED = 14;

// --- State Variables ---
int drowsyLevel = 3; // Start at 3 (Connecting)
unsigned long lastBlinkTime = 0;
bool blinkState = false;
const int blinkInterval = 500; 

// --- Audio Sequencing Variables ---
int dangerSequenceStep = 0;
unsigned long lastAudioTime = 0;
bool welcomePlayed = true; 

WebServer server(80);

// --- Function Declarations ---
void handleDrowsyAlert();
void handleMildAlert();
void handleReset();
void handleAudioSequencing();
void updateOLED(int state);

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for Serial Monitor to catch up

  // 1. Initialize OLED (Low Power)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED failed"));
  }
  updateOLED(3); // Show "Connecting" face

  // 2. Initialize Hardware Pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(RED_LED, OUTPUT); pinMode(YEL_LED, OUTPUT); pinMode(GRN_LED, OUTPUT);

  // 3. Initialize MP3 (STAGGERED START)
  // We do this BEFORE WiFi to avoid power spikes
  Serial.println(F("Initializing MP3..."));
  mySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(2000); // CRITICAL: Give the DFPlayer time to wake up before communicating

  if (myDFPlayer.begin(mySerial, true, true)) { 
    Serial.println(F("✅ DFPlayer Online"));
    myDFPlayer.volume(10); // KEEP VOLUME LOW (8-12) because you have no capacitor
    delay(500);
    myDFPlayer.stop();
  } else {
    Serial.println(F("❌ DFPlayer NOT detected. Check wiring/SD card."));
    // We don't use while(true) here so the rest of the car still works
  }

  // 4. WiFi Configuration (THE HEAVY LIFTER)
  // We do this last because it draws the most current
  delay(1000); 
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, password);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 20) {
    delay(500);
    Serial.print(".");
    counter++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    drowsyLevel = 0; // Set to Awake
    updateOLED(0); 
  } else {
    Serial.println("\n⚠️ WiFi Timeout - Running offline mode");
    drowsyLevel = 0;
  }

  if (MDNS.begin("sentinel")) {
    Serial.println("MDNS started");
  }

  server.on("/alert", handleDrowsyAlert); 
  server.on("/mild", handleMildAlert);   
  server.on("/reset", handleReset);      
  server.begin();
  
  Serial.println("System Ready!");
}

void loop() {
  server.handleClient();

  // Timer for blinking LEDs
  if (millis() - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = millis();
    blinkState = !blinkState;
  }

  handleAudioSequencing();

  // --- BRAIN: Control Motors, LEDs, and OLED ---
  if (drowsyLevel == 0) { 
    // AWAKE: Motors ON, Green LED ON
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    digitalWrite(GRN_LED, HIGH); digitalWrite(YEL_LED, LOW); digitalWrite(RED_LED, LOW);
    updateOLED(0); 
  } 
  else if (drowsyLevel == 1) { 
    // MILD: Motors ON, Yellow LED Blinking
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    digitalWrite(GRN_LED, LOW); digitalWrite(YEL_LED, blinkState); digitalWrite(RED_LED, LOW);
    updateOLED(1);
  } 
  else if (drowsyLevel == 2) { 
    // DANGER: Motors STOP, Red LED Blinking
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    digitalWrite(GRN_LED, LOW); digitalWrite(YEL_LED, LOW); digitalWrite(RED_LED, blinkState);
    updateOLED(2);
  }
}

void updateOLED(int state) {
  static int lastState = -1;
  if (state == lastState) return; 
  lastState = state;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (state == 0) { // AWAKE
    display.fillCircle(40, 30, 15, SSD1306_WHITE);
    display.fillCircle(88, 30, 15, SSD1306_WHITE);
    display.drawRoundRect(54, 50, 20, 5, 2, SSD1306_WHITE);
  } 
  else if (state == 1) { // MILD
    display.fillRect(25, 30, 30, 8, SSD1306_WHITE);
    display.fillRect(73, 30, 30, 8, SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(45, 50);
    display.print("TIRED...");
  } 
  else if (state == 2) { // DANGER
    display.setTextSize(4);
    display.setCursor(30, 20); display.print("X");
    display.setCursor(78, 20); display.print("X");
  }
  else if (state == 3) { // CONNECTING
    display.drawCircle(40, 30, 10, SSD1306_WHITE);
    display.drawCircle(88, 30, 10, SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(20, 50);
    display.print("CONNECTING...");
  }
  display.display();
}

void handleAudioSequencing() {
  if (drowsyLevel == 2) {
    welcomePlayed = false; 
    // Play danger audio every 4 seconds
    if (millis() - lastAudioTime > 4000) {
      lastAudioTime = millis();
      if (dangerSequenceStep == 0)      { myDFPlayer.playMp3Folder(1); dangerSequenceStep = 1; }
      else if (dangerSequenceStep == 1) { myDFPlayer.playMp3Folder(3); dangerSequenceStep = 2; }
      else if (dangerSequenceStep == 2) { myDFPlayer.playMp3Folder(1); dangerSequenceStep = 3; } 
      else if (dangerSequenceStep == 3) { myDFPlayer.playMp3Folder(4); dangerSequenceStep = 2; } 
    }
  } 
  else if (drowsyLevel == 0 && !welcomePlayed) {
    static int resetStep = 0;
    // Play recovery audio
    if (millis() - lastAudioTime > 4000) {
      if (resetStep == 0) { 
        myDFPlayer.playMp3Folder(6); 
        lastAudioTime = millis(); 
        resetStep = 1; 
      }
      else if (resetStep == 1) { 
        myDFPlayer.playMp3Folder(2); 
        welcomePlayed = true; 
        resetStep = 0; 
      }
    }
  }
}

void handleDrowsyAlert() {
  if (drowsyLevel != 2) {
    Serial.println("!!! DANGER ALERT !!!");
    drowsyLevel = 2;
    dangerSequenceStep = 0; 
    lastAudioTime = 0;          
  }
  server.send(200, "text/plain", "Danger Mode");
}

void handleMildAlert() {
  drowsyLevel = 1;
  server.send(200, "text/plain", "Mild Mode");
}

void handleReset() {
  if (drowsyLevel != 0) {
    drowsyLevel = 0;
    welcomePlayed = false; 
    lastAudioTime = 0;     
  }
  server.send(200, "text/plain", "System Reset");
}
