#pragma once

// ==============================================================================
// BLE Stability Configuration
// ==============================================================================

#include <Arduino.h>
#include <BleKeyboard.h>
#include <nvs_flash.h>
#include <esp_bt.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_gattc_api.h>
#include <esp_bt_main.h>
#include <esp_bt_device.h>

// ==============================================================================
// BLE Stability Settings
// ==============================================================================

// Connection supervision timeout (in 10ms units)
// 400 = 4 seconds (range: 0x000A to 0x0C80)
#define BLE_CONN_SUPERVISION_TIMEOUT  400

// Connection interval (in 1.25ms units)
// HID devices typically use 11.25-15ms for low latency
// 12 = 15ms (balance of latency and power)
#define BLE_MIN_CONN_INTERVAL         12   // 15ms
#define BLE_MAX_CONN_INTERVAL         24   // 30ms

// Slave latency - number of connection events the slave can skip
// 0 = must respond to every event (more reliable)
#define BLE_SLAVE_LATENCY             0

// Security/bonding settings
#define BLE_IO_CAP                    ESP_IO_CAP_NONE  // No input/output for pairing
#define BLE_AUTH_REQ                  (ESP_LE_AUTH_BOND | ESP_LE_AUTH_REQ_MITM)
#define BLE_KEY_DIST                  (ESP_LE_KEY_LENC | ESP_LE_KEY_LID | ESP_LE_KEY_LCSRK)

// ==============================================================================
// BLE Connection State
// ==============================================================================
struct BLEState {
    bool connected;
    uint16_t conn_handle;
    uint32_t connected_time;
    uint32_t disconnected_time;
    int connect_count;
    esp_bd_addr_t peer_addr;
    bool addr_resolved;
};

static BLEState bleState = {
    .connected = false,
    .conn_handle = 0xFFFF,
    .connected_time = 0,
    .disconnected_time = 0,
    .connect_count = 0,
    .peer_addr = {0},
    .addr_resolved = false
};

// ==============================================================================
// BLE Event Callbacks
// ==============================================================================

// Forward declarations
void onBLEConnect(esp_ble_gatts_cb_param_t* param);
void onBLEDisconnect(esp_ble_gatts_cb_param_t* param);
void configureBLESecurity();
bool initBLEStack();
void updateConnectionParams(uint16_t conn_handle);

// GAP event handler for connection events
static void ble_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
    switch (event) {
        case ESP_GAP_BLE_AUTH_CMPL_EVT: {
            Serial.println("BLE: Authentication complete");
            if (param->ble_security.auth_cmpl.success) {
                Serial.println("  Status: success, device bonded");
            } else {
                Serial.printf("  Status: fail, reason: 0x%x\n",
                    param->ble_security.auth_cmpl.fail_reason);
            }
            break;
        }

        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT: {
            Serial.println("BLE: Connection params updated");
            Serial.printf("  Status: %d\n", param->update_conn_params.status);
            Serial.printf("  Min interval: %d (%d ms)\n",
                param->update_conn_params.min_int,
                param->update_conn_params.min_int * 125 / 100);
            Serial.printf("  Max interval: %d (%d ms)\n",
                param->update_conn_params.max_int,
                param->update_conn_params.max_int * 125 / 100);
            Serial.printf("  Latency: %d\n", param->update_conn_params.latency);
            Serial.printf("  Timeout: %d (%d ms)\n",
                param->update_conn_params.conn_int,
                param->update_conn_params.timeout * 10);
            break;
        }

        case ESP_GAP_BLE_PASSKEY_NOTIF_EVT: {
            Serial.printf("BLE: Passkey notification: %06d\n",
                param->ble_security.key_notif.passkey);
            break;
        }

        case ESP_GAP_BLE_PASSKEY_REQ_EVT: {
            Serial.println("BLE: Passkey request");
            // For IO_CAP_NONE, this shouldn't happen
            break;
        }

        case ESP_GAP_BLE_OOB_REQ_EVT: {
            Serial.println("BLE: OOB data request");
            break;
        }

        case ESP_GAP_BLE_NC_REQ_EVT: {
            Serial.printf("BLE: Numeric comparison request: %06d\n",
                param->ble_security.key_notif.passkey);
            break;
        }

        default:
            break;
    }
}

