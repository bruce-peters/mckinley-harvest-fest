#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

// Name shown in the computer's Bluetooth settings
const char *DEVICE_NAME = "Play Button";

const uint8_t BUTTON_PIN = 4; // button wired between this pin and GND
const uint8_t LED_PIN = 2;    // onboard LED, on while connected
const uint32_t DEBOUNCE_MS = 30;

// Key sent on each press. Space is understood by phones, tablets and
// computers alike, and the player page listens for it.
const uint8_t KEY_SPACE = 0x2C;

// HID report descriptor: a standard keyboard (modifiers, reserved, 6 keys)
const uint8_t REPORT_ID = 1;
const uint8_t REPORT_MAP[] = {
  0x05, 0x01,      // Usage Page (Generic Desktop)
  0x09, 0x06,      // Usage (Keyboard)
  0xA1, 0x01,      // Collection (Application)
  0x85, REPORT_ID, //   Report ID
  0x05, 0x07,      //   Usage Page (Keyboard)
  0x19, 0xE0,      //   Usage Minimum (Left Control)
  0x29, 0xE7,      //   Usage Maximum (Right GUI)
  0x15, 0x00,      //   Logical Minimum (0)
  0x25, 0x01,      //   Logical Maximum (1)
  0x75, 0x01,      //   Report Size (1)
  0x95, 0x08,      //   Report Count (8)
  0x81, 0x02,      //   Input (Data, Variable, Absolute), modifier keys
  0x75, 0x08,      //   Report Size (8)
  0x95, 0x01,      //   Report Count (1)
  0x81, 0x01,      //   Input (Constant), reserved byte
  0x95, 0x06,      //   Report Count (6)
  0x15, 0x00,      //   Logical Minimum (0)
  0x25, 0x73,      //   Logical Maximum (F24)
  0x19, 0x00,      //   Usage Minimum (0)
  0x29, 0x73,      //   Usage Maximum (F24)
  0x81, 0x00,      //   Input (Data, Array), up to 6 keys held
  0xC0             // End Collection
};

NimBLECharacteristic *inputReport = nullptr;
volatile bool connected = false;

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *server, NimBLEConnInfo &connInfo) override {
    connected = true;
  }

  void onDisconnect(NimBLEServer *server, NimBLEConnInfo &connInfo, int reason) override {
    connected = false;
    NimBLEDevice::startAdvertising();
  }
};

void sendKey(uint8_t key);

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  NimBLEDevice::init(DEVICE_NAME);
  // Bonding lets the computer reconnect without pairing again
  NimBLEDevice::setSecurityAuth(true, false, true);

  NimBLEServer *server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  NimBLEHIDDevice *hid = new NimBLEHIDDevice(server);
  inputReport = hid->getInputReport(REPORT_ID);
  hid->setManufacturer("Bruce Peters");
  hid->setPnp(0x02, 0x303A, 0x0001, 0x0100);
  hid->setHidInfo(0x00, 0x01);
  hid->setReportMap((uint8_t *)REPORT_MAP, sizeof(REPORT_MAP));
  hid->setBatteryLevel(100);
  server->start();

  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  advertising->setAppearance(0x03C1); // HID keyboard
  advertising->addServiceUUID(hid->getHidService()->getUUID());
  advertising->setName(DEVICE_NAME);
  advertising->start();

  Serial.println("Advertising, waiting for the computer to connect");
}

void loop() {
  static bool lastReading = HIGH;
  static bool buttonState = HIGH;
  static uint32_t lastChange = 0;

  digitalWrite(LED_PIN, connected);

  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastReading) {
    lastReading = reading;
    lastChange = millis();
  }

  if (reading != buttonState && millis() - lastChange >= DEBOUNCE_MS) {
    buttonState = reading;
    // LOW means pressed because of the pull-up
    if (buttonState == LOW) {
      sendKey(KEY_SPACE);
    }
  }
}

void sendKey(uint8_t key) {
  if (!connected) {
    Serial.println("Button pressed, but no computer is connected");
    return;
  }

  // Report layout: modifiers, reserved, then up to six key codes
  uint8_t pressed[8] = {0, 0, key, 0, 0, 0, 0, 0};
  uint8_t released[8] = {0};
  inputReport->setValue(pressed, sizeof(pressed));
  inputReport->notify();
  delay(20);
  inputReport->setValue(released, sizeof(released));
  inputReport->notify();

  Serial.println("Sent key press");
}
