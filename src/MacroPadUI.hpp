#pragma once

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "Macros.hpp"

// ==============================================================================
// UI Constants
// ==============================================================================
#define SCREEN_WIDTH    480
#define SCREEN_HEIGHT   480

// Layout dimensions
#define HEADER_HEIGHT   40
#define FOOTER_HEIGHT   40
#define GRID_AREA_HEIGHT (SCREEN_HEIGHT - HEADER_HEIGHT - FOOTER_HEIGHT)

// Grid layout (responsive per-profile)
#define GRID_PADDING_X 10
#define GRID_PADDING_Y 10
#define BUTTON_SPACING_X 6
#define BUTTON_SPACING_Y 6
#define GRID_AVAILABLE_WIDTH  (SCREEN_WIDTH - (GRID_PADDING_X * 2))
#define GRID_AVAILABLE_HEIGHT (GRID_AREA_HEIGHT - (GRID_PADDING_Y * 2))

// Status bar
#define STATUS_BAR_Y    5
#define STATUS_BAR_HEIGHT 30
#define BT_STATUS_X     350
#define PROFILE_NAME_X  60

// Colors
#define COLOR_BG_HEADER     0x1082
#define COLOR_BG_FOOTER     0x1082
#define COLOR_BG_GRID       0x0000
#define COLOR_TEXT_HEADER   COLOR_WHITE
#define COLOR_TEXT_FOOTER   COLOR_WHITE
#define COLOR_BT_CONNECTED  0x07E0  // Green
#define COLOR_BT_DISCONNECTED 0xF800 // Red
#define COLOR_DIVIDER       0x4208

// Touch debounce
#define TOUCH_DEBOUNCE_MS   150
#define BUTTON_PRESS_DELAY  100

// Swipe detection
#define SWIPE_THRESHOLD     50
#define SWIPE_MIN_DISTANCE  80

// ==============================================================================
// Button State
// ==============================================================================
struct ButtonState {
    bool pressed;
    bool wasPressed;
    uint32_t pressStartTime;
    int16_t touchId;  // Track which touch point is pressing this button

    ButtonState() : pressed(false), wasPressed(false), pressStartTime(0), touchId(-1) {}
};

// ==============================================================================
// Forward declaration for callback
// ==============================================================================
class MacroPadUI;

// Callback function type for macro execution
typedef void (*MacroCallback)(const Macro& macro, int buttonIndex);

// Callback for profile change
typedef void (*ProfileChangeCallback)(int newProfileIndex);

// ==============================================================================
// MacroPad UI Class
// ==============================================================================
class MacroPadUI {
private:
    LGFX* _tft;
    Profile* _profiles;
    int _profileCount;
    int _currentProfileIndex;

    ButtonState _buttonStates[BUTTON_COUNT];

    // Touch handling
    int32_t _lastTouchX;
    int32_t _lastTouchY;
    uint32_t _lastTouchTime;
    bool _touchActive;
    int32_t _touchStartX;
    int32_t _touchStartY;

    // Callbacks
    MacroCallback _macroCallback;
    ProfileChangeCallback _profileChangeCallback;

    // Cached button coordinates
    int16_t _buttonX[BUTTON_COUNT];
    int16_t _buttonY[BUTTON_COUNT];

    // Needs full redraw flag
    bool _needsFullRedraw;

    // Bluetooth status cache
    bool _btConnected;

public:
    MacroPadUI(LGFX* tft, Profile* profiles, int profileCount)
        : _tft(tft), _profiles(profiles), _profileCount(profileCount),
          _currentProfileIndex(0), _lastTouchX(0), _lastTouchY(0),
          _lastTouchTime(0), _touchActive(false), _touchStartX(0), _touchStartY(0),
            _macroCallback(nullptr), _profileChangeCallback(nullptr), _needsFullRedraw(true),
            _btConnected(false)
    {
        updateButtonLayout();
    }

    void init() {
        _tft->setTextSize(1);
        _tft->setFont(&fonts::FreeSans9pt7b);
        updateButtonLayout();
        drawScreen();
    }

    void setMacroCallback(MacroCallback callback) {
        _macroCallback = callback;
    }

    void setProfileChangeCallback(ProfileChangeCallback callback) {
        _profileChangeCallback = callback;
    }