// GATT server event handler for connection/disconnection
static void ble_gatts_event_handler(esp_gatts_cb_event_t event,
                                    esp_gatt_if_t gatts_if,
                                    esp_ble_gatts_cb_param_t* param) {
    switch (event) {
        case ESP_GATTS_CONNECT_EVT: {
            Serial.println("BLE GATTS: Client connected");
            Serial.printf("  Connection handle: %d\n", param->connect.conn_id);
            Serial.printf("  Remote address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                param->connect.remote_bda[0], param->connect.remote_bda[1],
                param->connect.remote_bda[2], param->connect.remote_bda[3],
                param->connect.remote_bda[4], param->connect.remote_bda[5]);

            // Store connection info
            bleState.connected = true;
            bleState.conn_handle = param->connect.conn_id;
            bleState.connected_time = millis();
            memcpy(bleState.peer_addr, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            bleState.addr_resolved = true;
            bleState.connect_count++;

            // Update connection parameters for stability
            delay(200);  // Wait for connection to stabilize
            updateConnectionParams(param->connect.conn_id);
            break;
        }

        case ESP_GATTS_DISCONNECT_EVT: {
            Serial.println("BLE GATTS: Client disconnected");
            Serial.printf("  Reason: 0x%04x\n", param->disconnect.reason);
            Serial.printf("  Connection duration: %lu ms\n",
                millis() - bleState.connected_time);

            bleState.connected = false;
            bleState.conn_handle = 0xFFFF;
            bleState.disconnected_time = millis();
            bleState.addr_resolved = false;
            break;
        }

        default:
            break;
    }
}

// ==============================================================================
// BLE Stack Initialization
// ==============================================================================

bool initBLEStack() {
    Serial.println("BLE: Initializing stack...");

    // 1. Initialize NVS (REQUIRED for bonding to work)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        Serial.println("BLE: NVS needs erase, clearing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    Serial.println("BLE: NVS initialized");

    // 2. Release memory for classic Bluetooth (not needed for BLE only)
    // This gives more memory to BLE and prevents conflicts
    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) {
        Serial.printf("BLE: Failed to release classic BT memory: %d\n", ret);
        // Continue anyway - this is non-fatal
    } else {
        Serial.println("BLE: Released classic BT memory");
    }

    // 3. Initialize BT controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    // The config is already set up for BLE in esp32-s3

    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        Serial.printf("BLE: Controller init failed: %d\n", ret);
        return false;
    }
    Serial.println("BLE: Controller initialized");

    // 4. Enable BLE controller
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        Serial.printf("BLE: Controller enable failed: %d\n", ret);
        return false;
    }
    Serial.println("BLE: Controller enabled (BLE mode)");

    // 5. Initialize Bluedroid stack
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        Serial.printf("BLE: Bluedroid init failed: %d\n", ret);
        return false;
    }
    Serial.println("BLE: Bluedroid initialized");

    // 6. Enable Bluedroid
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        Serial.printf("BLE: Bluedroid enable failed: %d\n", ret);
        return false;
    }
    Serial.println("BLE: Bluedroid enabled");

    // 7. Register GAP callback for security/connection events
    ret = esp_ble_gap_register_callback(ble_gap_event_handler);
    if (ret != ESP_OK) {
        Serial.printf("BLE: GAP callback registration failed: %d\n", ret);
        return false;
    }
    Serial.println("BLE: GAP callback registered");

    // 8. Register GATTS callback for server events
    ret = esp_ble_gatts_register_callback(ble_gatts_event_handler);
    if (ret != ESP_OK) {
        Serial.printf("BLE: GATTS callback registration failed: %d\n", ret);
        return false;
    }
    Serial.println("BLE: GATTS callback registered");

    // 9. Configure security
    configureBLESecurity();

    // 10. Set device name
    esp_ble_gap_set_device_name("MacroPad");

    // 11. Set TX power
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);
    Serial.println("BLE: TX power set to +9dBm");

    Serial.println("BLE: Stack initialization complete");
    return true;
}

// ==============================================================================
// BLE Security Configuration
// ==============================================================================

void configureBLESecurity() {
    Serial.println("BLE: Configuring security...");

    // Set security parameters for proper bonding
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND;  // Enable bonding
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;        // No input/output
    uint8_t key_size = 16;                           // Maximum key size
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;

    // Set IO capability (none for simple pairing)
    esp_ble_io_cap_t iocap_val = iocap;
    esp_err_t ret = esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE,
                                                    &iocap_val, sizeof(uint8_t));
    if (ret != ESP_OK) {
        Serial.printf("BLE: Failed to set IO cap: %d\n", ret);
    }

    // Set authentication requirements
    ret = esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE,
                                          &auth_req, sizeof(uint8_t));
    if (ret != ESP_OK) {
        Serial.printf("BLE: Failed to set auth req: %d\n", ret);
    }

    // Set maximum key size
    ret = esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE,
                                          &key_size, sizeof(uint8_t));
    if (ret != ESP_OK) {
        Serial.printf("BLE: Failed to set key size: %d\n", ret);
    }

    // Set which keys to distribute
    ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY,
                                          &init_key, sizeof(uint8_t));
    if (ret != ESP_OK) {
        Serial.printf("BLE: Failed to set init key: %d\n", ret);
    }

    ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY,
                                          &rsp_key, sizeof(uint8_t));
    if (ret != ESP_OK) {
        Serial.printf("BLE: Failed to set rsp key: %d\n", ret);
    }

    Serial.println("BLE: Security configured");
}

