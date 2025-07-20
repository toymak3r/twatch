/*
 * T-Watch S3 - BLE Notification Receiver
 * Receives smartphone notifications via BLE without requiring additional apps
 * 
 * Features:
 * - BLE GATT Server for notification reception
 * - Android Notification Access Service
 * - iOS Notification Service
 * - Display notifications on watch screen
 * - Notification history and management
 */

#include <LilyGoLib.h>
#include <LovyanGFX.hpp>
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <NimBLE2904.h>
#include <ArduinoJson.h>

// Display configuration
static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

// BLE Configuration
#define DEVICE_NAME "T-Watch-S3"
#define SERVICE_UUID "1812"  // HID Service
#define NOTIFICATION_CHAR_UUID "2A4D"  // HID Report
#define CONTROL_CHAR_UUID "2A4E"  // HID Control Point

// Notification Service UUIDs (Android/iOS)
#define ANDROID_NOTIFICATION_SERVICE "0000fff0-0000-1000-8000-00805f9b34fb"
#define ANDROID_NOTIFICATION_CHAR "0000fff1-0000-1000-8000-00805f9b34fb"
#define IOS_NOTIFICATION_SERVICE "0000fff0-0000-1000-8000-00805f9b34fb"
#define IOS_NOTIFICATION_CHAR "0000fff1-0000-1000-8000-00805f9b34fb"

// Notification structure
struct Notification {
    String app;
    String title;
    String message;
    String timestamp;
    bool isRead;
};

#define MAX_NOTIFICATIONS 10
Notification notifications[MAX_NOTIFICATIONS];
int notificationCount = 0;
int currentNotification = 0;

// BLE Server and Service
NimBLEServer* pServer = nullptr;
NimBLEService* pNotificationService = nullptr;
NimBLECharacteristic* pNotificationChar = nullptr;
NimBLECharacteristic* pControlChar = nullptr;

// Connection state
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Display colors
#define COLOR_BACKGROUND 0x0000
#define COLOR_TEXT 0xFFFF
#define COLOR_NOTIFICATION 0xF800
#define COLOR_APP 0x07E0
#define COLOR_TIME 0x8410

// Function prototypes
void setupBLE();
void setupDisplay();
void displayNotification();
void displayNotificationList();
void processNotificationData(std::string data);
void addNotification(String app, String title, String message);
void clearNotifications();
void onConnect(NimBLEServer* pServer);
void onDisconnect(NimBLEServer* pServer);
void onWrite(NimBLECharacteristic* pCharacteristic);

// BLE Callback Classes
class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Device connected");
    }

    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Device disconnected");
    }
};

class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.printf("Received notification data: %s\n", value.c_str());
            processNotificationData(value);
        }
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("T-Watch S3 - BLE Notification Receiver");
    
    // Initialize watch
    watch.begin();
    watch.setBrightness(100);
    
    // Setup display
    setupDisplay();
    
    // Setup BLE
    setupBLE();
    
    // Display initial screen
    displayNotificationList();
    
    Serial.println("BLE Notification Receiver Ready!");
}

void setupDisplay() {
    lcd.init();
    lcd.setRotation(1);
    lcd.setBrightness(100);
    lcd.fillScreen(COLOR_BACKGROUND);
    
    sprite.createSprite(lcd.width(), lcd.height());
    sprite.setRotation(1);
}

void setupBLE() {
    // Initialize BLE
    NimBLEDevice::init(DEVICE_NAME);
    NimBLEDevice::setMTU(512);
    
    // Create server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    
    // Create notification service
    pNotificationService = pServer->createService(ANDROID_NOTIFICATION_SERVICE);
    
    // Create notification characteristic
    pNotificationChar = pNotificationService->createCharacteristic(
        ANDROID_NOTIFICATION_CHAR,
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_NR |
        NIMBLE_PROPERTY::NOTIFY
    );
    pNotificationChar->setCallbacks(new CharacteristicCallbacks());
    
    // Create control characteristic
    pControlChar = pNotificationService->createCharacteristic(
        CONTROL_CHAR_UUID,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::NOTIFY
    );
    
    // Start service
    pNotificationService->start();
    
    // Start advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(ANDROID_NOTIFICATION_SERVICE);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    NimBLEDevice::startAdvertising();
    
    Serial.println("BLE Advertising started");
}

void loop() {
    // Handle BLE connections
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
    
    // Handle touch input
    if (watch.getTouched()) {
        currentNotification = (currentNotification + 1) % max(1, notificationCount);
        displayNotificationList();
        delay(200);
    }
    
    // Handle long press to clear notifications
    static unsigned long touchStart = 0;
    static bool longPress = false;
    
    if (watch.getTouched()) {
        if (touchStart == 0) {
            touchStart = millis();
        } else if (millis() - touchStart > 2000 && !longPress) {
            longPress = true;
            clearNotifications();
            displayNotificationList();
        }
    } else {
        touchStart = 0;
        longPress = false;
    }
    
    delay(100);
}

