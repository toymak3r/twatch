/*
 * T-Watch S3 BLE Companion Example
 * 
 * Este exemplo demonstra como usar BLE no T-Watch S3 para:
 * - Criar um servidor GATT
 * - Enviar dados de saúde (passos, bateria, temperatura)
 * - Receber comandos do celular
 * - Sincronizar configurações
 * 
 * Funcionalidades:
 * - Serviço de saúde (passos, batimentos)
 * - Serviço de bateria (nível, carregando)
 * - Serviço de notificações
 * - Serviço de configuração
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <LovyanGFX.hpp>
#include <XPowersLib.h>
#include <SensorsLib.h>

// Configurações BLE
#define DEVICE_NAME "T-Watch-S3"
#define SERVICE_UUID_HEALTH "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SERVICE_UUID_BATTERY "180f"
#define SERVICE_UUID_NOTIFICATIONS "1812"
#define SERVICE_UUID_CONFIG "180a"

// Características UUIDs
#define CHAR_UUID_STEPS "2a6d"
#define CHAR_UUID_HEART_RATE "2a37"
#define CHAR_UUID_BATTERY_LEVEL "2a19"
#define CHAR_UUID_TEMPERATURE "2a6e"
#define CHAR_UUID_NOTIFICATIONS "2a08"
#define CHAR_UUID_CONFIG "2a29"

// Objetos globais
BLEServer* pServer = nullptr;
BLECharacteristic* pStepsCharacteristic = nullptr;
BLECharacteristic* pHeartRateCharacteristic = nullptr;
BLECharacteristic* pBatteryCharacteristic = nullptr;
BLECharacteristic* pTemperatureCharacteristic = nullptr;
BLECharacteristic* pNotificationsCharacteristic = nullptr;
BLECharacteristic* pConfigCharacteristic = nullptr;

bool deviceConnected = false;
bool oldDeviceConnected = false;

// Display
static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

// Sensores
XPowersPMU PMU;
SensorsLib BMA;

// Dados de saúde
uint32_t stepCount = 0;
uint8_t heartRate = 0;
uint8_t batteryLevel = 0;
float temperature = 0.0;
bool isCharging = false;

// Callbacks de conexão BLE
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Dispositivo conectado via BLE!");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Dispositivo desconectado via BLE!");
        // Reiniciar advertising
        pServer->getAdvertising()->start();
    }
};

// Callbacks de característica
class CharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.print("Comando recebido: ");
            for (int i = 0; i < value.length(); i++) {
                Serial.print(value[i]);
            }
            Serial.println();
            
            // Processar comandos
            processCommand(value);
        }
    }
};

void processCommand(std::string command) {
    if (command == "GET_STEPS") {
        updateSteps();
    } else if (command == "GET_BATTERY") {
        updateBattery();
    } else if (command == "GET_TEMP") {
        updateTemperature();
    } else if (command == "SYNC_TIME") {
        // Sincronizar tempo (implementar)
        Serial.println("Sincronização de tempo solicitada");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Iniciando T-Watch S3 BLE Companion...");
    
    // Inicializar display
    lcd.init();
    lcd.setRotation(1);
    lcd.setBrightness(128);
    lcd.fillScreen(TFT_BLACK);
    
    // Inicializar sensores
    initSensors();
    
    // Inicializar BLE
    initBLE();
    
    Serial.println("Setup completo!");
}

void initSensors() {
    // Inicializar PMU
    if (PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("PMU inicializado!");
        PMU.enableDC3();
        PMU.enableALDO2();
        PMU.enableALDO3();
        PMU.enableALDO4();
    }
    
    // Inicializar acelerômetro
    if (BMA.begin(Wire, BMA423_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("Acelerômetro inicializado!");
        BMA.enableAccelerometer();
        BMA.enablePedometer();
    }
}

void initBLE() {
    // Inicializar BLE
    BLEDevice::init(DEVICE_NAME);
    
    // Criar servidor BLE
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // Criar serviços
    createHealthService();
    createBatteryService();
    createNotificationsService();
    createConfigService();
    
    // Iniciar advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID_HEALTH);
    pAdvertising->addServiceUUID(SERVICE_UUID_BATTERY);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Funções que ajudam com iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE iniciado! Procurando por: " DEVICE_NAME);
}

void createHealthService() {
    BLEService *pHealthService = pServer->createService(SERVICE_UUID_HEALTH);
    
    // Característica de passos
    pStepsCharacteristic = pHealthService->createCharacteristic(
        CHAR_UUID_STEPS,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pStepsCharacteristic->addDescriptor(new BLE2902());
    
    // Característica de batimentos (simulada)
    pHeartRateCharacteristic = pHealthService->createCharacteristic(
        CHAR_UUID_HEART_RATE,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pHeartRateCharacteristic->addDescriptor(new BLE2902());
    
    // Característica de temperatura
    pTemperatureCharacteristic = pHealthService->createCharacteristic(
        CHAR_UUID_TEMPERATURE,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pTemperatureCharacteristic->addDescriptor(new BLE2902());
    
    pHealthService->start();
}

void createBatteryService() {
    BLEService *pBatteryService = pServer->createService(SERVICE_UUID_BATTERY);
    
    pBatteryCharacteristic = pBatteryService->createCharacteristic(
        CHAR_UUID_BATTERY_LEVEL,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pBatteryCharacteristic->addDescriptor(new BLE2902());
    
    pBatteryService->start();
}

void createNotificationsService() {
    BLEService *pNotificationsService = pServer->createService(SERVICE_UUID_NOTIFICATIONS);
    
    pNotificationsCharacteristic = pNotificationsService->createCharacteristic(
        CHAR_UUID_NOTIFICATIONS,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
    );
    pNotificationsCharacteristic->addDescriptor(new BLE2902());
    pNotificationsCharacteristic->setCallbacks(new CharacteristicCallbacks());
    
    pNotificationsService->start();
}

void createConfigService() {
    BLEService *pConfigService = pServer->createService(SERVICE_UUID_CONFIG);
    
    pConfigCharacteristic = pConfigService->createCharacteristic(
        CHAR_UUID_CONFIG,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pConfigCharacteristic->setCallbacks(new CharacteristicCallbacks());
    
    pConfigService->start();
}

void updateSteps() {
    if (BMA.isPedometer()) {
        stepCount = BMA.getPedometerCounter();
        pStepsCharacteristic->setValue((uint8_t*)&stepCount, 4);
        pStepsCharacteristic->notify();
        Serial.printf("Passos atualizados: %lu\n", stepCount);
    }
}

void updateBattery() {
    if (PMU.isBatteryConnect()) {
        batteryLevel = PMU.getBatteryPercent();
        isCharging = PMU.isCharging();
        
        uint8_t batteryData[2] = {batteryLevel, isCharging ? 1 : 0};
        pBatteryCharacteristic->setValue(batteryData, 2);
        pBatteryCharacteristic->notify();
        Serial.printf("Bateria atualizada: %d%% (carregando: %s)\n", batteryLevel, isCharging ? "SIM" : "NÃO");
    }
}

void updateTemperature() {
    // Simular temperatura (implementar sensor real)
    temperature = 25.0 + random(-5, 5);
    pTemperatureCharacteristic->setValue((uint8_t*)&temperature, 4);
    pTemperatureCharacteristic->notify();
    Serial.printf("Temperatura atualizada: %.1f°C\n", temperature);
}

void loop() {
    // Atualizar dados a cada 5 segundos se conectado
    static unsigned long lastUpdate = 0;
    if (deviceConnected && (millis() - lastUpdate > 5000)) {
        updateSteps();
        updateBattery();
        updateTemperature();
        lastUpdate = millis();
    }
    
    // Gerenciar reconexão
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // Dar tempo para a conexão se estabelecer
        pServer->startAdvertising();
        oldDeviceConnected = deviceConnected;
    }
    
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
    
    // Atualizar display
    updateDisplay();
    
    delay(1000);
}

void updateDisplay() {
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_WHITE);
    lcd.setTextSize(2);
    
    lcd.setCursor(10, 10);
    lcd.print("T-Watch S3 BLE");
    
    lcd.setTextSize(1);
    lcd.setCursor(10, 40);
    lcd.printf("Status: %s", deviceConnected ? "Conectado" : "Desconectado");
    
    lcd.setCursor(10, 60);
    lcd.printf("Passos: %lu", stepCount);
    
    lcd.setCursor(10, 80);
    lcd.printf("Bateria: %d%%", batteryLevel);
    
    lcd.setCursor(10, 100);
    lcd.printf("Temp: %.1fC", temperature);
    
    lcd.setCursor(10, 120);
    lcd.printf("Carregando: %s", isCharging ? "SIM" : "NAO");
} 