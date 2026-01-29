#pragma once

#include <Arduino.h>

// ==============================================================================
// HID Key Codes (USB HID Usage Tables)
// ==============================================================================

// Modifier keys
#define MODIFIER_NONE   0x00
#define MODIFIER_CTRL   0x01
#define MODIFIER_SHIFT  0x02
#define MODIFIER_ALT    0x04
#define MODIFIER_GUI    0x08  // Windows key / Command key

// Special keys
#define KEY_NONE        0x00
#define KEY_A           0x04
#define KEY_B           0x05
#define KEY_C           0x06
#define KEY_D           0x07
#define KEY_E           0x08
#define KEY_F           0x09
#define KEY_G           0x0A
#define KEY_H           0x0B
#define KEY_I           0x0C
#define KEY_J           0x0D
#define KEY_K           0x0E
#define KEY_L           0x0F
#define KEY_M           0x10
#define KEY_N           0x11
#define KEY_O           0x12
#define KEY_P           0x13
#define KEY_Q           0x14
#define KEY_R           0x15
#define KEY_S           0x16
#define KEY_T           0x17
#define KEY_U           0x18
#define KEY_V           0x19
#define KEY_W           0x1A
#define KEY_X           0x1B
#define KEY_Y           0x1C
#define KEY_Z           0x1D
#define KEY_1           0x1E
#define KEY_2           0x1F
#define KEY_3           0x20
#define KEY_4           0x21
#define KEY_5           0x22
#define KEY_6           0x23
#define KEY_7           0x24
#define KEY_8           0x25
#define KEY_9           0x26
#define KEY_0           0x27
#define KEY_ENTER       0x28
#define KEY_ESC         0x29
#define KEY_BACKSPACE   0x2A
#define KEY_TAB         0x2B
#define KEY_SPACE       0x2C
#define KEY_MINUS       0x2D
#define KEY_EQUAL       0x2E
#define KEY_LEFT_BRACE  0x2F
#define KEY_RIGHT_BRACE 0x30
#define KEY_BACKSLASH   0x31
#define KEY_SEMICOLON   0x33
#define KEY_QUOTE       0x34
#define KEY_TILDE       0x35
#define KEY_COMMA       0x36
#define KEY_PERIOD      0x37
#define KEY_SLASH       0x38
#define KEY_CAPS_LOCK   0x39

// Function keys
#define KEY_F1          0x3A
#define KEY_F2          0x3B
#define KEY_F3          0x3C
#define KEY_F4          0x3D
#define KEY_F5          0x3E
#define KEY_F6          0x3F
#define KEY_F7          0x40
#define KEY_F8          0x41
#define KEY_F9          0x42
#define KEY_F10         0x43
#define KEY_F11         0x44
#define KEY_F12         0x45
#define KEY_F13         0x68
#define KEY_F14         0x69
#define KEY_F15         0x6A
#define KEY_F16         0x6B
#define KEY_F17         0x6C
#define KEY_F18         0x6D
#define KEY_F19         0x6E
#define KEY_F20         0x6F
#define KEY_F21         0x70
#define KEY_F22         0x71
#define KEY_F23         0x72
#define KEY_F24         0x73

// Navigation keys
#define KEY_INSERT      0x49
#define KEY_HOME        0x4A
#define KEY_PAGE_UP     0x4B
#define KEY_DELETE      0x4C
#define KEY_END         0x4D
#define KEY_PAGE_DOWN   0x4E
#define KEY_RIGHT       0x4F
#define KEY_LEFT        0x50
#define KEY_DOWN        0x51
#define KEY_UP          0x52

// Media keys (Consumer Page)
#define KEY_MEDIA_PLAY_PAUSE    0xE8
#define KEY_MEDIA_STOP          0xE9
#define KEY_MEDIA_PREV          0xEA
#define KEY_MEDIA_NEXT          0xEB
#define KEY_MEDIA_VOLUME_UP     0xEC
#define KEY_MEDIA_VOLUME_DOWN   0xED
#define KEY_MEDIA_MUTE          0xEE

// System keys
#define KEY_SYS_POWER           0x81
#define KEY_SYS_SLEEP           0x82
#define KEY_SYS_WAKE            0x83

