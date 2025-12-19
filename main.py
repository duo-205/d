import time
import cv2
from ultralytics import YOLO
import serial

MODEL_PATH = "best.pt"
VIDEO_PATH = "test1.mp4"
SERIAL_PORT = "COM4"
SERIAL_BAUD = 115200
CONFIDENCE_THRESHOLD = 0.5
NMS_IOU = 0.45

ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=1)
time.sleep(2)  

model = YOLO(MODEL_PATH)
cap = cv2.VideoCapture(VIDEO_PATH)

track_pos = None
still_start = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break

    results = model(frame, conf=CONFIDENCE_THRESHOLD, iou=NMS_IOU)
    dets = []

    r = results[0]
    if hasattr(r, 'boxes') and len(r.boxes) > 0:
        for b in r.boxes:
            xyxy = b.xyxy[0].cpu().numpy()
            cls_idx = int(b.cls[0])
            name = model.names[cls_idx]
            dets.append({'class': name.lower(), 'box': xyxy})

    target = None
    h, w, _ = frame.shape
    for d in dets:
        if d['class'] == 'in_water2':
            x1, y1, x2, y2 = d['box']
            cx = (x1 + x2) / 2
            cy = (y1 + y2) / 2
            target = (cx, cy)
            break

    alert_trigger = False
    now = time.time()

    if target is not None:
        if track_pos is None:
            track_pos = target
            still_start = now
        else:
            dx = abs(target[0] - track_pos[0])
            dy = abs(target[1] - track_pos[1])
            if dx < 15 and dy < 15:
                if now - still_start >= 3:  
                    alert_trigger = True
            else:
                still_start = now
            track_pos = target
    else:
        track_pos = None
        still_start = now

    if alert_trigger:
        print("Sending alert signal...")
        ser.write(b'1')  
        alert_trigger = False  

    for d in dets:
        x1, y1, x2, y2 = d['box']
        cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), (0, 0, 255), 2)
        cv2.putText(frame, d['class'], (int(x1), int(y1)-5),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)

    cv2.imshow("Drowning Monitor", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
