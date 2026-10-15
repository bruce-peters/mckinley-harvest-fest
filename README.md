# McKinley Harvest Fest: Play Button Player

A physical button that starts a video. An ESP32 "button box" pairs with a computer, phone or tablet as a
Bluetooth keyboard and sends **Space** when the button is pressed. A single web page waits on a black screen
(it is meant to be projected), plays the chosen video from the start on each press, and goes black again
when the video ends.

**Live page:** https://bruce-peters.github.io/mckinley-harvest-fest/

## How to use it

1. Power on the button box. On the device that will show the video, open the **system Bluetooth
   settings** and connect to **"Play Button"**. The board's onboard LED lights while it is connected.
2. Open the live page and click **Choose video** to pick a video file from that device.
3. On a computer, press **F11** for fullscreen. The screen stays black and the mouse pointer is hidden.
4. Press the button. The video plays from the start. When it ends, the screen goes black until the next press.

Other controls:
- **Space** or **Enter** on a normal keyboard, or a tap/click on the screen, does the same as the button.
- **C** opens the file chooser to swap the video.
- A press during playback restarts the video (`RESTART_WHILE_PLAYING` in `index.html`).
- The chosen video is stored in the browser's IndexedDB on that device, so it survives a page reload.
  Nothing is uploaded.
- On phones and tablets the page asks the browser to keep the screen awake (Screen Wake Lock API, where supported).

Browser support: the page does **not** use Web Bluetooth. The button pairs through the operating system as an
ordinary keyboard, so any modern browser works (Chrome, Edge, Firefox, Safari). The video format has to be
one the browser can play (MP4/H.264 or WebM are safe choices).

If the first press shows "Tap the screen once to allow sound", the browser blocked audio autoplay. Tap or
click once, then use the button.

## Run the page locally

It is a single static `index.html` with no build step:

```bash
git clone https://github.com/bruce-peters/mckinley-harvest-fest.git
cd mckinley-harvest-fest
npx http-server . -p 8080      # or: python3 -m http.server 8080
```

Open http://localhost:8080/.

## Button box firmware (`McKinley Project/`)

PlatformIO project for an ESP32 that acts as a Bluetooth LE HID keyboard (NimBLE-Arduino).

Hardware:
- Any ESP32 dev board (PlatformIO board `esp32dev`)
- A momentary push button wired between **GPIO 4** and **GND** (the pin uses the internal pull-up; presses are debounced at 30 ms)
- The onboard LED on **GPIO 2** shows the connection state

Build and flash (needs Python 3):

```bash
pip install platformio
cd "McKinley Project"
pio run                        # compile
pio run -t upload              # flash over USB
pio device monitor -b 115200   # logs "Sent key press" on each press
```

`pio run` builds cleanly (tested with PlatformIO 6.2.0, espressif32 7.1.3, NimBLE-Arduino 2.5.1). Flashing and
pairing were not tested here because they need the board.

To change the device name, the pins or the key that is sent, edit the constants at the top of
`McKinley Project/src/main.cpp` (`DEVICE_NAME`, `BUTTON_PIN`, `LED_PIN`, `KEY_SPACE`). If you change the key,
also update `TRIGGER_KEYS` in `index.html`.

## Project structure

```
index.html                    the player page (served by GitHub Pages)
McKinley Project/platformio.ini   ESP32 board + NimBLE-Arduino dependency
McKinley Project/src/main.cpp     BLE keyboard firmware for the button box
```
