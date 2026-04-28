#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "HardwareSerial.h"
#include "DFRobotDFPlayerMini.h"

//for MP3
#define RX_PIN 27 
#define TX_PIN 26 
HardwareSerial mySerial(1); 
DFRobotDFPlayerMini myDFPlayer;

// --- DATA for MP3 ---
int fileNumbers[] = {1, 2, 3, 4, 5, 6};
String fileNames[] = {
  "0001_wake_up_alert",
  "0002_engine_restarted",
  "0003_emergency_engine_shutdown",
  "0004_please_wake_up",
  "0005_waiting_for_network_connection",
  "0006_welcome_back"
};

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
const int IN3 = 22;
const int IN4 = 23;

const int RED_LED = 13;
const int YEL_LED = 12;
const int GRN_LED = 14;

// --- State Variables ---
int drowsyLevel = 0; // 0 = Awake, 1 = Mild, 2 = Danger
unsigned long lastBlinkTime = 0;
bool blinkState = false;
const int blinkInterval = 500; // Blink speed in milliseconds

// --- Audio Sequencing Variables ---
int dangerSequenceStep = 0;
unsigned long lastAudioTime = 0;
bool welcomePlayed = true; // Set to true initially so it doesn't play on startup

WebServer server(80);

// --- Function Declarations ---
void handleDrowsyAlert();
void handleMildAlert();
void handleReset();
void handleAudioSequencing(); // Added for the logic fix

void setup() {
  Serial.begin(115200);

  // Initialize Motor Pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  
  // Initialize Traffic Light Pins
  pinMode(RED_LED, OUTPUT);
  pinMode(YEL_LED, OUTPUT);
  pinMode(GRN_LED, OUTPUT);

  // Initialize MP3
  mySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  if (myDFPlayer.begin(mySerial, false, false)) { // isACK=false for better stability
    myDFPlayer.volume(15); 
    Serial.println(F("DFPlayer Online"));
  }

  // WiFi Configuration
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
    Serial.println("\nConnected!");
    Serial.println(WiFi.localIP());
  }

  if (MDNS.begin("sentinel")) {
    Serial.println("MDNS started");
  }

  // Web Server Routes
  server.on("/alert", handleDrowsyAlert); // Red Blink + Stop
  server.on("/mild", handleMildAlert);   // Yellow Blink + Drive
  server.on("/reset", handleReset);       // Green Solid + Drive
  server.begin();
}

void loop() {
  server.handleClient();

  // Timer for blinking (Non-blocking)
  if (millis() - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = millis();
    blinkState = !blinkState;
  }

  // --- AUDIO LOGIC ---
  handleAudioSequencing();

  // --- BRAIN OF THE SYSTEM ---
  if (drowsyLevel == 0) { 
    // AWAKE: Green Stable, Motors Driving
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    
    digitalWrite(GRN_LED, HIGH);
    digitalWrite(YEL_LED, LOW);
    digitalWrite(RED_LED, LOW);
  } 
  else if (drowsyLevel == 1) { 
    // MILD: Yellow Blinking, Motors Driving (Warning)
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);

    digitalWrite(GRN_LED, LOW);
    digitalWrite(YEL_LED, blinkState); // Uses the timer state
    digitalWrite(RED_LED, LOW);
  } 
  else if (drowsyLevel == 2) { 
    // DANGER: Red Blinking, Motors Stopped
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);

    digitalWrite(GRN_LED, LOW);
    digitalWrite(YEL_LED, LOW);
    digitalWrite(RED_LED, blinkState); // Uses the timer state
  }
}

// --- Logic for MP3 Sequencing ---
void handleAudioSequencing() {
  // Danger Logic: 1-3 then loop 1-4
  if (drowsyLevel == 2) {
    welcomePlayed = false; 
    if (millis() - lastAudioTime > 4000) { // Adjust 4000ms based on audio length
      lastAudioTime = millis();
      if (dangerSequenceStep == 0)      { myDFPlayer.play(1); dangerSequenceStep = 1; }
      else if (dangerSequenceStep == 1) { myDFPlayer.play(3); dangerSequenceStep = 2; }
      else if (dangerSequenceStep == 2) { myDFPlayer.play(1); dangerSequenceStep = 3; } // Loop Start
      else if (dangerSequenceStep == 3) { myDFPlayer.play(4); dangerSequenceStep = 2; } // Loop Back
    }
  } 
  // Reset Logic: 6 then 2 once
  else if (drowsyLevel == 0 && !welcomePlayed) {
    static int resetStep = 0;
    if (millis() - lastAudioTime > 4000) {
      if (resetStep == 0) { 
        myDFPlayer.play(6); 
        lastAudioTime = millis(); 
        resetStep = 1; 
      }
      else if (resetStep == 1) { 
        myDFPlayer.play(2); 
        welcomePlayed = true; 
        resetStep = 0; 
      }
    }
  }
}

// --- Handler Functions ---
void handleDrowsyAlert() {
  if (drowsyLevel != 2) {
    Serial.println("!!! DANGER ALERT !!!");
    drowsyLevel = 2;
    dangerSequenceStep = 0; // Restart sequence from 1-3
    lastAudioTime = 0;      // Play immediately
  }
  server.send(200, "text/plain", "Danger Mode: Stopped");
}

void handleMildAlert() {
  Serial.println("Warning: Mild Drowsiness");
  drowsyLevel = 1;
  server.send(200, "text/plain", "Mild Mode: Warning");
}

void handleReset() {
  if (drowsyLevel != 0) {
    Serial.println("System Reset: Awake");
    drowsyLevel = 0;
    welcomePlayed = false; // Trigger welcome audio
    lastAudioTime = 0;     // Play immediately
  }
  server.send(200, "text/plain", "Normal Mode: Resumed");
}