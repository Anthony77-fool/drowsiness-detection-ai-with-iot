import torch
from matplotlib import pyplot as plt
import numpy as np #array transformation
import cv2
from ultralytics import YOLO

# Load a pretrained YOLOv8 small model
model = YOLO('best_v3_1.pt') #Nano - Fastest, best for my laptop's health

# Display model information
model.info()

#video capture (0) is my webcam port number
cap = cv2.VideoCapture(0)

#while the cam is open, loop on it
while cap.isOpened():

    #this one returns the frame of the webcam
    #which is basically the image
    ret, frame = cap.read()

    # 1. SCAN THE WEBCAM FRAME: This is the "Brain" working in real-time
    # We use a smaller imgsz (640) for the webcam to keep it fast
    results_webcam = model(frame, imgsz=640, conf=0.3)

    # 2. PLOT THE RESULTS: This draws the boxes and labels on the frame
    # .plot() returns a BGR image with boxes already drawn
    annotated_frame = results_webcam[0].plot()

    #shows ('text', image capture)
    cv2.imshow('Drowsiness Detection', annotated_frame)

    #if q is pressed, break the loop
    if cv2.waitKey(10) & 0xFF == ord('q'):
        break

#caches the break loop, and closes the camera
cap.release()
cv2.destroyAllWindows()