// ==============================================================================
// Macro Types
// ==============================================================================
enum MacroType {
    MACRO_TYPE_NONE = 0,
    MACRO_TYPE_KEY = 1,         // Single key
    MACRO_TYPE_COMBO = 2,       // Modifier + key
    MACRO_TYPE_SEQUENCE = 3,    // Multiple keys in sequence
    MACRO_TYPE_TEXT = 4,        // Type text string
    MACRO_TYPE_MEDIA = 5        // Media key
};

// ==============================================================================
// Button Colors
// ==============================================================================
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_ORANGE    0xFC00
#define COLOR_PURPLE    0x8010
#define COLOR_GRAY      0x8410
#define COLOR_DARK_GRAY 0x4208
#define COLOR_DARK_BLUE 0x0010
#define COLOR_DARK_GREEN 0x0400

// Button color scheme
#define BTN_COLOR_DEFAULT   0x3186  // Dark blue-gray
#define BTN_COLOR_PRESSED   0x74D   // Light blue
#define BTN_COLOR_TEXT      COLOR_WHITE
#define BTN_COLOR_SUBTEXT   0xBDF7  // Light gray

// Profile accent colors
#define PROFILE_COLOR_GENERAL   0x3186  // Blue
#define PROFILE_COLOR_DEV       0x0400  // Green
#define PROFILE_COLOR_MIR4      0x07E0  // Green
#define PROFILE_COLOR_PHOTOSHOP 0xF9E7  // Orange
#define PROFILE_COLOR_GAMING    0xD8A7  // Purple
#define PROFILE_COLOR_MEDIA     0xF800  // Red

// ==============================================================================
// Macro Structure
// ==============================================================================
struct Macro {
    const char* label;          // Button label (e.g., "Copy", "Paste")
    const char* sublabel;       // Secondary label showing shortcut
    MacroType type;             // Type of macro
    uint8_t modifiers;          // Modifier keys (Ctrl, Alt, Shift, GUI)
    uint8_t keyCount;           // Number of keys in sequence
    uint8_t keys[6];            // Array of key codes (max 6 for sequence)
    const char* text;           // Text string for text macros
    uint16_t color;             // Button color
    uint16_t pressColor;        // Color when pressed

    // Default constructor
    Macro() : label(""), sublabel(""), type(MACRO_TYPE_NONE), modifiers(0),
              keyCount(0), text(nullptr), color(BTN_COLOR_DEFAULT),
              pressColor(BTN_COLOR_PRESSED) {
        for (int i = 0; i < 6; i++) keys[i] = 0;
    }

    // Single key constructor
    static Macro singleKey(const char* label, const char* sublabel, uint8_t key,
                           uint16_t color = BTN_COLOR_DEFAULT) {
        Macro m;
        m.label = label;
        m.sublabel = sublabel;
        m.type = MACRO_TYPE_KEY;
        m.modifiers = MODIFIER_NONE;
        m.keyCount = 1;
        m.keys[0] = key;
        m.color = color;
        m.pressColor = BTN_COLOR_PRESSED;
        return m;
    }

    // Combo constructor
    static Macro combo(const char* label, const char* sublabel, uint8_t modifiers,
                       uint8_t key, uint16_t color = BTN_COLOR_DEFAULT) {
        Macro m;
        m.label = label;
        m.sublabel = sublabel;
        m.type = MACRO_TYPE_COMBO;
        m.modifiers = modifiers;
        m.keyCount = 1;
        m.keys[0] = key;
        m.color = color;
        m.pressColor = BTN_COLOR_PRESSED;
        return m;
    }

    // Media key constructor
    static Macro media(const char* label, uint8_t mediaKey,
                       uint16_t color = BTN_COLOR_DEFAULT) {
        Macro m;
        m.label = label;
        m.sublabel = "";
        m.type = MACRO_TYPE_MEDIA;
        m.modifiers = MODIFIER_NONE;
        m.keyCount = 1;
        m.keys[0] = mediaKey;
        m.color = color;
        m.pressColor = BTN_COLOR_PRESSED;
        return m;
    }

    // Text macro constructor
    static Macro textMacro(const char* label, const char* text,
                           uint16_t color = BTN_COLOR_DEFAULT) {
        Macro m;
        m.label = label;
        m.sublabel = "Text";
        m.type = MACRO_TYPE_TEXT;
        m.modifiers = MODIFIER_NONE;
        m.keyCount = 0;
        m.text = text;
        m.color = color;
        m.pressColor = BTN_COLOR_PRESSED;
        return m;
    }

