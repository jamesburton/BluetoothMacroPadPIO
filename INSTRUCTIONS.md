# Bluetooth Keyboard Macro Pad - Implementation Instructions

## Target Hardware
**ESP32-4848S040C_I** - Sunton 4.0" 480x480 Touch Display
- **Board:** ESP32-S3-DevKitC-1
- **Display:** 4.0" 480x480 IPS (ST7701S Driver via 3-wire SPI + RGB)
- **Touch Controller:** GT911 Capacitive Touch
- **Memory:** PSRAM enabled
- **Bluetooth:** Built-in ESP32-S3 BLE

## Reference Project
Use **c:\Development\PlatformIO\SnakePIO** as the reference implementation for:
- PlatformIO configuration (platformio.ini)
- Library dependencies (LovyanGFX 1.2.7)
- Platform version (espressif32@6.9.0)
- Display initialization and pinout (ST7701S setup)
- Touch controller setup (GT911)
- Memory configuration (PSRAM settings)
- Build flags and partition scheme

## Application Overview
Touch-based Bluetooth HID keyboard for custom macros:
- Grid of customizable buttons
- Send keyboard shortcuts, hotkeys, and text macros
- Act as Bluetooth keyboard for PC/Mac/tablet
- Multiple pages/profiles for different applications
- Visual feedback on button press

## Implementation Requirements

### Core Features
1. **Button Grid Layout**
   - 3x3, 4x4, or 5x4 button grid (customizable)
   - Large touch targets (~100-140px buttons)
   - Button labels (text or icons)
   - Visual state (pressed/released)
   - Haptic feedback (visual highlight)

2. **Macro Types**
   - **Single Key:** Press a key (e.g., F13, Media keys)
   - **Key Combo:** Modifier + key (e.g., Ctrl+C, Ctrl+Shift+T)
   - **Sequence:** Multiple keystrokes in order
   - **Text String:** Type out text (passwords, email, etc.)
   - **Delay:** Wait between keystrokes
   - **Special Keys:** Media controls (Play, Pause, Volume, etc.)

3. **Bluetooth HID**
   - Pair as Bluetooth keyboard
   - Send HID reports for key presses
   - Support modifier keys (Ctrl, Alt, Shift, Win/Cmd)
   - Support media keys and consumer controls
   - Connection status indicator

4. **Configuration**
   - Multiple pages/profiles (swipe to switch)
   - Customizable button labels and actions
   - Save configurations to flash memory
   - Web interface or app for setup (optional)
   - On-device editing (optional, advanced)

### Button Grid Layouts
- **3x3 Grid:** 9 macros, ~150px buttons (large, easy to press)
- **4x4 Grid:** 16 macros, ~110px buttons (good balance)
- **5x4 Grid:** 20 macros, ~90px buttons (more functions, smaller)

### Macro Configuration Structure
```cpp
struct Macro {
    String label;           // Button label (e.g., "Copy", "Paste")
    uint8_t type;           // 0=single key, 1=combo, 2=sequence, 3=text
    uint8_t modifiers;      // Ctrl, Alt, Shift, Win/Cmd bits
    uint8_t keys[10];       // Array of key codes
    String text;            // Text string for type 3
    uint16_t color;         // Button color
};

struct Profile {
    String name;            // Profile name (e.g., "Photoshop", "VS Code")
    Macro buttons[16];      // Array of macros (for 4x4 grid)
};
```

### Common Macro Examples
- **Copy/Paste:** Ctrl+C, Ctrl+V
- **Undo/Redo:** Ctrl+Z, Ctrl+Y
- **Save:** Ctrl+S
- **Screenshot:** Win+Shift+S, Cmd+Shift+4
- **Media:** Play/Pause, Next, Previous, Volume Up/Down
- **Application Switch:** Alt+Tab, Cmd+Tab
- **Virtual Desktops:** Win+Ctrl+Left/Right
- **Developer:** F5 (refresh), F12 (dev tools), Ctrl+Shift+T (reopen tab)
- **Photoshop:** Brush tools, layer shortcuts, etc.
- **Text Expansion:** Email signature, common phrases

