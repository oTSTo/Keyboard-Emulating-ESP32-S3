# Keyboard-Emulating-ESP32-S3
<p align="center">
  <a href="https://opensource.org/licenses/MIT">
    <img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT">
  </a>
</p>

## Description
The code creates a bluetooth bispositive that emulates the keyboard
## Features
| **Feature**                  | **Description**                                                                 |
|------------------------------|---------------------------------------------------------------------------------|
| **Emulation of a specific key** | You can emulate a specific key for example `0xE3`<-- is the windwos key      |
| **Writing text**                | You can emulate the keybord for writing text                                 |

## MUST-HAVE
- [Arduino IDE](https://www.arduino.cc/en/software)
- ESP32-S3

## WHAT YOU CAN CHANGE
Inside the code you can change
- The button pin for trigger the emulation
- The name that the bluetooth device will have
- The message you will send
- Ande the specific key you want emulate
```cpp
#define BUTTON_PIN 
#define DEVICE_NAME "ESP32 Keyboard"
#define MESSAGE "Hello from ESP32\n"  // Message to type
// The rest of the code

void loop()
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    if (isBleConnected)
    {
      // Example: Press the Windows key
      pressFunctionKey(0xE3);  // 0xE3 is the HID code for the Windows key
      //typeText(MESSAGE);
    }
  }
}
```

## ESSENTIAL LIBRARIES
- [BLEDevice.h](https://github.com/oTSTo/Keyboard-Emulating-ESP32-S3/blob/7278381af4591d3620dbfaab736fa532a36a32d1/Libraries/BLEDevice.h)
- [HIDTypes.h](https://github.com/oTSTo/Keyboard-Emulating-ESP32-S3/blob/7278381af4591d3620dbfaab736fa532a36a32d1/Libraries/HIDTypes.h)
- [USBAPI.h](https://github.com/oTSTo/Keyboard-Emulating-ESP32-S3/blob/7278381af4591d3620dbfaab736fa532a36a32d1/Libraries/USBAPI.h)
