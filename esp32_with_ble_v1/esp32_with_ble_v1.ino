#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include <Adafruit_Sensor.h>

#include <SparkFun_VEML6075_Arduino_Library.h>
VEML6075 uv; // Create a VEML6075 object

// server name
const char *name = "BLE-ESP32";

// service und characteristic uuids vom ble-nano uebernehmen
// mehr zu uuids -> https://www.uuidgenerator.net/
// ble service uuid
#define SERV_UUID "0000ffe0-0000-1000-8000-00805f9b34fb"
// ble characteristic uuid - senden:temperatur, empangen:led-kommando
#define CHAR_UUID "0000ffe1-0000-1000-8000-00805f9b34fb"

// variables for uv logic
double currentMinutes;
double maxMinutes;
double maxIndex;
double med;
double lsf;
double uv_index;
int skinType = -1;
const double meds[] = {150.0, 250.0, 300.0, 450.0, 600.0, 900.0};
const double lsfs[] = {1.0, 6.0, 10.0, 15.0, 20.0, 30.0, 50.0};
bool timeUp = false;

// function declarations
double calcMinutes(double med, double uv_index, double lsf);
double getUVIndex();
double calcSelfProtectionTime(double med, double uv_index);
double calcProtectedMinutes(double lsf, double selfProtectionTime);

// characteristic objekt
BLECharacteristic *pCharacteristic;

// connect flags
bool devConn = false;
bool oldConn = false;

// data flag
bool dataArrived = false;

//
// ble callbacks
//

// server callbacks - connect und disconnect
class MyServerCallbacks: public BLEServerCallbacks {
  // aufruf bei connect - flag setzen
  void onConnect(BLEServer *pServer) {
    Serial.printf("! mit ble client verbunden\n");
    devConn = true;
  };
  // aufruf bei disconnect - flag ruecksetzen
  void onDisconnect(BLEServer* pServer) {
    Serial.printf("! verbindung zu ble client getrennt\n");
    // hier Hauttyp und LSF auf default setzen?
    lsf = 1.0;
    skinType = -1;
    dataArrived = false;
    devConn = false;
  }
};

// characteristic write callback - daten von ble empfangen und led steuern
class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rvalue = pCharacteristic->getValue();
    if(rvalue.length() > 0) {
      if (dataArrived) {
        // daten sind schon da
        break;
      }
      Serial.printf("* daten empfangen: ");
      for(int i = 0; i < rvalue.length(); i++) {
        Serial.printf("%02x-%c ", rvalue[i], rvalue[i]);
        if (skinType < 0) {
          // setze Hauttyp
          skinType = int(rvalue[i]);
          med = meds[skinType - 1];
        } else {
          // setze lsf
          lsf = lsfs[int(rvalue[i])];
          if (lsf == 0.0) {
            lsf = 1.0;
          }
          // flag setzen...
          dataArrived = true;
        }
      }
      Serial.printf("\n");
    }
  }
};