### UI Layout Suggestion (480x480, 4x4 Grid)
```
[ Profile: VS Code      BT: ✓ ] (40px header)
┌──────┬──────┬──────┬──────┐
│ Run  │Debug │ Term │ Find │ (110px)
│  F5  │ F10  │Ctrl+`│Ctrl+F│
├──────┼──────┼──────┼──────┤
│ Save │ Undo │ Redo │Format│ (110px)
│Ctrl+S│Ctrl+Z│Ctrl+Y│Alt+F │
├──────┼──────┼──────┼──────┤
│ Copy │ Paste│ Cut  │Duplic│ (110px)
│Ctrl+C│Ctrl+V│Ctrl+X│Ctrl+D│
├──────┼──────┼──────┼──────┤
│Split │Close │  ←   │  →   │ (110px)
│Ctrl+\│Ctrl+W│ Prev │ Next │
└──────┴──────┴──────┴──────┘
[   ←   ] [ Home ] [   →   ] (40px footer)
```

### Technical Considerations
- **BLE HID Library:** Use ESP32-BLE-Keyboard library
- **Key Codes:** Standard USB HID keyboard codes
- **Pairing:** Handle Bluetooth pairing and reconnection
- **Power Management:** Sleep mode when inactive
- **Latency:** Minimize delay between touch and keystroke
- **Button Debouncing:** Prevent accidental double-presses
- **Visual Feedback:** Highlight button immediately on touch

### Bluetooth HID Implementation
Use **ESP32-BLE-Keyboard** library:
```cpp
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("ESP32 Macro Pad", "YourName", 100);

void setup() {
    bleKeyboard.begin();
}

void sendMacro() {
    if(bleKeyboard.isConnected()) {
        bleKeyboard.press(KEY_LEFT_CTRL);
        bleKeyboard.press('c');
        delay(100);
        bleKeyboard.releaseAll();
    }
}
```

### Additional Features (Optional)
- **On-Screen Keyboard:** Configure macros without PC
- **Web Interface:** Configure via WiFi (web server on ESP32)
- **Layers:** Hold button for alternate layer (like Shift key)
- **RGB Backlighting:** If LEDs added, sync with button colors
- **Rotary Encoder:** Add physical encoder for volume or scrolling
- **OLED per Button:** If budget allows, mini OLEDs per key
- **Import/Export:** Save/load profiles via SD card or WiFi

### Example Profiles
1. **General Productivity:**
   - Copy, Paste, Undo, Save, Screenshot, Lock PC, Volume controls

2. **Video Editing (Premiere/DaVinci):**
   - Play/Pause, Cut, Ripple Delete, Add Marker, Export

3. **Photo Editing (Photoshop/Lightroom):**
   - Brush, Eraser, Zoom, Crop, Adjustment layers, Export

4. **Development (VS Code/IDE):**
   - Run, Debug, Terminal, Find, Format, Git commit

5. **Gaming/Streaming (OBS):**
   - Start/Stop recording, Switch scene, Mute mic, Toggle camera

## Getting Started
1. Copy platformio.ini from SnakePIO and adapt as needed
2. Add ESP32-BLE-Keyboard library
3. Set up LovyanGFX display configuration
4. Implement button grid rendering
5. Create touch-to-button mapping
6. Initialize BLE keyboard
7. Define macro structure and storage
8. Implement macro execution (key press sending)
9. Create profile switching system
10. Add configuration interface (web or on-device)
11. Test with paired PC/Mac/tablet

## Library Dependencies (add to platformio.ini)
```ini
lib_deps =
    lovyan03/LovyanGFX@1.2.7
    T-vK/ESP32-BLE-Keyboard@^0.3.2
```

## Notes
- BLE keyboard works with Windows, macOS, Linux, iOS, Android
- Some systems may require pairing each time (security feature)
- Media keys may not work on all platforms (test compatibility)
- 480x480 display is perfect for 4x4 button grid
- Consider adding a "Settings" button for profile management
- Store profiles in SPIFFS or Preferences (non-volatile storage)
- Consider adding haptic motor for physical feedback
- Test latency - BLE should have <50ms delay for responsive feel
- Some applications may not accept rapid key sequences - add delays if needed
- Be cautious with text macros containing passwords - security risk
