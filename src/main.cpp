#include <Arduino.h>
#include <Wire.h>
#include <BleKeyboard.h>
#include <esp_task_wdt.h>
#include "DisplayConfig.hpp"
#include "LGFX_Setup.hpp"
#include "Macros.hpp"
#include "MacroPadUI.hpp"
#include "BLEConfig.hpp"

// ==============================================================================
// Configuration
// ==============================================================================

// Set to true to clear bonding data on next boot
#define CLEAR_BONDING_ON_BOOT false

// Connection debounce to prevent rapid connect/disconnect spam
#define CONNECTION_DEBOUNCE_MS 1000

// ==============================================================================
// Global Instances
// ==============================================================================
LGFX tft;
BleKeyboard bleKeyboard("MacroPad", "ESP32-S3", 100);

// Media key reports (avoid name collisions with HID macros)
static const MediaKeyReport MEDIA_PLAY_PAUSE = {8, 0};
static const MediaKeyReport MEDIA_STOP = {4, 0};
static const MediaKeyReport MEDIA_PREV = {2, 0};
static const MediaKeyReport MEDIA_NEXT = {1, 0};
static const MediaKeyReport MEDIA_VOL_UP = {32, 0};
static const MediaKeyReport MEDIA_VOL_DOWN = {64, 0};
static const MediaKeyReport MEDIA_MUTE = {16, 0};

Profile* profiles = nullptr;
MacroPadUI* ui = nullptr;

// Connection tracking
bool bleConnected = false;
uint32_t lastStatusUpdate = 0;
uint32_t lastConnectionChange = 0;
uint32_t bleConnectedSince = 0;
uint32_t bleDisconnectedSince = 0;
int bleConnectCount = 0;

// Simple BLE status (avoid custom BLE stack callbacks)
void printBLEStatusSimple() {
    Serial.println("\n--- BLE Status ---");
    Serial.printf("Connected: %s\n", bleKeyboard.isConnected() ? "YES" : "NO");
    Serial.printf("Connect count: %d\n", bleConnectCount);
    if (bleKeyboard.isConnected() && bleConnectedSince > 0) {
        Serial.printf("Connected for: %lu ms\n", millis() - bleConnectedSince);
    } else if (bleDisconnectedSince > 0) {
        Serial.printf("Last disconnect: %lu ms ago\n",
            millis() - bleDisconnectedSince);
    }
    Serial.println("------------------\n");
}

// ==============================================================================
// Watchdog Timer Management
// ==============================================================================
void initWatchdog() {
    // Disable watchdog for CPU0 (Bluetooth uses this)
    // The default task watchdog may trigger during BLE operations
    esp_err_t ret = esp_task_wdt_deinit();
    if (ret == ESP_OK) {
        Serial.println("WDT: Deinitialized default watchdog");
    }

    // Reinitialize with longer timeout for stability
    // Use the simpler API: esp_task_wdt_init(timeout_seconds, panic)
    ret = esp_task_wdt_init(30, false);  // 30 seconds, don't panic
    if (ret == ESP_OK) {
        Serial.println("WDT: Initialized with 30s timeout");
    } else {
        Serial.printf("WDT: Init failed: %d\n", ret);
    }

    // Subscribe the idle task of CPU1 (where loop() runs)
    ret = esp_task_wdt_add(NULL);
    if (ret == ESP_OK) {
        Serial.println("WDT: Loop task subscribed");
    }
}

void feedWatchdog() {
    esp_task_wdt_reset();
}

// ==============================================================================
// ST7701S Manual Initialization (3-Wire SPI)
// ==============================================================================
void st7701_send(uint8_t data, bool is_cmd) {
    digitalWrite(PIN_SPI_CS, LOW);

    // 9-bit SPI: 1st bit is D/C (0=Cmd, 1=Data)
    digitalWrite(PIN_SPI_SCK, LOW);
    digitalWrite(PIN_SPI_SDA, is_cmd ? LOW : HIGH);
    delayMicroseconds(1);
    digitalWrite(PIN_SPI_SCK, HIGH);
    delayMicroseconds(1);

    for (int i = 0; i < 8; i++) {
        digitalWrite(PIN_SPI_SCK, LOW);
        if (data & 0x80) digitalWrite(PIN_SPI_SDA, HIGH);
        else             digitalWrite(PIN_SPI_SDA, LOW);
        delayMicroseconds(1);
        digitalWrite(PIN_SPI_SCK, HIGH);
        delayMicroseconds(1);
        data <<= 1;
    }
    digitalWrite(PIN_SPI_CS, HIGH);
    delayMicroseconds(1);
}