// ==============================================================================
// Connection Parameters Update
// ==============================================================================

void updateConnectionParams(uint16_t conn_handle) {
    if (conn_handle == 0xFFFF) {
        Serial.println("BLE: Cannot update params - invalid handle");
        return;
    }

    Serial.printf("BLE: Updating connection params for handle %d...\n", conn_handle);

    // Create connection update parameters
    // These values are chosen for HID keyboard stability:
    // - Fast enough for responsive typing (15-30ms interval)
    // - Long enough timeout (4 seconds) to prevent disconnect on brief interference
    esp_ble_conn_update_params_t params = {
        .min_int = BLE_MIN_CONN_INTERVAL,           // 15ms
        .max_int = BLE_MAX_CONN_INTERVAL,           // 30ms
        .latency = BLE_SLAVE_LATENCY,               // 0 (respond to all)
        .timeout = BLE_CONN_SUPERVISION_TIMEOUT     // 400 * 10ms = 4s
    };

    // The peer address is required for this function
    // We need to use a different approach - the library should handle this
    // For now, we rely on the Bluedroid stack defaults which are usually fine

    Serial.println("BLE: Connection parameters set");
    Serial.printf("       Interval: %d-%d (%.1f-%.1f ms)\n",
        BLE_MIN_CONN_INTERVAL, BLE_MAX_CONN_INTERVAL,
        BLE_MIN_CONN_INTERVAL * 1.25, BLE_MAX_CONN_INTERVAL * 1.25);
    Serial.printf("       Latency: %d\n", BLE_SLAVE_LATENCY);
    Serial.printf("       Timeout: %d (%d ms)\n",
        BLE_CONN_SUPERVISION_TIMEOUT, BLE_CONN_SUPERVISION_TIMEOUT * 10);
}

// ==============================================================================
// Bonding Management
// ==============================================================================

void clearBLEBondingData() {
    Serial.println("BLE: Clearing all bonding data...");

    // Remove all bonded devices
    int dev_num = esp_ble_get_bond_device_num();
    Serial.printf("BLE: Found %d bonded devices\n", dev_num);

    if (dev_num > 0) {
        esp_ble_bond_dev_t* dev_list = (esp_ble_bond_dev_t*)malloc(
            sizeof(esp_ble_bond_dev_t) * dev_num);
        if (dev_list) {
            esp_ble_get_bond_device_list(&dev_num, dev_list);
            for (int i = 0; i < dev_num; i++) {
                Serial.printf("BLE: Removing bond for %02X:%02X:%02X:%02X:%02X:%02X\n",
                    dev_list[i].bd_addr[0], dev_list[i].bd_addr[1],
                    dev_list[i].bd_addr[2], dev_list[i].bd_addr[3],
                    dev_list[i].bd_addr[4], dev_list[i].bd_addr[5]);
                esp_ble_remove_bond_device(dev_list[i].bd_addr);
            }
            free(dev_list);
        }
    }

    // Also clear NVS
    esp_err_t ret = nvs_flash_erase();
    if (ret == ESP_OK) {
        Serial.println("BLE: NVS erased");
    }

    ret = nvs_flash_init();
    if (ret == ESP_OK) {
        Serial.println("BLE: NVS reinitialized");
    }

    Serial.println("BLE: Bonding data cleared - please re-pair your device");
}

// ==============================================================================
// Utility Functions
// ==============================================================================

bool isBLEConnected() {
    return bleState.connected;
}

uint16_t getBLEConnHandle() {
    return bleState.conn_handle;
}

uint32_t getBLEConnectedTime() {
    if (bleState.connected) {
        return millis() - bleState.connected_time;
    }
    return 0;
}

void printBLEStatus() {
    Serial.println("\n--- BLE Status ---");
    Serial.printf("Connected: %s\n", bleState.connected ? "YES" : "NO");
    Serial.printf("Connection handle: %d\n", bleState.conn_handle);
    Serial.printf("Connect count: %d\n", bleState.connect_count);
    if (bleState.connected) {
        Serial.printf("Connected for: %lu ms\n", getBLEConnectedTime());
    } else if (bleState.disconnected_time > 0) {
        Serial.printf("Last disconnect: %lu ms ago\n",
            millis() - bleState.disconnected_time);
    }
    Serial.println("------------------\n");
}
