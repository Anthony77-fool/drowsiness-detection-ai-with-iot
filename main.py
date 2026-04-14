import torch
from matplotlib import pyplot as plt
import numpy as np 
import cv2
from ultralytics import YOLO
import time  # Essential for measuring actual seconds

# Load a pretrained YOLOv8 small model
model = YOLO('best_v3_1.pt') 

# Display model information
model.info()

# Variables for timing logic
closed_start_time = None
status = "AWAKE"
color = (0, 255, 0)  # Green for Awake

# Video capture (0) is my webcam port number
cap = cv2.VideoCapture(0)

# While the cam is open, loop on it
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    # 1. SCAN THE WEBCAM FRAME
    results_webcam = model(frame, imgsz=640, conf=0.3)

    # --- LOGIC FOR CAPTURING CLOSED VS OPEN EYES ---
    # Extract detected class IDs from the results
    detected_classes = results_webcam[0].boxes.cls.cpu().numpy()

    # Check if 'Closed_Eyes' (Class 0) is detected in the frame
    if 0 in detected_classes:
        if closed_start_time is None:
            closed_start_time = time.time()  # Record the moment eyes closed
        
        # Calculate elapsed time
        elapsed = time.time() - closed_start_time

        # Categorize based on duration
        if elapsed >= 5.0:
            status = "SLEEPING"
            color = (0, 0, 255)  # Red
        elif elapsed >= 2.0:
            status = "MILD DROWSINESS"
            color = (0, 255, 255)  # Yellow
        else:
            status = "BLINKING/ACTIVE"
            color = (0, 255, 0) # Green
    else:
        # Reset if eyes are open (Class 1) or no eyes detected
        closed_start_time = None
        status = "AWAKE"
        color = (0, 255, 0)

    # 2. PLOT THE RESULTS
    annotated_frame = results_webcam[0].plot()

    # Overlay the category text on the screen
    cv2.putText(annotated_frame, f"STATE: {status}", (20, 50), 
                cv2.FONT_HERSHEY_SIMPLEX, 1, color, 3)

    # Shows ('text', image capture)
    cv2.imshow('Drowsiness Detection', annotated_frame)

    # If q is pressed, break the loop
    if cv2.waitKey(10) & 0xFF == ord('q'):
        break

# Caches the break loop, and closes the camera
cap.release()
cv2.destroyAllWindows()