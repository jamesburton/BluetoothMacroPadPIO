# Bluetooth Macro Pad (ESP32-S3 + 4.0" Touch Display)

Touch-driven Bluetooth HID keyboard macro pad for the **ESP32-4848S040C_I** (Sunton 4.0" 480x480 IPS with GT911 touch). It renders a multi-profile grid UI and sends keyboard/media macros over BLE.

## Features
- Touch UI with profile header, button grid, and navigation footer
- Configurable macro types: single key, combo, sequence, text, media
- Multiple profiles with different grid sizes and color themes
- BLE HID keyboard output with connection status indicator
- Manual ST7701S init (3-wire SPI) + RGB panel via LovyanGFX
- GT911 touch input handling
- PSRAM enabled, watchdog tuned for BLE stability

## Hardware Target
- **Board:** ESP32-S3-DevKitC-1
- **Display:** 4.0" 480x480 IPS (ST7701S driver via 3-wire SPI + RGB)
- **Touch:** GT911 capacitive touch
- **PSRAM:** Enabled

Pin mapping and display timing are defined in `src/DisplayConfig.hpp`.

## Project Structure
```
.
├─ platformio.ini
├─ src/
│  ├─ main.cpp            # App entry, BLE, UI, macro execution
│  ├─ Macros.hpp           # Macro types, key codes, profiles
│  ├─ MacroPadUI.hpp       # Touch UI rendering and interaction
│  ├─ LGFX_Setup.hpp       # LovyanGFX panel/touch configuration
│  ├─ DisplayConfig.hpp    # Pinout and ST7701S init sequence
│  └─ BLEConfig.hpp        # Optional BLE stability utilities
└─ INSTRUCTIONS.md         # Project implementation notes
```

## Dependencies
Defined in `platformio.ini`:
- `lovyan03/LovyanGFX@1.2.7`
- `https://github.com/T-vK/ESP32-BLE-Keyboard.git`

## Getting Started
### 1) Install PlatformIO
Use VS Code + PlatformIO extension or CLI (`pio`).

### 2) Build
```
pio run
```

### 3) Upload
```
pio run -t upload
```

### 4) Serial Monitor
```
pio device monitor -b 115200
```

You should see startup logs including display init, UI setup, and BLE status.

## Pairing & Usage
1. Power the device. The BLE keyboard advertises as **MacroPad**.
2. Pair from your host OS (Windows/macOS/Linux/iOS/Android).
3. Tap a button on the screen to send its macro.
4. Swipe/tap the footer buttons to change profiles.

The header shows Bluetooth connection status with a colored indicator.

## Configuration
### Change Grid Size (Default 5x5)
In `src/Macros.hpp`, adjust:
```cpp
#define ACTIVE_GRID_ROWS GRID_SIZE_5
#define ACTIVE_GRID_COLS GRID_SIZE_5
```
Each profile can override `gridRows`/`gridCols` if needed.

### Edit Macros & Profiles
Profiles are defined in `src/Macros.hpp` (e.g., `createGeneralProfile()`, `createDevProfile()`).
Use the `Macro::singleKey`, `Macro::combo`, `Macro::sequence`, `Macro::textMacro`, and `Macro::media` helpers.

### Clear BLE Bonding (Optional)
In `src/main.cpp`, set:
```cpp
#define CLEAR_BONDING_ON_BOOT true
```
On next boot, all bonding data is cleared and you can re-pair.

### Display & Touch Tuning
- Display pins and ST7701S init sequence: `src/DisplayConfig.hpp`
- LovyanGFX panel/touch setup: `src/LGFX_Setup.hpp`

## Current Macro Types
- **Single Key:** one key press
- **Combo:** modifier(s) + key
- **Sequence:** multiple keys in order
- **Text:** types a string
- **Media:** consumer/media keys (play, next, volume, etc.)

## Troubleshooting
- **No display output:** confirm pinout and ST7701S init sequence in `src/DisplayConfig.hpp`.
- **Touch not responding:** verify GT911 I2C address and pins in `src/DisplayConfig.hpp`.
- **BLE pairing fails:** clear bonds and retry; ensure `MacroPad` is not already paired.
- **Random disconnects:** ensure stable power and keep the host within range.

## Development Notes
- `main.cpp` manually runs the ST7701S init sequence before `tft.init()`.
- BLE uses `ESP32-BLE-Keyboard` and a simple connection debounce.
- Watchdog is reconfigured for BLE stability and fed in the main loop.

## Roadmap Ideas
- On-device macro editor
- Web-based configuration
- Profile import/export (SPIFFS or SD)
- Haptic feedback and/or physical buttons

## License
No license restrictions are imposed; use and modify freely.