    // Sequence constructor
    static Macro sequence(const char* label, const char* sublabel, uint8_t modifiers,
                          const uint8_t* keySeq, uint8_t count,
                          uint16_t color = BTN_COLOR_DEFAULT) {
        Macro m;
        m.label = label;
        m.sublabel = sublabel;
        m.type = MACRO_TYPE_SEQUENCE;
        m.modifiers = modifiers;
        m.keyCount = min(count, (uint8_t)6);
        for (int i = 0; i < m.keyCount; i++) {
            m.keys[i] = keySeq[i];
        }
        m.color = color;
        m.pressColor = BTN_COLOR_PRESSED;
        return m;
    }
};

// ==============================================================================
// Profile Structure
// ==============================================================================
// Grid size options (rows/cols): 2..6, any permutation
#define GRID_SIZE_2 2
#define GRID_SIZE_3 3
#define GRID_SIZE_4 4
#define GRID_SIZE_5 5
#define GRID_SIZE_6 6

#ifndef ACTIVE_GRID_ROWS
#define ACTIVE_GRID_ROWS GRID_SIZE_5
#endif

#ifndef ACTIVE_GRID_COLS
#define ACTIVE_GRID_COLS GRID_SIZE_5
#endif

#define MAX_GRID_ROWS GRID_SIZE_6
#define MAX_GRID_COLS GRID_SIZE_6
#define GRID_ROWS ACTIVE_GRID_ROWS
#define GRID_COLS ACTIVE_GRID_COLS

// Storage sized for max grid; active count depends on ACTIVE_GRID_ROWS/COLS
#define BUTTON_COUNT (MAX_GRID_ROWS * MAX_GRID_COLS)
#define ACTIVE_BUTTON_COUNT (GRID_ROWS * GRID_COLS)

struct Profile {
    const char* name;           // Profile name
    uint16_t accentColor;       // Profile color theme
    uint8_t gridRows;           // Active grid rows for this profile
    uint8_t gridCols;           // Active grid cols for this profile
    Macro buttons[BUTTON_COUNT]; // Button grid

    // Default constructor
    Profile() : name("Default"), accentColor(PROFILE_COLOR_GENERAL),
                gridRows(ACTIVE_GRID_ROWS), gridCols(ACTIVE_GRID_COLS) {}

    // Constructor with name
    Profile(const char* profileName, uint16_t color)
        : name(profileName), accentColor(color),
          gridRows(ACTIVE_GRID_ROWS), gridCols(ACTIVE_GRID_COLS) {}
};

// Helpers for mapping 4-column profiles into the active grid size
inline int gridIndexFixed(int row, int col, int cols) {
    return row * cols + col;
}

inline int gridIndexFrom4(int idx) {
    return gridIndexFixed(idx / 4, idx % 4, 4);
}

#define BTN4(profile, index, macro) profile.buttons[gridIndexFrom4(index)] = macro

// ==============================================================================
// Default Profiles
// ==============================================================================

// General Productivity Profile
inline Profile createGeneralProfile() {
    Profile p("General", PROFILE_COLOR_GENERAL);
    p.gridRows = 4;
    p.gridCols = 4;

    // Row 1
    BTN4(p, 0, Macro::combo("Copy", "Ctrl+C", MODIFIER_CTRL, KEY_C));
    BTN4(p, 1, Macro::combo("Paste", "Ctrl+V", MODIFIER_CTRL, KEY_V));
    BTN4(p, 2, Macro::combo("Cut", "Ctrl+X", MODIFIER_CTRL, KEY_X));
    BTN4(p, 3, Macro::combo("Undo", "Ctrl+Z", MODIFIER_CTRL, KEY_Z));

    // Row 2
    BTN4(p, 4, Macro::combo("Save", "Ctrl+S", MODIFIER_CTRL, KEY_S));
    BTN4(p, 5, Macro::combo("Find", "Ctrl+F", MODIFIER_CTRL, KEY_F));
    BTN4(p, 6, Macro::combo("Select", "Ctrl+A", MODIFIER_CTRL, KEY_A));
    BTN4(p, 7, Macro::combo("Redo", "Ctrl+Y", MODIFIER_CTRL, KEY_Y));

    // Row 3 - Media
    BTN4(p, 8, Macro::media("Play/Pause", KEY_MEDIA_PLAY_PAUSE, COLOR_DARK_GREEN));
    BTN4(p, 9, Macro::media("Prev", KEY_MEDIA_PREV, COLOR_DARK_GREEN));
    BTN4(p, 10, Macro::media("Next", KEY_MEDIA_NEXT, COLOR_DARK_GREEN));
    BTN4(p, 11, Macro::media("Mute", KEY_MEDIA_MUTE, COLOR_RED));

    // Row 4 - System
    BTN4(p, 12, Macro::singleKey("Vol -", "Volume", KEY_MEDIA_VOLUME_DOWN, COLOR_BLUE));
    BTN4(p, 13, Macro::singleKey("Vol +", "Volume", KEY_MEDIA_VOLUME_UP, COLOR_BLUE));
    BTN4(p, 14, Macro::combo("Screenshot", "Win+Shift+S", MODIFIER_GUI | MODIFIER_SHIFT, KEY_S, COLOR_PURPLE));
    BTN4(p, 15, Macro::combo("Lock", "Win+L", MODIFIER_GUI, KEY_L, COLOR_GRAY));

    return p;
}

