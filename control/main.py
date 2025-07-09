import tkinter as tk
import os
from PIL import Image, ImageTk
import cv2
import numpy as np

class Dashboard:
    def __init__(self):
        self.window = tk.Tk()
        self.window.title('Tennis Ball Robot')
        self.window.bind('<Escape>', lambda event: self.on_close())

        self.cap = cv2.VideoCapture(0)

        self.state_v = None
        self.mode_v = None
        self.controls_img = None

        self.layout_init()
        self.process_frame()

    def layout_init(self):
        main = tk.Frame(self.window, bg='black')

        main.rowconfigure(0, weight=10, uniform='equal')
        main.rowconfigure(1, weight=5, uniform='equal')
        main.columnconfigure(0, weight=14, uniform='equal')
        main.columnconfigure(1, weight=10, uniform='equal')

        # Info panel - State and Mode
        info_panel = tk.Frame(main, bg='black')
        info_panel.rowconfigure(0, weight=5, uniform='equal')
        info_panel.rowconfigure(1, weight=20, uniform='equal')
        info_panel.columnconfigure(0, weight=15, uniform='equal')
        info_panel.columnconfigure(1, weight=1, uniform='equal')
        info_panel.columnconfigure(2, weight=15, uniform='equal')

        # Headers
        tk.Label(info_panel, font=('Trebuchet MS', 25), text='STATE', anchor='w', bg='black', fg='light goldenrod').grid(row=0, column=0, sticky='nsew')
        tk.Label(info_panel, font=('Trebuchet MS', 25), text='MODE', anchor='w', bg='black', fg='light goldenrod').grid(row=0, column=2, sticky='nsew')
        # Spacer
        tk.Frame(info_panel, bg='black').grid(row=0, rowspan=2, column=1)
        # Values
        self.state_v = tk.Label(info_panel, font=('Trebuchet MS', 50), bg='gray10', fg='thistle2')
        self.state_v.grid(row=1, column=0, sticky='nsew')
        self.mode_v = tk.Label(info_panel, font=('Trebuchet MS', 50), bg='gray10', fg='thistle2')
        self.mode_v.grid(row=1, column=2, sticky='nsew')

        # Image with controls
        img = Image.open(os.path.join(os.path.dirname(__file__), 'assets', 'controls.png'))        
        self.controls_img = ImageTk.PhotoImage(img)
        tk.Label(main, bg='black', image=self.controls_img).grid(row=1, column=1, sticky='nsew')

        info_panel.grid(row=1, column=0, sticky='nsew', padx=15, pady=15)
        main.pack(fill='both', expand=True)

        # Camera stream
        self.video_label = tk.Label(main, bg='black')
        self.video_label.grid(row=0, columnspan=2, sticky='nsew')

    def process_frame(self):
        ret, frame = self.cap.read()
        if not ret:
            return

        # Tennis ball detection
        blurred = cv2.GaussianBlur(frame, (11, 11), 0)
        hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
        lower_yellow = np.array([25, 80, 80])
        upper_yellow = np.array([40, 255, 255])
        mask = cv2.inRange(hsv, lower_yellow, upper_yellow)
        mask = cv2.erode(mask, None, iterations=2)
        mask = cv2.dilate(mask, None, iterations=2)
        contours, _ = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        # Draw circle around ball
        if len(contours) > 0:
            c = max(contours, key=cv2.contourArea)
            ((x, y), radius) = cv2.minEnclosingCircle(c)

            if radius > 10:
                cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)
                cv2.putText(frame, "Tennis Ball", (int(x - radius), int(y - radius)),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
                
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        img = Image.fromarray(frame_rgb)
        imgtk = ImageTk.PhotoImage(image=img)

        self.video_label.imgtk = imgtk
        self.video_label.config(image=imgtk)

        self.window.after(10, self.process_frame)

    def on_close(self):
        self.cap.release()
        self.window.destroy()

if __name__ == '__main__':
    dashboard = Dashboard()
    # dashboard.window.state('zoomed')
    dashboard.window.attributes('-fullscreen', True)
    dashboard.window.mainloop()