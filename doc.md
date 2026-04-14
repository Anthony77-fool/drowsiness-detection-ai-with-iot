# Real-Time Drowsiness Detection System (YOLOv11)

## Project Overview
This project implements an intelligent monitoring system designed to detect driver fatigue in real-time. Using a custom-trained **YOLOv11 Nano** model, the system analyzes eye states (Open vs. Closed) and applies temporal logic to categorize risk levels—ranging from normal blinking to critical sleep.

## Technical Stack
* **Model:** YOLOv11n (Custom fine-tuned `best_v3_1.pt`)
* **Framework:** Ultralytics YOLOv8/v11
* **Language:** Python 3.10+
* **Libraries:** `opencv-python`, `torch`, `time`, `numpy`

---

## Detection Logic & Safety Criteria
The system does not simply detect a closed eye; it measures the **duration of closure** to distinguish between natural physiological blinks and dangerous fatigue.

| Category | Duration (s) | Description | Risk Level |
| :--- | :--- | :--- | :--- |
| **Awake** | 0s | Eyes are open or normal blinking occurs. | Low |
| **Blinking/Active** | < 2.0s | Detected eye closure is within natural parameters. | Low |
| **Mild Drowsiness** | 2.0s - 4.9s | Significant eye closure duration; early signs of fatigue. | Medium |
| **Sleeping** | ≥ 5.0s | Critical state. Immediate intervention or alert required. | High |

---

## Installation & Setup

### 1. Environment Setup
It is highly recommended to use a virtual environment to avoid dependency conflicts.
```bash
python -m venv venv
.\venv\Scripts\activate  # Windows