    void setBluetoothConnected(bool connected) {
        _btConnected = connected;
        drawBluetoothStatus(connected);
    }

    int getCurrentProfileIndex() const {
        return _currentProfileIndex;
    }

    const char* getCurrentProfileName() const {
        return _profiles[_currentProfileIndex].name;
    }

    void setProfile(int index) {
        if (index >= 0 && index < _profileCount && index != _currentProfileIndex) {
            _currentProfileIndex = index;
            _needsFullRedraw = true;
            updateButtonLayout();
            drawScreen();

            if (_profileChangeCallback) {
                _profileChangeCallback(_currentProfileIndex);
            }
        }
    }

    void nextProfile() {
        int next = (_currentProfileIndex + 1) % _profileCount;
        setProfile(next);
    }

    void prevProfile() {
        int prev = (_currentProfileIndex - 1 + _profileCount) % _profileCount;
        setProfile(prev);
    }

    void update() {
        // Handle touch input
        int32_t x, y;
        if (_tft->getTouch(&x, &y)) {
            handleTouch(x, y);
        } else {
            handleTouchRelease();
        }
    }

    void drawScreen() {
        drawHeader();
        drawGrid();
        drawFooter();
    }

    void drawHeader() {
        // Header background
        _tft->fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_BG_HEADER);

        // Profile name
        _tft->setTextColor(COLOR_TEXT_HEADER);
        _tft->setTextDatum(middle_left);
        _tft->setFont(&fonts::FreeSansBold9pt7b);
        _tft->drawString(_profiles[_currentProfileIndex].name, 10, HEADER_HEIGHT / 2);

        // Divider line
        _tft->drawFastHLine(0, HEADER_HEIGHT - 1, SCREEN_WIDTH, COLOR_DIVIDER);

        // Bluetooth status (persisted across redraws)
        drawBluetoothStatus(_btConnected);
    }

    void drawBluetoothStatus(bool connected) {
        _tft->setFont(&fonts::FreeSans9pt7b);
        _tft->setTextDatum(middle_right);

        // Clear the BT status area
        _tft->fillRect(BT_STATUS_X - 80, 0, 130, HEADER_HEIGHT - 1, COLOR_BG_HEADER);

        // Draw BT icon and text
        if (connected) {
            _tft->setTextColor(COLOR_BT_CONNECTED);
            _tft->drawString("BT: ", BT_STATUS_X + 60, HEADER_HEIGHT / 2);
            _tft->fillCircle(BT_STATUS_X + 75, HEADER_HEIGHT / 2, 5, COLOR_BT_CONNECTED);
        } else {
            _tft->setTextColor(COLOR_BT_DISCONNECTED);
            _tft->drawString("BT: ", BT_STATUS_X + 60, HEADER_HEIGHT / 2);
            _tft->drawCircle(BT_STATUS_X + 75, HEADER_HEIGHT / 2, 5, COLOR_BT_DISCONNECTED);
        }
    }

    void drawGrid() {
        // Clear grid area
        _tft->fillRect(0, HEADER_HEIGHT, SCREEN_WIDTH, GRID_AREA_HEIGHT, COLOR_BG_GRID);

        // Draw all buttons
        Profile& p = _profiles[_currentProfileIndex];
        for (int i = 0; i < activeButtonCount(); i++) {
            drawButton(i, p.buttons[i], false);
        }
    }

    void drawButton(int index, const Macro& macro, bool pressed) {
        if (macro.type == MACRO_TYPE_NONE && (!macro.label || strlen(macro.label) == 0)) {
            return;
        }
        int16_t x = _buttonX[index];
        int16_t y = _buttonY[index];
        int16_t bw = buttonWidth();
        int16_t bh = buttonHeight();

        // Button color
        uint16_t bgColor = pressed ? macro.pressColor : macro.color;

        // Draw button background with rounded corners effect (simulated with rectangle)
        _tft->fillRoundRect(x, y, bw, bh, 8, bgColor);

        // Draw border
        uint16_t borderColor = pressed ? COLOR_WHITE : COLOR_DARK_GRAY;
        _tft->drawRoundRect(x, y, bw, bh, 8, borderColor);

        // Draw label
        if (macro.label && strlen(macro.label) > 0) {
            _tft->setFont(&fonts::FreeSansBold9pt7b);
            _tft->setTextColor(BTN_COLOR_TEXT);
            _tft->setTextDatum(middle_center);

            // Main label
            _tft->drawString(macro.label, x + bw / 2, y + bh / 2 - 10);

            // Sublabel (shortcut)
            if (macro.sublabel && strlen(macro.sublabel) > 0) {
                _tft->setFont(&fonts::FreeSans9pt7b);
                _tft->setTextColor(BTN_COLOR_SUBTEXT);
                _tft->drawString(macro.sublabel, x + bw / 2, y + bh / 2 + 12);
            }
        }
    }

    void drawFooter() {
        int16_t footerY = SCREEN_HEIGHT - FOOTER_HEIGHT;

        // Footer background
        _tft->fillRect(0, footerY, SCREEN_WIDTH, FOOTER_HEIGHT, COLOR_BG_FOOTER);

        // Divider line
        _tft->drawFastHLine(0, footerY, SCREEN_WIDTH, COLOR_DIVIDER);

        // Navigation buttons
        _tft->setFont(&fonts::FreeSansBold9pt7b);
        _tft->setTextColor(COLOR_TEXT_FOOTER);
        _tft->setTextDatum(middle_center);

        // Left arrow (previous profile)
        _tft->fillRoundRect(20, footerY + 5, 100, 30, 5, 0x3186);
        _tft->drawString("< Prev", 70, footerY + 20);

        // Home indicator (shows current profile number)
        char profileNum[8];
        snprintf(profileNum, sizeof(profileNum), "%d/%d", _currentProfileIndex + 1, _profileCount);
        _tft->fillRoundRect(190, footerY + 5, 100, 30, 5, 0x4208);
        _tft->drawString(profileNum, 240, footerY + 20);

        // Right arrow (next profile)
        _tft->fillRoundRect(360, footerY + 5, 100, 30, 5, 0x3186);
        _tft->drawString("Next >", 410, footerY + 20);
    }

    void highlightButton(int index, bool pressed) {
        if (index >= 0 && index < activeButtonCount()) {
            Profile& p = _profiles[_currentProfileIndex];
            drawButton(index, p.buttons[index], pressed);
        }
    }

