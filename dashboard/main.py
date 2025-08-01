import tkinter as tk
import os
from PIL import Image, ImageTk
import cv2
import numpy as np
import threading
import asyncio
from bleak import BleakClient, BleakScanner

class Dashboard:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title('Tennis Ball Robot')

        self.cap = cv2.VideoCapture(0)

        self.state_v = None
        self.mode_v = None
        self.controls_img = None

        # Bluetooth / Controls
        self.root.bind('<Escape>', lambda event: self.on_close())
        for key in ['<Left>', '<Right>', '<Up>', '<Down>', '<p>', '<l>', '<P>', '<L>']:
            self.root.bind(key, self.on_key)
            self.root.bind(f'<KeyRelease-{key[1:-1]}>', self.on_key_release)
        self.ble_client = None
        self.ble_loop = asyncio.new_event_loop()
        threading.Thread(target=self.run_ble_loop, daemon=True).start()

        self.layout_init()
        self.process_cam()

    def layout_init(self):
        main = tk.Frame(self.root, bg='black')

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
        self.state_v = tk.Label(info_panel, text='Startup', font=('Trebuchet MS', 50), bg='gray10', fg='thistle2')
        self.state_v.grid(row=1, column=0, sticky='nsew')
        self.mode_v = tk.Label(info_panel, font=('Trebuchet MS', 20), bg='gray10', fg='thistle2')
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

    def process_cam(self):
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
                cv2.putText(frame, 'Tennis Ball', (int(x - radius), int(y - radius)),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
                
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        img = Image.fromarray(frame_rgb)
        imgtk = ImageTk.PhotoImage(image=img)

        self.video_label.imgtk = imgtk
        self.video_label.config(image=imgtk)

        self.root.after(10, self.process_cam)

    def ble_connect(self):
        DEVICE_NAME = 'Tennis Bot'

        async def connect():
            self.mode_v.config(text='Scanning for BLE devices...')
            devices = await BleakScanner.discover(timeout=5.0)

            for d in devices:
                if d.name == DEVICE_NAME:
                    print("Found device:", d.name, d.address)
                    self.ble_client = BleakClient(d)

                    await asyncio.sleep(1)

                    await self.ble_client.connect()
                    self.mode_v.config(text=f'Connected to {DEVICE_NAME}')
                    services = self.ble_client.services
                    for s in services:
                        print("Service:", s)
                    return

            self.mode_v.config(text=f"Device not found")

        self.ble_loop.call_soon_threadsafe(asyncio.create_task, connect())

    def run_ble_loop(self):
        asyncio.set_event_loop(self.ble_loop)
        self.ble_loop.run_forever()

    def on_key(self, event):
        key = event.keysym
        if key == 'p' or key == 'l':
            key = key.upper()
        match key:
            case 'Left':
                self.state_v.config(text='Turn left')
            case 'Right':
                self.state_v.config(text='Turn right') 
            case 'P':
                self.state_v.config(text='Pickup')
            case 'L':
                self.state_v.config(text='Launch')
            case _:
                self.state_v.config(text=f'{key}')
        if self.ble_client and self.ble_client.is_connected:
            msg = key.encode()

            async def send():
                try:
                    await self.ble_client.write_gatt_char('0000dead-0000-1000-8000-00805f9b34fb', msg)
                    print('Sent:', key)
                except Exception as e:
                    print('BLE send error:', e)

            self.ble_loop.call_soon_threadsafe(asyncio.create_task, send())

    def on_key_release(self, event):
        self.state_v.config(text=f'Stop')
        if self.ble_client and self.ble_client.is_connected:
            msg = 'Stop'.encode()

            async def send():
                try:
                    await self.ble_client.write_gatt_char('0000dead-0000-1000-8000-00805f9b34fb', msg)
                    print('Sent: Stop')
                except Exception as e:
                    print('BLE send error:', e)

            self.ble_loop.call_soon_threadsafe(asyncio.create_task, send())

    def on_close(self):
        self.cap.release()
        self.root.destroy()

if __name__ == '__main__':
    dashboard = Dashboard()
    threading.Thread(target=dashboard.ble_connect).start()
    dashboard.root.state('zoomed')
    # dashboard.root.attributes('-fullscreen', True)
    dashboard.root.mainloop()