void st7701_write_command(uint8_t c) {
    st7701_send(c, true);
}

void st7701_write_data(uint8_t d) {
    st7701_send(d, false);
}

void run_init_sequence() {
    Serial.println("ST7701: Starting Manual Init...");
    pinMode(PIN_SPI_CS, OUTPUT);
    pinMode(PIN_SPI_SDA, OUTPUT);
    pinMode(PIN_SPI_SCK, OUTPUT);

    digitalWrite(PIN_SPI_CS, HIGH);
    digitalWrite(PIN_SPI_SCK, HIGH);
    digitalWrite(PIN_SPI_SDA, HIGH);

    delay(120);

    const size_t n = sizeof(ST7701_INIT_SEQUENCE);
    size_t i = 0;
    while (i < n) {
        const uint8_t cmd = ST7701_INIT_SEQUENCE[i++];
        if (cmd == 0x00 && i >= n) break; // EOF

        if (cmd == 0xFF) {
            st7701_write_command(cmd);
            if (i >= n) break;
            uint8_t len = ST7701_INIT_SEQUENCE[i++];
            for (uint8_t j = 0; j < len; j++) {
                if (i >= n) break;
                st7701_write_data(ST7701_INIT_SEQUENCE[i++]);
            }
            continue;
        }

        if (cmd == 0x00) {
             break;
        }

        st7701_write_command(cmd);
        if (i >= n) break;
        uint8_t len = ST7701_INIT_SEQUENCE[i++];

        for (uint8_t j = 0; j < len; j++) {
            if (i >= n) break;
            st7701_write_data(ST7701_INIT_SEQUENCE[i++]);
        }

        if (cmd == 0x11) delay(120);
        if (cmd == 0x29) delay(50);
        if (cmd == 0xFF) delay(10);
    }
    Serial.println("ST7701: Manual Init Done.");
}

// ==============================================================================
// HID Key Conversion
// ==============================================================================
uint8_t hidToBleKey(uint8_t hidKey) {
    // Non-printing HID keys (F-keys, arrows, etc.) need +0x88 offset for BleKeyboard
    if ((hidKey >= KEY_F1 && hidKey <= KEY_F24) ||
        (hidKey >= KEY_INSERT && hidKey <= KEY_PAGE_DOWN) ||
        (hidKey >= KEY_RIGHT && hidKey <= KEY_UP) ||
        hidKey == KEY_ENTER || hidKey == KEY_ESC || hidKey == KEY_BACKSPACE || hidKey == KEY_TAB) {
        return hidKey + 0x88;
    }

    if (hidKey >= KEY_A && hidKey <= KEY_Z) {
        return 'a' + (hidKey - KEY_A);
    }
    if (hidKey >= KEY_1 && hidKey <= KEY_9) {
        return '1' + (hidKey - KEY_1);
    }
    if (hidKey == KEY_0) {
        return '0';
    }

    switch (hidKey) {
        case KEY_ENTER: return hidKey + 0x88;
        case KEY_ESC: return hidKey + 0x88;
        case KEY_BACKSPACE: return hidKey + 0x88;
        case KEY_TAB: return hidKey + 0x88;
        case KEY_SPACE: return ' ';
        case KEY_MINUS: return '-';
        case KEY_EQUAL: return '=';
        case KEY_LEFT_BRACE: return '[';
        case KEY_RIGHT_BRACE: return ']';
        case KEY_BACKSLASH: return '\\';
        case KEY_SEMICOLON: return ';';
        case KEY_QUOTE: return '\'';
        case KEY_TILDE: return '`';
        case KEY_COMMA: return ',';
        case KEY_PERIOD: return '.';
        case KEY_SLASH: return '/';

        case KEY_F1: return hidKey + 0x88;
        case KEY_F2: return hidKey + 0x88;
        case KEY_F3: return hidKey + 0x88;
        case KEY_F4: return hidKey + 0x88;
        case KEY_F5: return hidKey + 0x88;
        case KEY_F6: return hidKey + 0x88;
        case KEY_F7: return hidKey + 0x88;
        case KEY_F8: return hidKey + 0x88;
        case KEY_F9: return hidKey + 0x88;
        case KEY_F10: return hidKey + 0x88;
        case KEY_F11: return hidKey + 0x88;
        case KEY_F12: return hidKey + 0x88;
        case KEY_F13: return hidKey + 0x88;
        case KEY_F14: return hidKey + 0x88;
        case KEY_F15: return hidKey + 0x88;
        case KEY_F16: return hidKey + 0x88;
        case KEY_F17: return hidKey + 0x88;
        case KEY_F18: return hidKey + 0x88;
        case KEY_F19: return hidKey + 0x88;
        case KEY_F20: return hidKey + 0x88;
        case KEY_F21: return hidKey + 0x88;
        case KEY_F22: return hidKey + 0x88;
        case KEY_F23: return hidKey + 0x88;
        case KEY_F24: return hidKey + 0x88;

        case KEY_INSERT: return hidKey + 0x88;
        case KEY_HOME: return hidKey + 0x88;
        case KEY_PAGE_UP: return hidKey + 0x88;
        case KEY_DELETE: return hidKey + 0x88;
        case KEY_END: return hidKey + 0x88;
        case KEY_PAGE_DOWN: return hidKey + 0x88;
        case KEY_RIGHT: return hidKey + 0x88;
        case KEY_LEFT: return hidKey + 0x88;
        case KEY_DOWN: return hidKey + 0x88;
        case KEY_UP: return hidKey + 0x88;

        default: return 0;
    }
}