// Developer Profile (VS Code shortcuts)
inline Profile createDevProfile() {
    Profile p("VS Code", PROFILE_COLOR_DEV);
    p.gridRows = 4;
    p.gridCols = 4;

    // Row 1
    BTN4(p, 0, Macro::singleKey("Run", "F5", KEY_F5, 0x0400));
    BTN4(p, 1, Macro::singleKey("Debug", "F10", KEY_F10, 0x0500));
    BTN4(p, 2, Macro::combo("Terminal", "Ctrl+`", MODIFIER_CTRL, KEY_TILDE, 0x0600));
    BTN4(p, 3, Macro::combo("Find", "Ctrl+F", MODIFIER_CTRL, KEY_F));

    // Row 2
    BTN4(p, 4, Macro::combo("Save", "Ctrl+S", MODIFIER_CTRL, KEY_S));
    BTN4(p, 5, Macro::combo("Undo", "Ctrl+Z", MODIFIER_CTRL, KEY_Z));
    BTN4(p, 6, Macro::combo("Redo", "Ctrl+Y", MODIFIER_CTRL, KEY_Y));
    BTN4(p, 7, Macro::combo("Format", "Alt+Shift+F", MODIFIER_ALT | MODIFIER_SHIFT, KEY_F));

    // Row 3
    BTN4(p, 8, Macro::combo("Copy", "Ctrl+C", MODIFIER_CTRL, KEY_C));
    BTN4(p, 9, Macro::combo("Paste", "Ctrl+V", MODIFIER_CTRL, KEY_V));
    BTN4(p, 10, Macro::combo("Cut", "Ctrl+X", MODIFIER_CTRL, KEY_X));
    BTN4(p, 11, Macro::combo("Dup Line", "Ctrl+D", MODIFIER_CTRL, KEY_D));

    // Row 4
    BTN4(p, 12, Macro::combo("Split", "Ctrl+\\", MODIFIER_CTRL, KEY_BACKSLASH));
    BTN4(p, 13, Macro::combo("Close", "Ctrl+W", MODIFIER_CTRL, KEY_W));
    BTN4(p, 14, Macro::combo("Prev Tab", "Ctrl+PgUp", MODIFIER_CTRL, KEY_PAGE_UP));
    BTN4(p, 15, Macro::combo("Next Tab", "Ctrl+PgDn", MODIFIER_CTRL, KEY_PAGE_DOWN));

    return p;
}

