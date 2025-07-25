/*
  RadioLib LoRaWAN End Device ABP Example

  This example sets up a LoRaWAN node using ABP (activation
  by personalization). Before you start, you will have to
  register your device at https://www.thethingsnetwork.org/
  After your device is registered, you can run this example.
  The device will start uploading data directly,
  without having to join the network.

  NOTE: LoRaWAN requires storing some parameters persistently!
        RadioLib does this by using EEPROM, by default
        starting at address 0 and using 32 bytes.
        If you already use EEPROM in your application,
        you will have to either avoid this range, or change it
        by setting a different start address by changing the value of
        RADIOLIB_HAL_PERSISTENT_STORAGE_BASE macro, either
        during build or in src/BuildOpt.h.

  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the library
#include <LilyGoLib.h>
#include <LV_Helper.h>


// how often to send an uplink - consider legal & FUP constraints - see notes
const uint32_t uplinkIntervalSeconds = 5UL * 60UL;    // minutes x seconds


// device address - either a development address or one assigned
// to the LoRaWAN Service Provider - TTN will generate one for you
#ifndef RADIOLIB_LORAWAN_DEV_ADDR   // Replace with your DevAddr
#define RADIOLIB_LORAWAN_DEV_ADDR       0x260D55DD
#endif

#ifndef RADIOLIB_LORAWAN_FNWKSINT_KEY   // Replace with your FNwkSInt Key 
#define RADIOLIB_LORAWAN_FNWKSINT_KEY   0x24, 0x83, 0x65, 0x51, 0x23, 0x32, 0x82, 0x47, 0xEF, 0x16, 0x4D, 0x7B, 0x0C, 0xB2, 0xC5, 0x96
#endif
#ifndef RADIOLIB_LORAWAN_SNWKSINT_KEY   // Replace with your SNwkSInt Key 
#define RADIOLIB_LORAWAN_SNWKSINT_KEY   0xAD, 0xD1, 0x50, 0x62, 0x28, 0x8F, 0xEE, 0xA4, 0xD5, 0x6B, 0x6B, 0xF3, 0x56, 0x3D, 0x5A, 0x90
#endif
#ifndef RADIOLIB_LORAWAN_NWKSENC_KEY    // Replace with your NwkSEnc Key 
#define RADIOLIB_LORAWAN_NWKSENC_KEY    0xFE, 0x9D, 0x84, 0xD9, 0x4A, 0x66, 0x69, 0x08, 0x01, 0x7B, 0x17, 0xE5, 0x38, 0x53, 0x7F, 0xD9
#endif
#ifndef RADIOLIB_LORAWAN_APPS_KEY       // Replace with your AppS Key 
#define RADIOLIB_LORAWAN_APPS_KEY       0xF4, 0x29, 0xA2, 0x5E, 0xE5, 0x49, 0x97, 0x50, 0x96, 0xB3, 0x31, 0x22, 0x34, 0xFD, 0x08, 0x5B
#endif

// for the curious, the #ifndef blocks allow for automated testing &/or you can
// put your EUI & keys in to your platformio.ini - see wiki for more tips

// regional choices: EU868, US915, AU915, AS923, IN865, KR920, CN780, CN500
const LoRaWANBand_t Region = CN500;
const uint8_t subBand = 0;  // For US915, change this to 2, otherwise leave on 0


SX1262 radio = newModule();


// copy over the keys in to the something that will not compile if incorrectly formatted
uint32_t devAddr =        RADIOLIB_LORAWAN_DEV_ADDR;
uint8_t fNwkSIntKey[] = { RADIOLIB_LORAWAN_FNWKSINT_KEY };
uint8_t sNwkSIntKey[] = { RADIOLIB_LORAWAN_SNWKSINT_KEY };
uint8_t nwkSEncKey[] =  { RADIOLIB_LORAWAN_NWKSENC_KEY };
uint8_t appSKey[] =     { RADIOLIB_LORAWAN_APPS_KEY };

// create the LoRaWAN node
LoRaWANNode node(&radio, &Region, subBand);


String stateDecode(const int16_t result);
void debug(bool isFail, const __FlashStringHelper *message, int state, bool Freeze);

void setup()
{
    Serial.begin(11520);

    watch.begin();

    Serial.println(F("Initialise the radio"));
    int state = radio.begin();
    debug(state != RADIOLIB_ERR_NONE, F("Initialise radio failed"), state, true);

    Serial.println(F("Initialise LoRaWAN Network credentials"));
    node.beginABP(devAddr, fNwkSIntKey, sNwkSIntKey, nwkSEncKey, appSKey);

    node.activateABP();
    debug(state != RADIOLIB_ERR_NONE, F("Activate ABP failed"), state, true);

    Serial.println(F("Ready!\n"));
}

// counter to keep track of transmitted packets
int count = 0;

void loop()
{
    Serial.println(F("Sending uplink"));

    // This is the place to gather the sensor inputs
    // Instead of reading any real sensor, we just generate some random numbers as example
    uint8_t value1 = radio.random(100);
    uint16_t value2 = radio.random(2000);

    // Build payload byte array
    uint8_t uplinkPayload[3];
    uplinkPayload[0] = value1;
    uplinkPayload[1] = highByte(value2);   // See notes for high/lowByte functions
    uplinkPayload[2] = lowByte(value2);

    // Perform an uplink
    int state = node.sendReceive(uplinkPayload, sizeof(uplinkPayload));
    debug(state < RADIOLIB_ERR_NONE, F("Error in sendReceive"), state, false);

    // Check if a downlink was received
    // (state 0 = no downlink, state 1/2 = downlink in window Rx1/Rx2)
    if (state > 0) {
        Serial.println(F("Received a downlink"));
    } else {
        Serial.println(F("No downlink received"));
    }

    Serial.print(F("Next uplink in "));
    Serial.print(uplinkIntervalSeconds);
    Serial.println(F(" seconds\n"));

    // Wait until next uplink - observing legal & TTN FUP constraints
    delay(uplinkIntervalSeconds * 1000UL);  // delay needs milli-seconds
}



// result code to text - these are error codes that can be raised when using LoRaWAN
// however, RadioLib has many more - see https://jgromes.github.io/RadioLib/group__status__codes.html for a complete list
String stateDecode(const int16_t result)
{
    switch (result) {
    case RADIOLIB_ERR_NONE:
        return "ERR_NONE";
    case RADIOLIB_ERR_CHIP_NOT_FOUND:
        return "ERR_CHIP_NOT_FOUND";
    case RADIOLIB_ERR_PACKET_TOO_LONG:
        return "ERR_PACKET_TOO_LONG";
    case RADIOLIB_ERR_RX_TIMEOUT:
        return "ERR_RX_TIMEOUT";
    case RADIOLIB_ERR_CRC_MISMATCH:
        return "ERR_CRC_MISMATCH";
    case RADIOLIB_ERR_INVALID_BANDWIDTH:
        return "ERR_INVALID_BANDWIDTH";
    case RADIOLIB_ERR_INVALID_SPREADING_FACTOR:
        return "ERR_INVALID_SPREADING_FACTOR";
    case RADIOLIB_ERR_INVALID_CODING_RATE:
        return "ERR_INVALID_CODING_RATE";
    case RADIOLIB_ERR_INVALID_FREQUENCY:
        return "ERR_INVALID_FREQUENCY";
    case RADIOLIB_ERR_INVALID_OUTPUT_POWER:
        return "ERR_INVALID_OUTPUT_POWER";
    case RADIOLIB_ERR_NETWORK_NOT_JOINED:
        return "RADIOLIB_ERR_NETWORK_NOT_JOINED";
    case RADIOLIB_ERR_DOWNLINK_MALFORMED:
        return "RADIOLIB_ERR_DOWNLINK_MALFORMED";
    case RADIOLIB_ERR_INVALID_REVISION:
        return "RADIOLIB_ERR_INVALID_REVISION";
    case RADIOLIB_ERR_INVALID_PORT:
        return "RADIOLIB_ERR_INVALID_PORT";
    case RADIOLIB_ERR_NO_RX_WINDOW:
        return "RADIOLIB_ERR_NO_RX_WINDOW";
    case RADIOLIB_ERR_INVALID_CID:
        return "RADIOLIB_ERR_INVALID_CID";
    case RADIOLIB_ERR_UPLINK_UNAVAILABLE:
        return "RADIOLIB_ERR_UPLINK_UNAVAILABLE";
    case RADIOLIB_ERR_COMMAND_QUEUE_FULL:
        return "RADIOLIB_ERR_COMMAND_QUEUE_FULL";
    case RADIOLIB_ERR_COMMAND_QUEUE_ITEM_NOT_FOUND:
        return "RADIOLIB_ERR_COMMAND_QUEUE_ITEM_NOT_FOUND";
    case RADIOLIB_ERR_JOIN_NONCE_INVALID:
        return "RADIOLIB_ERR_JOIN_NONCE_INVALID";
    case RADIOLIB_ERR_N_FCNT_DOWN_INVALID:
        return "RADIOLIB_ERR_N_FCNT_DOWN_INVALID";
    case RADIOLIB_ERR_A_FCNT_DOWN_INVALID:
        return "RADIOLIB_ERR_A_FCNT_DOWN_INVALID";
    case RADIOLIB_ERR_DWELL_TIME_EXCEEDED:
        return "RADIOLIB_ERR_DWELL_TIME_EXCEEDED";
    case RADIOLIB_ERR_CHECKSUM_MISMATCH:
        return "RADIOLIB_ERR_CHECKSUM_MISMATCH";
    case RADIOLIB_ERR_NO_JOIN_ACCEPT:
        return "RADIOLIB_ERR_NO_JOIN_ACCEPT";
    case RADIOLIB_LORAWAN_SESSION_RESTORED:
        return "RADIOLIB_LORAWAN_SESSION_RESTORED";
    case RADIOLIB_LORAWAN_NEW_SESSION:
        return "RADIOLIB_LORAWAN_NEW_SESSION";
    case RADIOLIB_ERR_NONCES_DISCARDED:
        return "RADIOLIB_ERR_NONCES_DISCARDED";
    case RADIOLIB_ERR_SESSION_DISCARDED:
        return "RADIOLIB_ERR_SESSION_DISCARDED";
    }
    return "See https://jgromes.github.io/RadioLib/group__status__codes.html";
}

// helper function to display any issues
void debug(bool failed, const __FlashStringHelper *message, int state, bool halt)
{
    if (failed) {
        Serial.print(message);
        Serial.print(" - ");
        Serial.print(stateDecode(state));
        Serial.print(" (");
        Serial.print(state);
        Serial.println(")");
        while (halt) {
            delay(1);
        }
    }
}

// helper function to display a byte array
void arrayDump(uint8_t *buffer, uint16_t len)
{
    for (uint16_t c = 0; c < len; c++) {
        char b = buffer[c];
        if (b < 0x10) {
            Serial.print('0');
        }
        Serial.print(b, HEX);
    }
    Serial.println();
}