// ==============================================================================
// Macro Execution
// ==============================================================================
void executeMacro(const Macro& macro, int buttonIndex) {
    if (!bleKeyboard.isConnected()) {
        Serial.println("BLE not connected, cannot send macro");
        return;
    }

    Serial.printf("Executing macro: %s (type=%d)\n", macro.label, macro.type);

    switch (macro.type) {
        case MACRO_TYPE_KEY:
            if (macro.keyCount > 0 && macro.keys[0] != KEY_NONE) {
                uint8_t key = hidToBleKey(macro.keys[0]);
                if (key != 0) {
                    bleKeyboard.write(key);
                    Serial.printf("Sent key: 0x%02X\n", key);
                }
            }
            break;

        case MACRO_TYPE_COMBO:
            {
                uint8_t key = hidToBleKey(macro.keys[0]);
                if (key != 0) {
                    if (macro.modifiers & MODIFIER_CTRL) {
                        bleKeyboard.press(KEY_LEFT_CTRL);
                    }
                    if (macro.modifiers & MODIFIER_SHIFT) {
                        bleKeyboard.press(KEY_LEFT_SHIFT);
                    }
                    if (macro.modifiers & MODIFIER_ALT) {
                        bleKeyboard.press(KEY_LEFT_ALT);
                    }
                    if (macro.modifiers & MODIFIER_GUI) {
                        bleKeyboard.press(KEY_LEFT_GUI);
                    }

                    bleKeyboard.press(key);
                    delay(50);
                    bleKeyboard.releaseAll();

                    Serial.printf("Sent combo: modifiers=0x%02X key=0x%02X\n",
                                  macro.modifiers, key);
                }
            }
            break;

        case MACRO_TYPE_SEQUENCE:
            {
                for (int i = 0; i < macro.keyCount; i++) {
                    uint8_t key = hidToBleKey(macro.keys[i]);
                    if (key != 0) {
                        bleKeyboard.write(key);
                        delay(30);
                    }
                }
                Serial.printf("Sent sequence of %d keys\n", macro.keyCount);
            }
            break;

        case MACRO_TYPE_TEXT:
            if (macro.text != nullptr) {
                bleKeyboard.print(macro.text);
                Serial.printf("Sent text: %s\n", macro.text);
            }
            break;

        case MACRO_TYPE_MEDIA:
            if (macro.keyCount > 0) {
                uint8_t mediaKey = macro.keys[0];
                switch (mediaKey) {
                    case KEY_MEDIA_PLAY_PAUSE:
                        bleKeyboard.write(MEDIA_PLAY_PAUSE);
                        break;
                    case KEY_MEDIA_STOP:
                        bleKeyboard.write(MEDIA_STOP);
                        break;
                    case KEY_MEDIA_PREV:
                        bleKeyboard.write(MEDIA_PREV);
                        break;
                    case KEY_MEDIA_NEXT:
                        bleKeyboard.write(MEDIA_NEXT);
                        break;
                    case KEY_MEDIA_VOLUME_UP:
                        bleKeyboard.write(MEDIA_VOL_UP);
                        break;
                    case KEY_MEDIA_VOLUME_DOWN:
                        bleKeyboard.write(MEDIA_VOL_DOWN);
                        break;
                    case KEY_MEDIA_MUTE:
                        bleKeyboard.write(MEDIA_MUTE);
                        break;
                }
                Serial.printf("Sent media key: 0x%02X\n", mediaKey);
            }
            break;

        default:
            Serial.println("Unknown macro type");
            break;
    }
}

