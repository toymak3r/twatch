# 📱 BLE (Bluetooth Low Energy) no T-Watch S3

## ✅ Capacidades BLE

O **T-Watch S3** possui suporte completo a **Bluetooth Low Energy (BLE)** através do chip **ESP32-S3**.

### 🔧 Hardware Suportado
- **ESP32-S3**: Suporte nativo a BLE 5.0
- **Frequência**: 2.4GHz
- **Alcance**: ~10-30 metros (dependendo do ambiente)
- **Consumo**: Baixo (otimizado para wearables)

## 🚀 Funcionalidades Possíveis

### 1. 📱 App Companion
- **Sincronização de dados** com smartphone
- **Configurações remotas** via app
- **Notificações push** do celular
- **Controle de música** (play/pause, volume)

### 2. 📊 Health & Fitness
- **Sincronização de passos** com apps de fitness
- **Dados de batimentos** (se implementar sensor)
- **Temperatura corporal** (se implementar sensor)
- **Sono e atividade** (acelerômetro)

### 3. ⚙️ Configurações
- **Sincronização de tempo** via celular
- **Configurações de display** (brilho, tema)
- **Configurações de alarme**
- **Atualizações OTA** (Over-The-Air)

### 4. 🔔 Notificações
- **SMS/WhatsApp** notifications
- **Chamadas** (atender/rejeitar)
- **Email** notifications
- **Apps sociais** (Instagram, Facebook, etc.)

## 📋 Exemplo Implementado

### Arquivo: `ble_companion.ino`

Este exemplo demonstra:

#### 🔧 Serviços BLE Criados:
1. **Health Service** (`4fafc201-1fb5-459e-8fcc-c5c9c331914b`)
   - Passos (`2a6d`)
   - Batimentos (`2a37`) 
   - Temperatura (`2a6e`)

2. **Battery Service** (`180f`)
   - Nível da bateria (`2a19`)
   - Status de carregamento

3. **Notifications Service** (`1812`)
   - Receber notificações (`2a08`)

4. **Config Service** (`180a`)
   - Configurações (`2a29`)

#### 📡 Funcionalidades:
- **Servidor GATT** completo
- **Advertising** automático
- **Reconexão** automática
- **Callbacks** de conexão/desconexão
- **Comandos** bidirecionais

## 🛠️ Como Usar

### 1. Compilar e Upload
```bash
pio run -e twatch-s3 --target upload
```

### 2. Conectar com Celular
1. **Ativar Bluetooth** no celular
2. **Procurar dispositivos** - deve aparecer "T-Watch-S3"
3. **Conectar** ao dispositivo
4. **Autorizar** a conexão

### 3. Apps de Teste
- **nRF Connect** (Android/iOS) - Para testar BLE
- **BLE Scanner** (Android) - Para debug
- **LightBlue** (iOS) - Para desenvolvimento

## 📱 Desenvolvimento de App

### Para Android:
```kotlin
// Exemplo básico de conexão
private fun connectToWatch() {
    val bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
    val device = bluetoothAdapter.getRemoteDevice("MAC_ADDRESS")
    
    device.connectGatt(context, false, object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                // Conectado!
                gatt.discoverServices()
            }
        }
        
        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            // Serviços descobertos
            val service = gatt.getService(UUID.fromString("4fafc201-1fb5-459e-8fcc-c5c9c331914b"))
            val characteristic = service.getCharacteristic(UUID.fromString("2a6d"))
            gatt.readCharacteristic(characteristic)
        }
    })
}
```

### Para iOS:
```swift
// Exemplo básico de conexão
func connectToWatch() {
    let centralManager = CBCentralManager()
    centralManager.scanForPeripherals(withServices: nil, options: nil)
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if peripheral.name == "T-Watch-S3" {
            central.connect(peripheral, options: nil)
        }
    }
}
```

## 🔧 Comandos Disponíveis

### Enviados pelo Celular:
- `GET_STEPS` - Solicitar contagem de passos
- `GET_BATTERY` - Solicitar status da bateria
- `GET_TEMP` - Solicitar temperatura
- `SYNC_TIME` - Sincronizar tempo

### Recebidos pelo Celular:
- **Passos**: Valor numérico (4 bytes)
- **Bateria**: Nível (1 byte) + Status carregamento (1 byte)
- **Temperatura**: Float (4 bytes)

## ⚡ Otimizações de Bateria

### Implementadas:
- **Advertising** apenas quando necessário
- **Intervalos** de atualização configuráveis
- **Sleep mode** quando desconectado
- **Reconexão** inteligente

### Configurações:
```cpp
// Intervalos de atualização
#define UPDATE_INTERVAL 5000  // 5 segundos
#define ADVERTISING_INTERVAL 1000  // 1 segundo

// Power saving
WiFi.setSleep(true);  // WiFi sleep mode
setCpuFrequencyMhz(80);  // CPU reduzido
```

## 🚀 Próximos Passos

### 1. App Mobile
- Desenvolver app Android/iOS completo
- Interface para configurações
- Dashboard de dados de saúde
- Notificações push

### 2. Funcionalidades Avançadas
- **GPS assistido** via celular
- **Sincronização de agenda**
- **Controle de smart home**
- **Pagamentos** (NFC + BLE)

### 3. Integração com Apps
- **Google Fit** / **Apple Health**
- **Strava** / **Garmin Connect**
- **WhatsApp** / **Telegram**
- **Spotify** / **YouTube Music**

## 🔍 Debug e Troubleshooting

### Problemas Comuns:
1. **Não aparece no scan**
   - Verificar se BLE está ativo
   - Verificar advertising interval
   - Verificar nome do dispositivo

2. **Conexão instável**
   - Ajustar parâmetros de conexão
   - Verificar interferência WiFi
   - Verificar distância

3. **Dados não atualizam**
   - Verificar callbacks
   - Verificar UUIDs
   - Verificar permissões

### Debug Serial:
```
Iniciando T-Watch S3 BLE Companion...
PMU inicializado!
Acelerômetro inicializado!
BLE iniciado! Procurando por: T-Watch-S3
Dispositivo conectado via BLE!
Passos atualizados: 1234
Bateria atualizada: 85% (carregando: NÃO)
```

## 📚 Recursos Adicionais

- [ESP32 BLE Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/bluetooth.html)
- [Arduino BLE Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE)
- [GATT Services](https://www.bluetooth.com/specifications/gatt/services/)
- [BLE UUIDs](https://www.bluetooth.com/specifications/assigned-numbers/) 