// MIR4 Profile (default PC-style shortcuts - adjust in-game if needed)
inline Profile createMir4Profile() {
    Profile p("MIR4", PROFILE_COLOR_MIR4);
    p.gridRows = 5;
    p.gridCols = 4;

    // Row 1 - Function keys (as shown in overlay)
    BTN4(p, 0, Macro::singleKey("Character", "F1", KEY_F1, COLOR_BLUE));
    BTN4(p, 1, Macro::singleKey("Mail", "F2", KEY_F2, COLOR_BLUE));
    BTN4(p, 2, Macro::singleKey("Events", "F3", KEY_F3, COLOR_BLUE));
    BTN4(p, 3, Macro::singleKey("Shop", "F4", KEY_F4, COLOR_BLUE));

    // Row 2 - Function keys
    BTN4(p, 4, Macro::singleKey("Clan Info", "F5", KEY_F5, COLOR_PURPLE));
    BTN4(p, 5, Macro::singleKey("F6", "F6", KEY_F6, COLOR_PURPLE));
    BTN4(p, 6, Macro::singleKey("Equipment", "F7", KEY_F7, COLOR_PURPLE));
    BTN4(p, 7, Macro::singleKey("Notify", "F8", KEY_F8, COLOR_PURPLE));

    // Row 3 - Function keys + UI/mission
    BTN4(p, 8, Macro::singleKey("Game Menu", "F9", KEY_F9, COLOR_ORANGE));
    BTN4(p, 9, Macro::singleKey("Map", "F10", KEY_F10, COLOR_ORANGE));
    BTN4(p, 10, Macro::singleKey("Quest", "Q", KEY_Q, COLOR_DARK_GREEN));
    BTN4(p, 11, Macro::singleKey("Guild", "G", KEY_G, COLOR_CYAN));

    // Row 4 - Combat/utility
    BTN4(p, 12, Macro::singleKey("V", "V", KEY_V, COLOR_DARK_GREEN));
    BTN4(p, 13, Macro::singleKey("R", "R", KEY_R, COLOR_DARK_GREEN));
    BTN4(p, 14, Macro::singleKey("Target", "Tab", KEY_TAB, COLOR_DARK_GRAY));
    BTN4(p, 15, Macro::singleKey("Jump", "Space", KEY_SPACE, COLOR_GRAY));

    // Row 5 - Potions
    BTN4(p, 16, Macro::singleKey("Potion 1", "8", KEY_8, COLOR_RED));
    BTN4(p, 17, Macro::singleKey("Potion 2", "9", KEY_9, COLOR_RED));
    BTN4(p, 18, Macro::singleKey("Potion 3", "0", KEY_0, COLOR_RED));
    BTN4(p, 19, Macro::singleKey("Swap Pot", "-", KEY_MINUS, COLOR_ORANGE));

    return p;
}

// Photoshop Profile
inline Profile createPhotoshopProfile() {
    Profile p("Photoshop", PROFILE_COLOR_PHOTOSHOP);
    p.gridRows = 4;
    p.gridCols = 4;

    // Row 1 - Tools
    BTN4(p, 0, Macro::singleKey("Brush", "B", KEY_B, 0xF800));
    BTN4(p, 1, Macro::singleKey("Eraser", "E", KEY_E, 0xF800));
    BTN4(p, 2, Macro::singleKey("Clone", "S", KEY_S, 0xF800));
    BTN4(p, 3, Macro::singleKey("Heal", "J", KEY_J, 0xF800));

    // Row 2
    BTN4(p, 4, Macro::singleKey("Zoom", "Z", KEY_Z, 0xF9E7));
    BTN4(p, 5, Macro::singleKey("Crop", "C", KEY_C, 0xF9E7));
    BTN4(p, 6, Macro::singleKey("Lasso", "L", KEY_L, 0xF9E7));
    BTN4(p, 7, Macro::singleKey("Move", "V", KEY_V, 0xF9E7));

    // Row 3
    BTN4(p, 8, Macro::combo("Undo", "Ctrl+Z", MODIFIER_CTRL, KEY_Z));
    BTN4(p, 9, Macro::combo("Redo", "Ctrl+Shift+Z", MODIFIER_CTRL | MODIFIER_SHIFT, KEY_Z));
    BTN4(p, 10, Macro::combo("Free Trans", "Ctrl+T", MODIFIER_CTRL, KEY_T));
    BTN4(p, 11, Macro::combo("Deselect", "Ctrl+D", MODIFIER_CTRL, KEY_D));

    // Row 4
    BTN4(p, 12, Macro::combo("New Layer", "Ctrl+Shift+N", MODIFIER_CTRL | MODIFIER_SHIFT, KEY_N));
    BTN4(p, 13, Macro::combo("Merge", "Ctrl+E", MODIFIER_CTRL, KEY_E));
    BTN4(p, 14, Macro::singleKey("Fill", "G", KEY_G, 0xF800));
    BTN4(p, 15, Macro::combo("Export", "Ctrl+Shift+S", MODIFIER_CTRL | MODIFIER_SHIFT, KEY_S));

    return p;
}