// ==============================================================================
// Profile Change Handler
// ==============================================================================
void onProfileChanged(int newProfileIndex) {
    Serial.printf("Switched to profile: %s\n", profiles[newProfileIndex].name);
}

// ==============================================================================
// Setup and Loop
// ==============================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n================================");
    Serial.println("Bluetooth Macro Pad Starting...");
    Serial.println("================================");
    Serial.printf("PSRAM Size: %d bytes\n", ESP.getPsramSize());

    // 1. Initialize watchdog
    initWatchdog();

    // 2. Run Manual Init for ST7701S
    Serial.println("Initializing display...");
    run_init_sequence();

    // 3. Initialize LGFX
    Serial.println("Starting TFT...");
    tft.init();
    tft.setBrightness(255);

    // 4. Initialize profiles
    Serial.println("Loading profiles...");
    profiles = getAllProfiles();

    // 5. Create UI
    Serial.println("Creating UI...");
    ui = new MacroPadUI(&tft, profiles, PROFILE_COUNT);
    ui->setMacroCallback(executeMacro);
    ui->setProfileChangeCallback(onProfileChanged);
    ui->init();

    // 6. Start BLE Keyboard
    Serial.println("Starting BLE Keyboard...");
    bleKeyboard.begin();
    Serial.println("BLE Keyboard started");

    // Optionally clear bonds AFTER BLE stack is up
    #if CLEAR_BONDING_ON_BOOT
    delay(200);
    clearBLEBondingData();
    #endif

    Serial.println("\n================================");
    Serial.println("Setup complete!");
    Serial.println("Waiting for BLE connection...");
    Serial.println("================================\n");

    printBLEStatusSimple();
}

void loop() {
    // Feed watchdog
    feedWatchdog();

    // Update UI (handles touch input)
    ui->update();

    uint32_t now = millis();

    // Check BLE connection status with debounce
    bool currentlyConnected = bleKeyboard.isConnected();
    if (currentlyConnected != bleConnected) {
        if (now - lastConnectionChange > CONNECTION_DEBOUNCE_MS) {
            lastConnectionChange = now;
            bleConnected = currentlyConnected;
            ui->setBluetoothConnected(bleConnected);

            if (bleConnected) {
                bleConnectedSince = now;
                bleConnectCount++;
                Serial.println("\n*** BLE CONNECTED ***");
                printBLEStatusSimple();
            } else {
                bleDisconnectedSince = now;
                Serial.println("\n*** BLE DISCONNECTED ***");
                printBLEStatusSimple();
            }
        }
    }

    // Periodic status update (every 10 seconds)
    if (now - lastStatusUpdate > 10000) {
        lastStatusUpdate = now;

        if (bleConnected && bleConnectedSince > 0) {
            Serial.printf("BLE: Stable connection, uptime: %lu ms\n",
                now - bleConnectedSince);
        } else {
            Serial.println("BLE: Waiting for connection...");
        }

        // Also print memory status periodically
        Serial.printf("Heap: %d free, PSRAM: %d free\n",
            ESP.getFreeHeap(), ESP.getFreePsram());
    }

    delay(5);
}