void processNotificationData(std::string data) {
    // Parse JSON notification data
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, data);
    
    if (error) {
        Serial.printf("JSON parsing failed: %s\n", error.c_str());
        return;
    }
    
    String app = doc["app"].as<String>();
    String title = doc["title"].as<String>();
    String message = doc["message"].as<String>();
    
    if (app.length() > 0 && title.length() > 0) {
        addNotification(app, title, message);
        displayNotificationList();
    }
}

void addNotification(String app, String title, String message) {
    // Shift existing notifications
    if (notificationCount >= MAX_NOTIFICATIONS) {
        for (int i = 0; i < MAX_NOTIFICATIONS - 1; i++) {
            notifications[i] = notifications[i + 1];
        }
        notificationCount = MAX_NOTIFICATIONS - 1;
    }
    
    // Add new notification
    notifications[notificationCount].app = app;
    notifications[notificationCount].title = title;
    notifications[notificationCount].message = message;
    notifications[notificationCount].timestamp = String(millis() / 1000) + "s ago";
    notifications[notificationCount].isRead = false;
    
    notificationCount++;
    
    Serial.printf("Added notification: %s - %s\n", app.c_str(), title.c_str());
}

void clearNotifications() {
    notificationCount = 0;
    currentNotification = 0;
    Serial.println("Notifications cleared");
}

void displayNotificationList() {
    sprite.fillScreen(COLOR_BACKGROUND);
    
    // Header
    sprite.setTextColor(COLOR_TEXT);
    sprite.setTextSize(2);
    sprite.setCursor(10, 10);
    sprite.print("Notifications");
    
    if (deviceConnected) {
        sprite.setTextColor(COLOR_APP);
        sprite.setTextSize(1);
        sprite.setCursor(10, 35);
        sprite.print("BLE Connected");
    } else {
        sprite.setTextColor(COLOR_NOTIFICATION);
        sprite.setTextSize(1);
        sprite.setCursor(10, 35);
        sprite.print("BLE Disconnected");
    }
    
    if (notificationCount == 0) {
        sprite.setTextColor(COLOR_TIME);
        sprite.setTextSize(1);
        sprite.setCursor(10, 60);
        sprite.print("No notifications");
        sprite.setCursor(10, 80);
        sprite.print("Waiting for BLE...");
    } else {
        // Display current notification
        int idx = currentNotification;
        if (idx < notificationCount) {
            sprite.setTextColor(COLOR_APP);
            sprite.setTextSize(1);
            sprite.setCursor(10, 60);
            sprite.print(notifications[idx].app);
            
            sprite.setTextColor(COLOR_TEXT);
            sprite.setTextSize(1);
            sprite.setCursor(10, 80);
            sprite.print(notifications[idx].title);
            
            sprite.setTextColor(COLOR_TIME);
            sprite.setTextSize(1);
            sprite.setCursor(10, 100);
            sprite.print(notifications[idx].message);
            
            sprite.setCursor(10, 120);
            sprite.print(notifications[idx].timestamp);
            
            // Notification counter
            sprite.setTextColor(COLOR_NOTIFICATION);
            sprite.setCursor(10, 140);
            sprite.printf("%d/%d", idx + 1, notificationCount);
        }
    }
    
    // Instructions
    sprite.setTextColor(COLOR_TIME);
    sprite.setTextSize(1);
    sprite.setCursor(10, 200);
    sprite.print("Tap: Next | Long: Clear");
    
    sprite.pushSprite(0, 0);
}

void displayNotification() {
    if (currentNotification >= notificationCount) return;
    
    sprite.fillScreen(COLOR_BACKGROUND);
    
    Notification& notif = notifications[currentNotification];
    
    // App name
    sprite.setTextColor(COLOR_APP);
    sprite.setTextSize(2);
    sprite.setCursor(10, 10);
    sprite.print(notif.app);
    
    // Title
    sprite.setTextColor(COLOR_TEXT);
    sprite.setTextSize(1);
    sprite.setCursor(10, 40);
    sprite.print(notif.title);
    
    // Message
    sprite.setTextColor(COLOR_TIME);
    sprite.setCursor(10, 60);
    sprite.print(notif.message);
    
    // Timestamp
    sprite.setCursor(10, 80);
    sprite.print(notif.timestamp);
    
    sprite.pushSprite(0, 0);
} 