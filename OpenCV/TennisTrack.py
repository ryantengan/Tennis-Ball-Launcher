import cv2
import numpy as np
import time  # Added for FPS throttle

# Open ESP IP Web Server
cap = cv2.VideoCapture('http://192.168.2.214/')

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # ✅ Resize to reduce load — adjust scale as needed
    frame = cv2.resize(frame, (0, 0), fx=5, fy=5)

    # Blur to reduce noise
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)

    # Convert to HSV color space
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

    # Define range for tennis ball green-yellow
    lower_yellow = np.array([25, 80, 80])
    upper_yellow = np.array([40, 255, 255])

    # Create mask
    mask = cv2.inRange(hsv, lower_yellow, upper_yellow)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)

    # Find contours
    contours, _ = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # Proceed if we found something
    if len(contours) > 0:
        # Find largest contour
        c = max(contours, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)

        if radius > 10:
            # Draw circle and centroid
            cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)
            cv2.putText(frame, "Tennis Ball", (int(x - radius), int(y - radius)),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)

    cv2.imshow("Tennis Ball Tracking", frame)

    # ✅ Add small delay to throttle to ~10 FPS
    time.sleep(0.1)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
