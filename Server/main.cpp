/******************************************************************************

u-blox NINA-W10 Arduino example
Leonardo Bispo
March - 2019
https://github.com/ldab/Arduino-u-blox-NINA-W10

Distributed as-is; no warranty is given.

This example tries to implement some key functions and key PIN atributes in order
to get you started with using Arduino and the espresif ESP32 board u-blox NINA-W10

Timer functionas are implemented intead of delay() and the PINs have been re-mapped
on the #define section

*************   SERVER example, connects to a Client Central.   ***************

******************************************************************************/

#include "Arduino.h"
#include "Wire.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Ticker.h>
#include <Adafruit_SHT31.h>

/******************     NINA-W10 PIN Definition      ************************************
 * .platformio\packages\framework-arduinoespressif32\variants\nina_w10
LED_GREEN = 33;
LED_RED   = 23;
LED_BLUE  = 21;

SW2       = 27;

SDA       = 12;
SCL       = 13;
***************************************************************************************/

// UUIDs can be unique generated at https://www.uuidgenerator.net/
// or follow BLE GATTs https://www.bluetooth.com/specifications/gatt/services

// Environmental Sensing -> https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.environmental_sensing.xml
static BLEUUID serviceUUID("181A");
// The characteristic of data we will provide
static BLEUUID tcharUUID("2A6E");
static BLEUUID hcharUUID("2A6F");
// This device name
const char charNAME[] = "NINA-W10";

uint32_t passkey = 0420;

// Create functions prior to calling them as .cpp files are differnt from Arduino .ino
void readSensor ( void );
void blinky     ( void );
void colorLED   ( void );

// Initialize the Temperature and Humidity Sensor SHT31
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Create timers using Ticker library in oder to avoid delay()
Ticker blinkIt;
Ticker readIt;

// Declare Global variables
float t = NAN;
float h = NAN;

bool connected = false;

class MyServerCallback : public BLEServerCallbacks {
  void onConnect(BLEClient* pclient) 
  {

  }

  void onDisconnect(BLEClient* pclient) 
  {
    connected = false;
    Serial.println("onDisconnect");
  }
};

void setup()
{
  pinMode( LED_RED  , OUTPUT );
  pinMode( LED_GREEN, OUTPUT );
  pinMode( LED_BLUE , OUTPUT );
  digitalWrite( LED_RED  , HIGH );
  digitalWrite( LED_GREEN, HIGH );
  digitalWrite( LED_BLUE , HIGH );

  pinMode( SW2, INPUT_PULLUP );
  attachInterrupt( SW2, colorLED, FALLING );

  Serial.begin(115200);

  if( !sht31.begin(0x44) ){
    Serial.println("Failed to find sensor, please check wiring and address");
  }
  
  BLEDevice::init(charNAME);

  // Create a GATT Server
  BLEServer *pServer = BLEDevice::createServer();
  // Create the service
  BLEService *pService = pServer->createService(serviceUUID);
  // Create the characteristic
  BLECharacteristic* tCharacteristic = pService->createCharacteristic(tcharUUID, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic* hCharacteristic = pService->createCharacteristic(hcharUUID, BLECharacteristic::PROPERTY_READ);
  tCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  hCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  // Set the characteristic value
  readSensor();
  tCharacteristic->setValue( t );
  hCharacteristic->setValue( h );
  // Start the service
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  // Security, using defined PIN number -> https://github.com/nkolban/esp32-snippets/issues/793#issuecomment-458947313
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;     //bonding with peer device after authentication
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
    uint8_t key_size = 16;      //the key size should be 7~16 bytes
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    //set static passkey
    uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
    uint8_t oob_support = ESP_BLE_OOB_DISABLE;
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
    // esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(uint8_t));
    /* If your BLE device act as a Slave, the init_key means you hope which types of key of the master should distribut to you,
    and the response key means which key you can distribut to the Master;
    If your BLE device act as a master, the response key means you hope which types of key of the slave should distribut to you,
    and the init key means which key you can distribut to the slave. */
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
  
  // Start Timers, read sensor and blink Blue LED
  readIt.attach( 5, readSensor );
  blinkIt.attach( 1, blinky );
}

void loop()
{

  // Should probably put this on a Timer but would work for this example.
  delay(2000);

}

void readSensor( void )
{
  t = sht31.readTemperature();
  h = sht31.readHumidity();

  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp ºC = "); Serial.println(t);
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
  } else { 
    Serial.println("Failed to read humidity");
  }
  Serial.println();
  
}

// Toggle RGB LED
void blinky( void )
{
  uint8_t led = 0;

  if( connected )
    led = LED_GREEN;
  else 
    led = LED_RED;

  digitalWrite( led, !digitalRead( led ));
}

// Random function just to show Switch 2
void colorLED( void )
{
  uint8_t ran = random(0,3);
  
  switch (ran)
  {
  case 1:
    digitalWrite( LED_RED, LOW );
    break;
  
  case 2:
    digitalWrite( LED_GREEN, LOW );
    break;

  default:
    digitalWrite( LED_GREEN, HIGH );
    digitalWrite( LED_RED, HIGH );
    break;
  }

}
