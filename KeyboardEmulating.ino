//Here you can find the code for the keybord key
//https://learn.microsoft.com/it-it/windows/configuration/keyboard-filter/keyboardfilter-key-names
#define US_KEYBOARD 1

#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"

// Change the below values if desired
#define BUTTON_PIN 14
#define DEVICE_NAME "ESP32 Keyboard"
#define MESSAGE "Hello from ESP32\n"  // Message to type

// Forward declarations
void bluetoothTask(void *);
void pressFunctionKey(uint8_t key);
void typeText(const char *text);

bool isBleConnected = false;

void setup()
{
  Serial.begin(9600);

  // Configure pin for button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Start Bluetooth task
  xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);
}

void loop()
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    if (isBleConnected)
    {
      // Example: Press the Windows key
      pressFunctionKey(0xE3);  // 0xE3 is the HID code for the Windows key
      //typeText(MESSAGE); // Uncomment this part if you want write text (Here you can find the code for the keybord key https://learn.microsoft.com/it-it/windows/configuration/keyboard-filter/keyboardfilter-key-names)
    }
  }

  delay(100);
}

// Message (report) sent when a key is pressed or released
struct InputReport
{
  uint8_t modifiers;      // bitmask: CTRL = 1, SHIFT = 2, ALT = 4, GUI (Windows) = 8
  uint8_t reserved;       // must be 0
  uint8_t pressedKeys[6]; // up to six concurrently pressed keys
};

// Message (report) received when an LED's state changed
struct OutputReport
{
  uint8_t leds; // bitmask: num lock = 1, caps lock = 2, scroll lock = 4, compose = 8, kana = 16
};

// The report map describes the HID device (a keyboard in this case) and
// the messages (reports in HID terms) sent and received.
static const uint8_t REPORT_MAP[] = {
    USAGE_PAGE(1), 0x01,      // Generic Desktop Controls
    USAGE(1), 0x06,           // Keyboard
    COLLECTION(1), 0x01,      // Application
    REPORT_ID(1), 0x01,       //   Report ID (1)
    USAGE_PAGE(1), 0x07,      //   Keyboard/Keypad
    USAGE_MINIMUM(1), 0xE0,   //   Keyboard Left Control
    USAGE_MAXIMUM(1), 0xE7,   //   Keyboard Right Control
    LOGICAL_MINIMUM(1), 0x00, //   Each bit is either 0 or 1
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_COUNT(1), 0x08, //   8 bits for the modifier keys
    REPORT_SIZE(1), 0x01,
    HIDINPUT(1), 0x02,     //   Data, Var, Abs
    REPORT_COUNT(1), 0x01, //   1 byte (unused)
    REPORT_SIZE(1), 0x08,
    HIDINPUT(1), 0x01,     //   Const, Array, Abs
    REPORT_COUNT(1), 0x06, //   6 bytes (for up to 6 concurrently pressed keys)
    REPORT_SIZE(1), 0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x65, //   101 keys
    USAGE_MINIMUM(1), 0x00,
    USAGE_MAXIMUM(1), 0x65,
    HIDINPUT(1), 0x00,     //   Data, Array, Abs
    REPORT_COUNT(1), 0x05, //   5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
    REPORT_SIZE(1), 0x01,
    USAGE_PAGE(1), 0x08,    //   LEDs
    USAGE_MINIMUM(1), 0x01, //   Num Lock
    USAGE_MAXIMUM(1), 0x05, //   Kana
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    HIDOUTPUT(1), 0x02,    //   Data, Var, Abs
    REPORT_COUNT(1), 0x01, //   3 bits (Padding)
    REPORT_SIZE(1), 0x03,
    HIDOUTPUT(1), 0x01, //   Const, Array, Abs
    END_COLLECTION(0)   // End application collection
};

BLEHIDDevice *hid;
BLECharacteristic *input;
BLECharacteristic *output;

const InputReport NO_KEY_PRESSED = {};

/*
 * Callbacks related to BLE connection
 */
class BleKeyboardCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *server)
  {
    isBleConnected = true;

    // Allow notifications for characteristics
    BLE2902 *cccDesc = (BLE2902 *)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    cccDesc->setNotifications(true);

    Serial.println("Client has connected");
  }

  void onDisconnect(BLEServer *server)
  {
    isBleConnected = false;

    // Disallow notifications for characteristics
    BLE2902 *cccDesc = (BLE2902 *)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    cccDesc->setNotifications(false);

    Serial.println("Client has disconnected");
  }
};

/*
 * Called when the client (computer, smart phone) wants to turn on or off
 * the LEDs in the keyboard.
 *
 * bit 0 - NUM LOCK
 * bit 1 - CAPS LOCK
 * bit 2 - SCROLL LOCK
 */
class OutputCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *characteristic)
  {
    OutputReport *report = (OutputReport *)characteristic->getData();
    Serial.print("LED state: ");
    Serial.print((int)report->leds);
    Serial.println();
  }
};

void bluetoothTask(void *)
{
  // Initialize the device
  BLEDevice::init(DEVICE_NAME);
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new BleKeyboardCallbacks());

  // Create an HID device
  hid = new BLEHIDDevice(server);
  input = hid->inputReport(1);   // report ID
  output = hid->outputReport(1); // report ID
  output->setCallbacks(new OutputCallbacks());

  // Set manufacturer name
  hid->manufacturer()->setValue("Maker Community");
  // Set USB vendor and product ID
  hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  // Information about HID device: device is not localized, device can be connected
  hid->hidInfo(0x00, 0x02);

  // Security: device requires bonding
  BLESecurity *security = new BLESecurity();
  security->setAuthenticationMode(ESP_LE_AUTH_BOND);

  // Set report map
  hid->reportMap((uint8_t *)REPORT_MAP, sizeof(REPORT_MAP));
  hid->startServices();

  // Set battery level to 100%
  hid->setBatteryLevel(100);

  // Advertise the services
  BLEAdvertising *advertising = server->getAdvertising();
  advertising->setAppearance(HID_KEYBOARD);
  advertising->addServiceUUID(hid->hidService()->getUUID());
  advertising->addServiceUUID(hid->deviceInfo()->getUUID());
  advertising->addServiceUUID(hid->batteryService()->getUUID());
  advertising->start();

  Serial.println("BLE ready");
  delay(portMAX_DELAY);
};

// Function to emulate a function key press (e.g., Windows key)
void pressFunctionKey(uint8_t key)
{
  // Create input report for the function key
  InputReport report = {
      .modifiers = 0x08,  // Modifier for the Windows key (GUI)
      .reserved = 0,
      .pressedKeys = {
          key,  // HID code for the function key (e.g., 0xE3 for Windows key)
          0, 0, 0, 0, 0}};

  // Send the input report
  input->setValue((uint8_t *)&report, sizeof(report));
  input->notify();

  delay(50);  // Simulate key press duration

  // Release the key
  input->setValue((uint8_t *)&NO_KEY_PRESSED, sizeof(NO_KEY_PRESSED));
  input->notify();
}

// Function to type a text message
void typeText(const char *text)
{
  int len = strlen(text);
  for (int i = 0; i < len; i++)
  {
    // Translate character to key combination
    uint8_t val = (uint8_t)text[i];
    if (val > KEYMAP_SIZE)
      continue; // Character not available on keyboard - skip
    KEYMAP map = keymap[val];

    // Create input report
    InputReport report = {
        .modifiers = map.modifier,
        .reserved = 0,
        .pressedKeys = {
            map.usage,
            0, 0, 0, 0, 0}};

    // Send the input report
    input->setValue((uint8_t *)&report, sizeof(report));
    input->notify();

    delay(5);

    // Release all keys between two characters
    input->setValue((uint8_t *)&NO_KEY_PRESSED, sizeof(NO_KEY_PRESSED));
    input->notify();

    delay(5);
  }
}
