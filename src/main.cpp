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

******************************************************************************/

#include "Arduino.h"
#include "Wire.h"
#include "BLEDevice.h"
#include "Ticker.h"
#include "Adafruit_SHT31.h"

#define RGB_R         13  // P0.13 - GPIO1
#define RGB_G         25  // P0.25 - GPIO7 BOOT_I - *THIS PIN IS SHARED WITH SWITCH 1*
#define RGB_B         32  // P1.00 - GPIO8 SWO_I

// The remote service we wish to connect to.
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan    = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice*     myDevice;

// Create functions prior to calling them as .cpp files are differnt from Arduino .ino
void setupBLE   ( void );
void blinky     ( void );
void colorLED   ( void );
void readSensor ( void );
bool connectToServer( void );

// Initialize the Temperature and Humidity Sensor SHT31
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Create timers using Ticker library in oder to avoid delay()
Ticker blinkLED ( blinky, 600 );
Ticker readIT   ( readSensor, 5000 );

// Declare Global variables
float t = NAN;
float h = NAN;

static void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic,
                            uint8_t* pData,
                            size_t length,
                            bool isNotify )
{
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

// Scan for BLE servers and find the first one that advertises the service we are looking for
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
    {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    } // Found our server
  } // onResult
};

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
}

void setup()
{
  Serial.begin(115200);

  readIT.start();

  if( !sht31.begin(0x44) ){
    Serial.println("Failed to find sensor, please check wiring and address");
  }
  
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

}

void loop()
{
  // Keep feeding timers
  blinkLED.update();
  readIT.update();

  // If "doConnect" is true BLE Server has been found, Now we connect to it.
  if (doConnect == true) 
  {
    if ( connectToServer() ) // Connect to the BLE Server found 
    {
      Serial.println("We are now connected to the BLE Server.");
    } 
    else
    {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }

    doConnect = false;
  }

    // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected)
  {
    String newValue = "Time since boot: " + String(millis()/1000);
    Serial.println("Setting new characteristic value to \"" + newValue + "\"");
    
    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  }
  else if(doScan)
  {
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }

} // end of loop

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

void blinky( void )
{
  // Toggle LED_DS1
  //digitalWrite(LED_DS1, LED);

  // Toggle LED_RTS
  //digitalWrite(LED_RTS, !LED);

}