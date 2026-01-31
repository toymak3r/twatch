/**
 * @file      WirelessBasic.ino
 * @author    T-Watch S3 Wireless Basics
 * @license   MIT
 * @date      2026-01-31
 * 
 * ESSENTIAL EXAMPLE - WiFi and Bluetooth
 * Learn: Basic wireless connectivity, power management
 */

#include <LilyGoLib.h>
#include <LV_Helper.h>
#include <WiFi.h>
#include <BluetoothSerial.h>

// Wireless globals
lv_obj_t *wirelessLabel;
lv_obj_t *wifiLabel;
lv_obj_t *btLabel;
lv_obj_t *statusLabel;
BluetoothSerial SerialBT;

// WiFi credentials (change these)
const char* ssid = "YourWiFiNetwork";
const char* password = "YourPassword";

void setup() {
    Serial.begin(115200);
    Serial.println("=== T-Watch Wireless Basics ===");

    watch.begin();
    beginLvglHelper();
    
    createWirelessUI();
    
    // Initialize Bluetooth
    SerialBT.begin("T-Watch-S3");
    
    // Apply moderate power settings
    watch.setPowerProfile(POWER_PROFILE_BALANCED);
    
    Serial.println("✅ Wireless initialized");
    Serial.println("📡 Starting WiFi scan...");
    
    // Start WiFi scan (non-blocking)
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.scanDelete();
    WiFi.scanNetworks(false, true, false, 300); // 300ms per channel
}

void loop() {
    static unsigned long lastUpdate = 0;
    static int networkCount = -1;
    
    // Check WiFi scan results
    int foundNetworks = WiFi.scanComplete();
    if (foundNetworks >= 0) {
        networkCount = foundNetworks;
        Serial.printf("📡 Found %d WiFi networks\n", networkCount);
    }
    
    // Update UI every 2 seconds
    if (millis() - lastUpdate > 2000) {
        updateWirelessStatus(networkCount);
        lastUpdate = millis();
    }
    
    // Handle Bluetooth data
    if (SerialBT.available()) {
        String btData = SerialBT.readString();
        Serial.printf("🔵 BT RX: %s\n", btData.c_str());
        lv_label_set_text(statusLabel, ("🔵 BT: " + btData).c_str());
    }
    
    // Check for touch to send BT message
    handleTouchBT();
    
    lv_task_handler();
    delay(100);
}

void createWirelessUI() {
    // Dark theme
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    
    // Title
    wirelessLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(wirelessLabel, "📡 T-Watch Wireless");
    lv_obj_set_style_text_color(wirelessLabel, lv_color_hex(0x00ff00), 0);
    lv_obj_align(wirelessLabel, LV_ALIGN_TOP_MID, 0, 10);
    
    // WiFi status
    wifiLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(wifiLabel, "WiFi: Scanning...");
    lv_obj_set_style_text_color(wifiLabel, lv_color_hex(0x00ffff), 0);
    lv_obj_align(wifiLabel, LV_ALIGN_CENTER, 0, -30);
    
    // Bluetooth status
    btLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(btLabel, "BT: T-Watch-S3");
    lv_obj_set_style_text_color(btLabel, lv_color_hex(0xff00ff), 0);
    lv_obj_align(btLabel, LV_ALIGN_CENTER, 0, 0);
    
    // Status
    statusLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(statusLabel, "👆 Touch to send BT message");
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(statusLabel, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    watch.setBrightness(100);
}

void updateWirelessStatus(int networkCount) {
    // Update WiFi status
    if (networkCount >= 0) {
        lv_label_set_text_fmt(wifiLabel, "📡 WiFi: %d networks found", networkCount);
    } else {
        lv_label_set_text(wifiLabel, "📡 WiFi: Scanning...");
    }
    
    // Update Bluetooth status
    bool btConnected = SerialBT.hasClient();
    lv_label_set_text_fmt(btLabel, "🔵 BT: %s (%d)",
                         SerialBT.hasClient() ? "Connected" : "Waiting",
                         SerialBT.available());
    
    // Update system status
    Serial.printf("📊 Status: WiFi Networks: %d, BT Clients: %d\n",
                 max(0, networkCount), btConnected ? 1 : 0);
}

void handleTouchBT() {
    static unsigned long lastTouch = 0;
    
    if (watch.getTouched() && (millis() - lastTouch > 1000)) {
        lastTouch = millis();
        
        // Send test message via Bluetooth
        String message = "Hello from T-Watch! Time: " + String(millis() / 1000) + "s";
        SerialBT.println(message);
        
        lv_label_set_text(statusLabel, "📤 BT message sent!");
        Serial.printf("📤 BT TX: %s\n", message.c_str());
    }
}

void connectToWiFi() {
    Serial.printf("🔌 Connecting to WiFi: %s\n", ssid);
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n✅ WiFi Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        lv_label_set_text_fmt(wifiLabel, "🌐 WiFi: %s", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\n❌ WiFi connection failed");
        lv_label_set_text(wifiLabel, "❌ WiFi: Connection failed");
    }
}

void startWiFiScan() {
    Serial.println("🔍 Starting new WiFi scan...");
    WiFi.scanDelete();
    WiFi.scanNetworks(false, true, false, 300); // Power efficient scan
}

void optimizeWirelessPower() {
    // Enable WiFi power saving
    WiFi.setSleep(true);
    
    // Set WiFi transmission power (lower = less power)
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // Moderate power
    
    // Enable Bluetooth power saving
    // Note: ESP32 Bluetooth has limited power management options
    
    Serial.println("⚡ Wireless power optimization applied");
}

void demonstrateLowPowerWireless() {
    Serial.println("🔋 Low power wireless demo...");
    
    // Set maximum power saving
    WiFi.setSleep(true);
    WiFi.setTxPower(WIFI_POWER_2dBm); // Very low power
    
    // Disconnect from WiFi to save power
    WiFi.disconnect();
    
    // Put Bluetooth in discoverable mode only
    SerialBT.end();
    delay(1000);
    SerialBT.begin("T-Watch-LP");
    
    lv_label_set_text(wifiLabel, "🔋 WiFi: Off (power save)");
    lv_label_set_text(btLabel, "🔵 BT: Low power mode");
    
    Serial.println("✅ Low power wireless mode active");
}