//
// setup
//
void setup() 
{
  Serial.begin(115200);
  Serial.println("! starte ble bme680 notify server");

   // i2c fuer uv sensor einstellen
  Wire.begin();
  // bme680 initialisieren
  if (uv.begin() == false) {
    Serial.println("Unable to communicate with  VEML6075.");
    while (1);
  }
  Serial.printf("! UV_Sensor gefunden\n");
  //Serial.println("UVA, UVB, UV Index");
  currentMinutes = 0;
  maxMinutes = 1;
  maxIndex = 0;
    
  // ble mit Geraetenamen initialisieren
  BLEDevice::init(name);
  
  // ble server erzeugen
  BLEServer *pServer = BLEDevice::createServer();
  
  // server callbacks installieren
  pServer->setCallbacks(new MyServerCallbacks());
  
  // service erzeugen
  BLEService *pService = pServer->createService(SERV_UUID);
  
  // characteristic erzeugen
  pCharacteristic = pService->createCharacteristic(
                    CHAR_UUID, 
                    BLECharacteristic::PROPERTY_READ |
                    BLECharacteristic::PROPERTY_WRITE |
                    BLECharacteristic::PROPERTY_NOTIFY |
                    BLECharacteristic::PROPERTY_INDICATE);
         
  // descriptor erzeugen - fuer notify/indicate notwendig
  pCharacteristic->addDescriptor(new BLE2902());

  // characteristic write callback anhaengen
  pCharacteristic->setCallbacks(new MyCallbacks());
  
  // service starten
  pService->start();

  // advertising starten
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERV_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); 
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("! ble server gestartet");
  Serial.println("! advertising gestartet - warte auf anrufe ...");
}
//
// loop
//
void loop() 
{
  static int cnt = 0;   // zaehler
  char svalue[32];

  if(devConn && dataArrived) {
    cnt++;
    if((cnt % 4) == 0) {
      Serial.println("We are now in the loop");
      if (currentMinutes > 0 && !timeUp) {
        double remainingTime = maxMinutes - currentMinutes;
        Serial.print("Verbleibende Zeit bis zum Sonnenbrand: ");
        Serial.println(remainingTime);
        sprintf(svalue, "%d", int(remainingTime));
        Serial.printf("* Timer #%d senden - %s\n", cnt, svalue);
        pCharacteristic->setValue(svalue);
        // notify und indicate -> neue daten pushen
        // evtl. ueberfluessig, beides zu tun (?)
        // apps nrf connect und ligthblue verhalten sich unterschiedlich
        // -> daher notify und indicate
        pCharacteristic->notify();
        pCharacteristic->indicate();
        oldConn = true;
      }
      if (currentMinutes >= maxMinutes && !timeUp) {
        timeUp = true;
        Serial.println("Aus der Sonne!"); 
        sprintf(svalue, "%s", "Aus der Sonne");
        Serial.printf("* Timer #%d senden - %s\n", cnt, svalue);
        pCharacteristic->setValue(svalue);
        // notify und indicate -> neue daten pushen
        // evtl. ueberfluessig, beides zu tun (?)
        // apps nrf connect und ligthblue verhalten sich unterschiedlich
        // -> daher notify und indicate
        pCharacteristic->notify();
        pCharacteristic->indicate();
        oldConn = true;
      }
      if (uv_index > 2) {
        currentMinutes++;
      }
      if (int(uv_index) <= maxIndex) {
        delay(250);
        return;
      }
      if (maxIndex == 0) {
        Serial.println("maxIndex == 0 -> Werte initial berechnet...");
        double protectedMinutes = calcMinutes(med, uv_index, lsf);
        maxMinutes = round(protectedMinutes);
        delay(250);
        return;
      }
      double newProtectedMinutes = calcMinutes(med, uv_index, lsf);
      currentMinutes = round((currentMinutes / maxMinutes) * newProtectedMinutes);
      maxMinutes = round(newProtectedMinutes);
      
    }
    // schleifenzaehler hochzaehlen
    ++cnt;  
  }
  
  // nicht verbunden -> advertising erneut starten
  else if(oldConn) {
    oldConn = false;
    Serial.printf("! starte advertising\n");
    BLEDevice::startAdvertising();
  }

  delay(250);  
}

double getUVIndex() {
  double uv_index = uv.index();
  Serial.println("UV-Index=" + String(uv_index));
  return uv_index;
}

double calcMinutes(double med, double uv_index, double lsf) {
  double protectedMinutes = calcProtectedMinutes(lsf, calcSelfProtectionTime(med, uv_index));
  maxIndex = int(uv_index);
  return protectedMinutes;
}

double calcSelfProtectionTime(double med, double uv_index) {
  return (med / (uv_index * 1.5));
}

double calcProtectedMinutes(double lsf, double selfProtectionTime) {
  return (selfProtectionTime * lsf * 0.6);
}
