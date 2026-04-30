# 🚗💤 Eye-O-T Pilot: Fatigue Detection at the Edge

<p align="center">
  <img src="https://img.shields.io/badge/Edge%20AI-Real--Time-blue?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Python-3.10+-yellow?style=for-the-badge&logo=python" />
  <img src="https://img.shields.io/badge/YOLOv11-Nano-red?style=for-the-badge" />
  <img src="https://img.shields.io/badge/ESP32-IoT-green?style=for-the-badge&logo=espressif" />
  <img src="https://img.shields.io/badge/Status-Completed-success?style=for-the-badge" />
</p>

---

## 🎥 Demo Showcase

### 📌 System Demo (Real-Time Detection)
<p align="center">
  <img src="https://via.placeholder.com/800x400?text=Insert+YOLO+Detection+GIF+Here" alt="demo1"/>
</p>

### 🚨 Emergency Stop Trigger
<p align="center">
  <img src="https://via.placeholder.com/800x400?text=Insert+ESP32+Response+GIF+Here" alt="demo2"/>
</p>

> ⚠️ Replace the placeholders above with your actual demo GIFs (drag & drop into GitHub or use `/assets/demo.gif`)

---

## 📝 Project Overview

**Eye-O-T Pilot** is an Edge AI monitoring system designed to detect and intervene during driver fatigue events in real-time. The system uses a custom-trained **YOLOv11 Nano** model to analyze ocular states and applies temporal logic to categorize risk levels.

By processing data locally, the system eliminates cloud latency, ensuring immediate autonomous action to stop the vehicle during critical sleep states.

---

## 🚨 The Real-World Problem

Driver fatigue contributes to nearly **20% of road accidents**.

Traditional IoT systems depend on cloud processing → **delays = danger**  
**Eye-O-T Pilot solves this by moving intelligence directly to the Edge.**

---

## 🧠 Technical Stack & Intelligence

- **Model:** YOLOv11n (`best_v3_1.pt`)
- **Framework:** Ultralytics YOLO
- **Languages:** Python 3.10+ & C++ (Arduino)
- **Architecture:** Edge AI + IoT Actuation (ESP32)

---

## 🧪 Detection Logic

The system measures eye closure duration to distinguish between normal blinking and fatigue:

| Category | Duration (s) | Description | Risk | Action |
|----------|-------------|-------------|------|--------|
| **Awake** | 0s | Normal | Low | Green LED (Drive) |
| **Blinking** | < 2.0s | Natural blink | Low | Green LED |
| **Mild Drowsiness** | 2.0 – 4.9s | Fatigue onset | Medium | Yellow LED |
| **Sleeping** | ≥ 5.0s | Critical | 🔴 High | **Emergency Stop** |

---

## 🛠️ Hardware Setup

- ESP32 DevKit V1  
- L298N Motor Driver  
- 2x DC Gear Motors  
- DFPlayer Mini + 3W Speaker  
- LED Traffic Light Module  
- 2x 18650 Li-ion Batteries (7.4V)

---

## 🔌 Hardware Port Mapping

| Component | ESP32 Pin | Device Pin |
|----------|----------|------------|
| **DFPlayer Mini** | VIN / GND | VCC / GND |
| | D26 | TX |
| | D27 | RX (1kΩ resistor) |
| **Motor Driver** | D18 / D19 | IN1 / IN2 |
| | D22 / D23 | IN3 / IN4 |
| **Traffic Light** | D13 / D12 / D14 | Red / Yellow / Green |

---

## 🚀 Installation & Setup

### 1️⃣ Python (Edge Inference)

```bash
python -m venv venv
.\venv\Scripts\activate
pip install ultralytics opencv-python torch numpy
```

---

### 2️⃣ ESP32 (Autonomous System)

**SD Card Setup**
- Format: FAT32  
- Create folder: `mp3`  
- Add:
```
0001.mp3
0002.mp3
...
0006.mp3
```

**Upload**
- Flash `.ino` file to ESP32

**Communication Endpoints**
```
/alert
/mild
/reset
```

---

## 🏗️ System Architecture

```
[ Camera ]
     ↓
[ YOLOv11 Model (Edge) ]
     ↓
[ Fatigue Logic Engine ]
     ↓
[ ESP32 Controller ]
     ↓
[ Motors + LEDs + Audio Alert ]
```

---

## 📂 Repository Structure

```
/SourceCode
   ├── esp32_controller.ino
   └── detection.py

/ML_Model
   └── best_v3_1.pt

/Documentation
   └── infographic.pdf
```

---

## 🌟 Key Features

✔ Real-time fatigue detection  
✔ Fully offline (Edge AI)  
✔ Autonomous emergency stop  
✔ Low-latency decision system  
✔ Hardware-integrated safety response  

---

## 🔮 Future Improvements

- CAN Bus vehicle integration  
- Night vision / IR support  
- Biometric sensors (heart rate, EEG)  
- Mobile dashboard (real-time monitoring)  

---

## 👥 Team

- Marck Anthony Sabado  
- Sakira Julian  
- Mark Angel Manzanillo  
- Jazel Marithe Vintero  

---

## 📌 Project Insight

> “When milliseconds matter, the cloud is too far.”

**Eye-O-T Pilot proves that Edge AI is not just faster — it’s safer.**

---

## ⭐ Support

If you find this project interesting, consider giving it a ⭐ on GitHub!