private:
    int gridRows() const {
        int rows = _profiles[_currentProfileIndex].gridRows;
        return rows > 0 ? rows : 1;
    }

    int gridCols() const {
        int cols = _profiles[_currentProfileIndex].gridCols;
        return cols > 0 ? cols : 1;
    }

    int activeButtonCount() const {
        return gridRows() * gridCols();
    }

    int buttonWidth() const {
        int cols = gridCols();
        return (GRID_AVAILABLE_WIDTH - ((cols - 1) * BUTTON_SPACING_X)) / cols;
    }

    int buttonHeight() const {
        int rows = gridRows();
        return (GRID_AVAILABLE_HEIGHT - ((rows - 1) * BUTTON_SPACING_Y)) / rows;
    }

    int gridTotalWidth() const {
        int cols = gridCols();
        return (cols * buttonWidth()) + ((cols - 1) * BUTTON_SPACING_X);
    }

    int gridTotalHeight() const {
        int rows = gridRows();
        return (rows * buttonHeight()) + ((rows - 1) * BUTTON_SPACING_Y);
    }

    int gridStartX() const {
        return (SCREEN_WIDTH - gridTotalWidth()) / 2;
    }

    int gridStartY() const {
        return HEADER_HEIGHT + ((GRID_AREA_HEIGHT - gridTotalHeight()) / 2);
    }

    void updateButtonLayout() {
        int rows = gridRows();
        int cols = gridCols();
        int bw = buttonWidth();
        int bh = buttonHeight();
        int startX = gridStartX();
        int startY = gridStartY();

        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                int idx = row * cols + col;
                _buttonX[idx] = startX + col * (bw + BUTTON_SPACING_X);
                _buttonY[idx] = startY + row * (bh + BUTTON_SPACING_Y);
            }
        }
    }

    void handleTouch(int32_t x, int32_t y) {
        uint32_t now = millis();

        if (!_touchActive) {
            // New touch started
            _touchStartX = x;
            _touchStartY = y;
            _touchActive = true;
        }

        _lastTouchX = x;
        _lastTouchY = y;
        _lastTouchTime = now;

        // Check if touching header (profile swipe area)
        if (y < HEADER_HEIGHT) {
            // Header touch - could use for profile switching gestures
            return;
        }

        // Check if touching footer (navigation)
        if (y >= SCREEN_HEIGHT - FOOTER_HEIGHT) {
            return;  // Handle footer in touch release
        }

        // Check which button is being touched
        int buttonIndex = getButtonAt(x, y);

        if (buttonIndex >= 0) {
            ButtonState& state = _buttonStates[buttonIndex];

            if (!state.pressed) {
                // Button just pressed
                state.pressed = true;
                state.pressStartTime = now;
                highlightButton(buttonIndex, true);

                // Execute macro
                Profile& p = _profiles[_currentProfileIndex];
                if (p.buttons[buttonIndex].type != MACRO_TYPE_NONE) {
                    if (_macroCallback) {
                        _macroCallback(p.buttons[buttonIndex], buttonIndex);
                    }
                }
            }
        }

            // Release buttons that are no longer being touched
            for (int i = 0; i < activeButtonCount(); i++) {
                if (_buttonStates[i].pressed && i != buttonIndex) {
                    _buttonStates[i].pressed = false;
                    highlightButton(i, false);
                }
            }
    }

    void handleTouchRelease() {
        if (!_touchActive) return;

        // Check for swipe gesture
        int16_t dx = _lastTouchX - _touchStartX;
        int16_t dy = _lastTouchY - _touchStartY;

        // Profile swipe detection (in header area)
        if (_touchStartY < HEADER_HEIGHT && abs(dx) > SWIPE_MIN_DISTANCE) {
            if (dx > 0) {
                prevProfile();
            } else {
                nextProfile();
            }
        }

        // Footer button detection
        if (_lastTouchY >= SCREEN_HEIGHT - FOOTER_HEIGHT) {
            int16_t footerY = SCREEN_HEIGHT - FOOTER_HEIGHT;
            // Prev button
            if (_lastTouchX >= 20 && _lastTouchX <= 120 &&
                _lastTouchY >= footerY + 5 && _lastTouchY <= footerY + 35) {
                prevProfile();
            }
            // Next button
            else if (_lastTouchX >= 360 && _lastTouchX <= 460 &&
                     _lastTouchY >= footerY + 5 && _lastTouchY <= footerY + 35) {
                nextProfile();
            }
        }

            // Release all buttons
            for (int i = 0; i < activeButtonCount(); i++) {
                if (_buttonStates[i].pressed) {
                    _buttonStates[i].pressed = false;
                    highlightButton(i, false);
                }
            }

        _touchActive = false;
    }

    int getButtonAt(int32_t x, int32_t y) {
        // Check if within grid area
        int16_t startX = gridStartX();
        int16_t startY = gridStartY();
        int16_t totalW = gridTotalWidth();
        int16_t totalH = gridTotalHeight();
        int16_t bw = buttonWidth();
        int16_t bh = buttonHeight();

        if (y < startY || y >= startY + totalH) {
            return -1;
        }

        if (x < startX || x >= startX + totalW) {
            return -1;
        }

        // Calculate row and column
        int cols = gridCols();
        int rows = gridRows();
        int col = (x - startX) / (bw + BUTTON_SPACING_X);
        int row = (y - startY) / (bh + BUTTON_SPACING_Y);

        // Check if actually within a button (not in spacing)
        int localX = (x - startX) % (bw + BUTTON_SPACING_X);
        int localY = (y - startY) % (bh + BUTTON_SPACING_Y);

        if (localX >= bw || localY >= bh) {
            return -1;  // In spacing area
        }

        // Validate bounds
        if (row >= 0 && row < rows && col >= 0 && col < cols) {
            int idx = row * cols + col;
            Profile& p = _profiles[_currentProfileIndex];
            if (p.buttons[idx].type == MACRO_TYPE_NONE &&
                (!p.buttons[idx].label || strlen(p.buttons[idx].label) == 0)) {
                return -1;
            }
            return idx;
        }

        return -1;
    }
};