// Media Control Profile
inline Profile createMediaProfile() {
    Profile p("Media", PROFILE_COLOR_MEDIA);
    p.gridRows = 2;
    p.gridCols = 4;

    // Row 1
    BTN4(p, 0, Macro::media("Play", KEY_MEDIA_PLAY_PAUSE, COLOR_GREEN));
    BTN4(p, 1, Macro::media("Stop", KEY_MEDIA_STOP, COLOR_RED));
    BTN4(p, 2, Macro::media("Vol Up", KEY_MEDIA_VOLUME_UP, COLOR_GREEN));
    BTN4(p, 3, Macro::media("Mute", KEY_MEDIA_MUTE, COLOR_RED));

    // Row 2
    BTN4(p, 4, Macro::media("Prev", KEY_MEDIA_PREV, COLOR_BLUE));
    BTN4(p, 5, Macro::media("Next", KEY_MEDIA_NEXT, COLOR_BLUE));
    BTN4(p, 6, Macro::media("Vol Down", KEY_MEDIA_VOLUME_DOWN, COLOR_GREEN));
    BTN4(p, 7, Macro::singleKey("", "", KEY_NONE));

    return p;
}

// Gaming/OBS Streaming Profile
inline Profile createGamingProfile() {
    Profile p("OBS/Gaming", PROFILE_COLOR_GAMING);
    p.gridRows = 4;
    p.gridCols = 4;

    // Row 1 - OBS Controls
    BTN4(p, 0, Macro::combo("Start Rec", "Ctrl+F9", MODIFIER_CTRL, KEY_F9, COLOR_RED));
    BTN4(p, 1, Macro::combo("Stop Rec", "Ctrl+F10", MODIFIER_CTRL, KEY_F10, COLOR_RED));
    BTN4(p, 2, Macro::combo("Pause Rec", "Ctrl+F11", MODIFIER_CTRL, KEY_F11, COLOR_ORANGE));
    BTN4(p, 3, Macro::combo("Screenshot", "F12", KEY_NONE, KEY_F12, COLOR_BLUE));

    // Row 2 - Scene/Sources
    BTN4(p, 4, Macro::singleKey("Scene 1", "F1", KEY_F1, COLOR_PURPLE));
    BTN4(p, 5, Macro::singleKey("Scene 2", "F2", KEY_F2, COLOR_PURPLE));
    BTN4(p, 6, Macro::singleKey("Scene 3", "F3", KEY_F3, COLOR_PURPLE));
    BTN4(p, 7, Macro::singleKey("Scene 4", "F4", KEY_F4, COLOR_PURPLE));

    // Row 3 - Audio
    BTN4(p, 8, Macro::combo("Mute Mic", "Ctrl+M", MODIFIER_CTRL, KEY_M, COLOR_CYAN));
    BTN4(p, 9, Macro::combo("Mute Desktop", "Ctrl+D", MODIFIER_CTRL, KEY_D, COLOR_CYAN));
    BTN4(p, 10, Macro::media("Vol Down", KEY_MEDIA_VOLUME_DOWN, COLOR_GREEN));
    BTN4(p, 11, Macro::media("Vol Up", KEY_MEDIA_VOLUME_UP, COLOR_GREEN));

    // Row 4 - Gaming utilities
    BTN4(p, 12, Macro::combo("Discord Mute", "Ctrl+Shift+M", MODIFIER_CTRL | MODIFIER_SHIFT, KEY_M, 0x7282));
    BTN4(p, 13, Macro::combo("Discord Deafen", "Ctrl+Shift+D", MODIFIER_CTRL | MODIFIER_SHIFT, KEY_D, 0x7282));
    BTN4(p, 14, Macro::combo("Push to Talk", "F13", KEY_NONE, KEY_F13, COLOR_RED));
    BTN4(p, 15, Macro::combo("Push to Mute", "F14", KEY_NONE, KEY_F14, COLOR_RED));

    return p;
}

// Number of profiles
#define PROFILE_COUNT 6

// Array of all profiles
inline Profile* getAllProfiles() {
    static Profile profiles[PROFILE_COUNT];
    profiles[0] = createGeneralProfile();
    profiles[1] = createDevProfile();
    profiles[2] = createMir4Profile();
    profiles[3] = createPhotoshopProfile();
    profiles[4] = createMediaProfile();
    profiles[5] = createGamingProfile();
    return profiles;
}
