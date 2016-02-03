#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <SPI.h>
#include "LedControlSPIESP8266.h"
#include <Base64.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <IRremoteESP8266.h>
#include "DHT.h"
#include <FS.h>
#include <elapsedMillis.h>


//holds the current upload
File UploadFile;
String fileName;

//-------------- FSBrowser application -----------
//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}


DHT dht = DHT(0, DHT11);
IRsend irsend(5); //an IR led is connected to GPIO pin 0

String sep = "____";
String ESPimaticVersion = "0.1.25";
String DS18B20Enabled = "0";
String DHTEnabled = "0";
String MatrixEnabled = "0";
String LastDS18B20 = "";
String LastDHTtemp = "";
String LastDHThum = "";
String ShowOnMatrix = "";
String WMode = "";
int FSTotal;
int FSUsed;
String MatrixIntensity;
String DeviceName;
String EnableWebAuth;
String BSlocal;
String ADCEnabled;
int ADC;

// EEPROM Adress & Length
#define ssid_Address 0
#define password_Address 32
#define pimhost_Address 97
#define pimport_Address 129
#define pimuser_Address 134
#define pimpass_Address 145
#define enablematrix_Address 165
#define matrixpin_Address 166
#define enableds18b20_Address 168
#define ds18b20pin_Address 169
#define enabledht_Address 171
#define dhttype_Address 172
#define dhtpin_Address 173
#define enablesleep_Address 175
#define ds18b20var_Address 176
#define ds18b20interval_Address 206
#define ds18b20resolution_Address 208
#define enableir_Address 210
#define irpin_Address 211
#define enablerelay_Address 213
#define relay1pin_Address 214
#define relay2pin_Address 216
#define relay3pin_Address 218
#define dhttempvar_Address 220
#define dhthumvar_Address 250
#define dhtinterval_Address 280
#define relay4pin_Address 282
#define eeprommd5_Address 284
#define version_Address 316
#define availablegpio_Address 324
#define showonmatrix_Address 341
#define matrixintensity_Address 342
#define relay1type_Address 344
#define relay2type_Address 345
#define relay3type_Address 346
#define relay4type_Address 347
#define devicename_Address 348
#define webuser_Address 378
#define webpass_Address 403
#define enablewebauth_Address 429
#define espimaticapikey_Address 430
#define bslocal_Address 445
#define enableadc_Address 446
#define adcinterval_Address 447
#define adcvar_Address 449
#define enableled_Address 449
#define led1pin_Address 479
#define led2pin_Address 481
#define led3pin_Address 483
int EepromAdress[] = {ssid_Address, password_Address, pimhost_Address, pimport_Address, pimuser_Address, pimpass_Address, enablematrix_Address, matrixpin_Address, enableds18b20_Address, ds18b20pin_Address, enabledht_Address, dhttype_Address, dhtpin_Address, enablesleep_Address, ds18b20var_Address, ds18b20interval_Address, ds18b20resolution_Address, enableir_Address, irpin_Address, enablerelay_Address, relay1pin_Address, relay2pin_Address, relay3pin_Address, dhttempvar_Address, dhthumvar_Address, dhtinterval_Address, relay4pin_Address, eeprommd5_Address, version_Address, availablegpio_Address, showonmatrix_Address, matrixintensity_Address, relay1type_Address, relay2type_Address, relay3type_Address, relay4type_Address, devicename_Address, webuser_Address, webpass_Address, enablewebauth_Address, espimaticapikey_Address, bslocal_Address, enableadc_Address, adcinterval_Address, adcvar_Address, enableled_Address, led1pin_Address, led2pin_Address, led3pin_Address};
int EepromLength[] = {31, 65, 32, 5, 11, 20, 1, 2, 1, 2, 1, 1, 2, 1, 30, 2, 2, 1, 2, 1, 2, 2, 2, 30, 30, 2, 2, 32, 8, 17, 1, 2, 1, 1 ,1 ,1, 30, 25, 25, 1, 15, 1, 1, 2, 30};
int StartAddress = 0;

#define ErrorWifi 0
#define ErrorEeprom 1
#define ErrorDs18b20 2
#define ErrorUpgrade 3
#define ErrorDht 4
int ErrorList[] = {0, 0, 0, 0, 0};

// Timer stuff
elapsedMillis timeElapsed; //declare global if you don't want it reset every time loop runs


#define BASE64_LEN 40
char unameenc[BASE64_LEN];
uint8_t ONE_WIRE_BUS;
OneWire oneWire = OneWire(ONE_WIRE_BUS); // Setup a oneWire instance
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature
DeviceAddress insideThermometer;

uint8_t LEDpin;
//LedControl lc=LedControl(2,2); // Load pin, number of LED displays
LedControl lc = LedControl(LEDpin, 2); // Load pin, number of LED displays

/* we always wait a bit between updates of the display */
unsigned long delaytime = 100;


const char* APssid = "ESPimatic";
const char* APpassword = "espimatic";

long ds18b20_sendInterval    = 60000; //in millis
long ds18b20_lastInterval  = 0;
long dht_sendInterval    = 60000; //in millis
long dht_lastInterval  = 0;
long adc_sendInterval    = 60000; //in millis
long adc_lastInterval  = 0;

ESP8266WebServer  server(80);
//String ClientIP;
WiFiClient client;


const static byte alphabetBitmap[41][8] = {
  {0x0, 0x0, 0x7E, 0x81, 0x81, 0x81, 0x7E, 0x0}, //0
  {0x0, 0x0, 0x4, 0x82, 0xFF, 0x80, 0x0, 0x0}, //1
  {0x0, 0x0, 0xE2, 0x91, 0x91, 0x91, 0x8E, 0x0}, //2
  {0x0, 0x0, 0x42, 0x89, 0x89, 0x89, 0x76, 0x0}, //3
  {0x0, 0x0, 0x1F, 0x10, 0x10, 0xFF, 0x10, 0x0}, //4
  {0x0, 0x0, 0x8F, 0x89, 0x89, 0x89, 0x71, 0x0}, //5
  {0x0, 0x0, 0x7E, 0x89, 0x89, 0x89, 0x71, 0x0}, //6
  {0x0, 0x0, 0x1, 0x1, 0xF9, 0x5, 0x3, 0x0}, //7
  {0x0, 0x0, 0x76, 0x89, 0x89, 0x89, 0x76, 0x0}, //8
  {0x0, 0x0, 0x8E, 0x91, 0x91, 0x91, 0x7E, 0x0}, //9
  {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, // blank space
  {0x0, 0x0, 0x0, 0x0, 0x90, 0x0, 0x0, 0x0}, //:
  {0x0, 0x0, 0x0, 0x10, 0x10, 0x10, 0x10, 0x0}, // -
  {0x0, 0x0, 0x0, 0x0, 0x80, 0x0, 0x0, 0x0}, // .
  {0x0, 0x0, 0xFC, 0x9, 0x11, 0x21, 0xFC, 0x0}, //Ñ
  {0x0, 0x0, 0xFE, 0x11, 0x11, 0x11, 0xFE, 0x0}, //A
  {0x0, 0x0, 0xFF, 0x89, 0x89, 0x89, 0x76, 0x0}, //B
  {0x0, 0x0, 0x7E, 0x81, 0x81, 0x81, 0x42, 0x0}, //C
  {0x0, 0x0, 0xFF, 0x81, 0x81, 0x81, 0x7E, 0x0}, //D
  {0x0, 0x0, 0xFF, 0x89, 0x89, 0x89, 0x81, 0x0}, //E
  {0x0, 0x0, 0xFF, 0x9, 0x9, 0x9, 0x1, 0x0}, //F
  {0x0, 0x0, 0x7E, 0x81, 0x81, 0x91, 0x72, 0x0}, //G
  {0x0, 0x0, 0xFF, 0x8, 0x8, 0x8, 0xFF, 0x0}, //H
  {0x0, 0x0, 0x0, 0x81, 0xFF, 0x81, 0x0, 0x0}, //I
  {0x0, 0x0, 0x60, 0x80, 0x80, 0x80, 0x7F, 0x0}, //J
  {0x0, 0x0, 0xFF, 0x18, 0x24, 0x42, 0x81, 0x0}, //K
  {0x0, 0x0, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x0}, //L
  {0x0, 0x0, 0xFF, 0x2, 0x4, 0x2, 0xFF, 0x0}, //M
  {0x0, 0x0, 0xFF, 0x2, 0x4, 0x8, 0xFF, 0x0}, //N
  {0x0, 0x0, 0x7E, 0x81, 0x81, 0x81, 0x7E, 0x0}, //O
  {0x0, 0x0, 0xFF, 0x11, 0x11, 0x11, 0xE, 0x0}, //P
  {0x0, 0x0, 0x7E, 0x81, 0x81, 0xA1, 0x7E, 0x80}, //Q
  {0x0, 0x0, 0xFF, 0x11, 0x31, 0x51, 0x8E, 0x0}, //R
  {0x0, 0x0, 0x46, 0x89, 0x89, 0x89, 0x72, 0x0}, //S
  {0x0, 0x0, 0x1, 0x1, 0xFF, 0x1, 0x1, 0x0}, //T
  {0x0, 0x0, 0x7F, 0x80, 0x80, 0x80, 0x7F, 0x0}, //U
  {0x0, 0x0, 0x3F, 0x40, 0x80, 0x40, 0x3F, 0x0}, //V
  {0x0, 0x0, 0x7F, 0x80, 0x60, 0x80, 0x7F, 0x0}, //W
  {0x0, 0x0, 0xE3, 0x14, 0x8, 0x14, 0xE3, 0x0}, //X
  {0x0, 0x0, 0x3, 0x4, 0xF8, 0x4, 0x3, 0x0}, //Y
  {0x0, 0x0, 0xE1, 0x91, 0x89, 0x85, 0x83, 0x0} //Z
};

String HandleEeprom (int StartAddress, String action, String value = "")
{
  int ArrPos;
  int SizeOfArr = (sizeof(EepromAdress) / 4 ) - 1 ;
  String Eeprom_Content;

  for (int i = 0; i < (SizeOfArr + 1); i++)
  {
    if (EepromAdress[i] == StartAddress)
    {
      ArrPos = i;
      break;
    }
  }

  if (action == "read")
  {
    for (int i = StartAddress; i < (StartAddress + EepromLength[ArrPos]); ++i)
    {
      if (EEPROM.read(i) != 0) {
        Eeprom_Content += char(EEPROM.read(i));
      }
    }
    return Eeprom_Content;
  }

  if (action == "write")
  {
    //Clear EEPROM first
    for (int i = StartAddress; i < (StartAddress + EepromLength[ArrPos]); ++i) {
      EEPROM.write(i, 0);
    }
    int bytesCnt = StartAddress;
    for (int i = 0; i < value.length(); ++i)
    {
      EEPROM.write(bytesCnt, value[i]);
      ++bytesCnt;
    }

    // Commit changes to EEPROM
    EEPROM.commit();

    // Calculate new MD5 and write to EEPROM
    if (StartAddress != eeprommd5_Address)
    {
      EepromMD5();
    }
  }
}

int CheckEeprom()
{
  // Calculate MD5 of ALL eeprom valuesand compare with MD5 written in eeprom
  MD5Builder md5;
  md5.begin();

  int SizeOfArr = (sizeof(EepromAdress) / 4 ) - 1 ;
  String Eeprom_Content;
  String CurrentMD5 = HandleEeprom(eeprommd5_Address, "read");

  for (int i = 0; i < (SizeOfArr + 1); i++)
  {
    if (EepromAdress[i] != eeprommd5_Address)
    {
      Eeprom_Content += HandleEeprom(EepromAdress[i], "read");
    }
  }
  md5.add(Eeprom_Content);
  md5.calculate();
  if (md5.toString() != CurrentMD5)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

void EepromMD5()
{
  MD5Builder md5;
  md5.begin();

  int SizeOfArr = (sizeof(EepromAdress) / 4 ) - 1 ;
  String Eeprom_Content;

  for (int i = 0; i < (SizeOfArr + 1); i++)
  {
    if (EepromAdress[i] != eeprommd5_Address)
    {
      Eeprom_Content += HandleEeprom(EepromAdress[i], "read");
    }
  }
  md5.add(Eeprom_Content);
  md5.calculate();
  String value = md5.toString();
  int ArrPos;

  // find eeprommd5_Address in array
  for (int i = 0; i < (SizeOfArr + 1); i++)
  {
    if (EepromAdress[i] == eeprommd5_Address)
    {
      ArrPos = i;
      break;
    }
  }

    //Clear EEPROM first
    for (int i = eeprommd5_Address; i < (eeprommd5_Address + EepromLength[ArrPos]); ++i) {
      EEPROM.write(i, 0);
    }
    int bytesCnt = eeprommd5_Address;
    for (int i = 0; i < value.length(); ++i)
    {
      EEPROM.write(bytesCnt, value[i]);
      ++bytesCnt;
    }
    // Commit changes to EEPROM
    EEPROM.commit();


}

void setup()
{
  Serial.begin(115200);
  delay(500);
  int SizeOfArr = (sizeof(EepromAdress) / 4 ) - 1 ;
  int eeprom_alloc = EepromAdress[SizeOfArr] + EepromLength[SizeOfArr];
  EEPROM.begin(eeprom_alloc);
  
  // Check if EEPROM is valid
  int EepromStatus = CheckEeprom();
  if (EepromStatus != 1)
  {
    ErrorList[ErrorEeprom] = 1;
  }

  String eepromVersion = HandleEeprom(version_Address, "read");
  if (eepromVersion != ESPimaticVersion)
  {
    ErrorList[ErrorUpgrade] = 1;
    HandleEeprom(version_Address, "write", ESPimaticVersion);
  }

  // Check if SPIFFS is OK
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS failed, needs formatting");
    handleFormat();
    delay(500);
    ESP.restart();
  }
  else
  {
    FSInfo fs_info;
    if (!SPIFFS.info(fs_info))
    {
      Serial.println("fs_info failed");
    }
    else
    {
      FSTotal = fs_info.totalBytes;
      FSUsed = fs_info.usedBytes;
    }
  }

  BSlocal = HandleEeprom(bslocal_Address, "read");
  
  String IREnabled = HandleEeprom(enableir_Address, "read");
  if (IREnabled == "1")
  {
    irsend.begin();
  }

  DeviceName = HandleEeprom(devicename_Address, "read");

  MatrixEnabled = HandleEeprom(enablematrix_Address, "read");
  if (MatrixEnabled == "1")
  {
    String MatrixPin = HandleEeprom(matrixpin_Address, "read");
    ShowOnMatrix = HandleEeprom(showonmatrix_Address, "read");
    MatrixIntensity = HandleEeprom(matrixintensity_Address, "read");

    
    LEDpin = MatrixPin.toInt();
    lc = LedControl(LEDpin, 2);
    /*
    The MAX72XX is in power-saving mode on startup,
    we have to do a wakeup call
    */
    lc.shutdown(0, false);
    lc.shutdown(1, false);
    /* Set the brightness to a medium values */
    lc.setIntensity(0, MatrixIntensity.toInt());
    lc.setIntensity(1, MatrixIntensity.toInt());
    /* and clear the display */
    lc.clearDisplay(0);
    lc.clearDisplay(1);
  }


  String ssidStored = HandleEeprom(ssid_Address, "read");
  String passStored = HandleEeprom(password_Address, "read");
  if (ssidStored == "" || passStored == "")
  {
    Serial.println("No wifi configuration found, starting in AP mode");
    Serial.println("SSID: ");
    Serial.println(APssid);
    Serial.println("password: ");
    Serial.println(APpassword);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(APssid, APpassword);
    WMode = "AP";
    Serial.print("Connected to ");
    Serial.println(APssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    if (MatrixEnabled == "1")
    {
      // Display 'AP' on LED's
      CharOnLED(30, 0);
      CharOnLED(15, 1);
    }
  }
  else
  {
    int i = 0;
    int row = 0;
    int col = 0;
    Serial.println("Connecting to :");
    Serial.println(ssidStored);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssidStored.c_str(), passStored.c_str());
    while (WiFi.status() != WL_CONNECTED && i < 31 && MatrixEnabled == "1")
    {
      delay(1000);
      Serial.print(".");
      lc.setLed(0, row, col, true);
      lc.setLed(1, row, col, true);
      ++i;

      ++row;
      // if (col == 8) { ++col; }
      if (row == 8) {
        row = 0;
        ++col;
      }
    }
    while (WiFi.status() != WL_CONNECTED && i < 31 && MatrixEnabled != "0")
    {
      delay(1000);
      Serial.print(".");
      ++i;
    }


    if (WiFi.status() != WL_CONNECTED && i >= 30)
    {
      WiFi.disconnect();
      delay(1000);
      Serial.println("");
      Serial.println("Couldn't connect to network :( ");
      Serial.println("Setting up access point");
      Serial.println("SSID: ");
      Serial.println(APssid);
      Serial.println("password: ");
      Serial.println(APpassword);
      WiFi.mode(WIFI_AP);
      WiFi.softAP(APssid, APpassword);
      WMode = "AP";
      Serial.print("Connected to ");
      Serial.println(APssid);
      IPAddress myIP = WiFi.softAPIP();
      Serial.print("IP address: ");
      Serial.println(myIP);
      if (MatrixEnabled == "1")
      {
        // Display 'AP' on LED's
        CharOnLED(30, 0);
        CharOnLED(15, 1);
      }
    }
    else
    {
      Serial.println("");
      Serial.print("Connected to ");
      Serial.println(ssidStored);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      if (MatrixEnabled == "1")
      {
        // Display 'OK' on LED's
        CharOnLED(25, 0);
        CharOnLED(29, 1);
      }
    }
  }

  String RelayEnabled = HandleEeprom(enablerelay_Address, "read");
  if (RelayEnabled == "1")
  {
    String relay1pin = HandleEeprom(relay1pin_Address, "read");
    String relay1type = HandleEeprom(relay1type_Address, "read");
    String relay2pin = HandleEeprom(relay2pin_Address, "read");
    String relay2type = HandleEeprom(relay2type_Address, "read");
    String relay3pin = HandleEeprom(relay3pin_Address, "read");
    String relay3type = HandleEeprom(relay3type_Address, "read");
    String relay4pin = HandleEeprom(relay4pin_Address, "read");
    String relay4type = HandleEeprom(relay4type_Address, "read");

    pinMode(relay1pin.toInt(), OUTPUT);
    if (relay1type == "0")
    {
      digitalWrite(relay1pin.toInt(), HIGH);
    }
    else
    {
      digitalWrite(relay1pin.toInt(), LOW);
    }
    
    pinMode(relay2pin.toInt(), OUTPUT);
    if (relay2type == "0")
    {
      digitalWrite(relay2pin.toInt(), HIGH);
    }
    else
    {
      digitalWrite(relay2pin.toInt(), LOW);
    }
    
    pinMode(relay3pin.toInt(), OUTPUT);
    if (relay3type == "0")
    {
      digitalWrite(relay3pin.toInt(), HIGH);
    }
    else
    {
      digitalWrite(relay3pin.toInt(), LOW);
    }

    pinMode(relay4pin.toInt(), OUTPUT);
    if (relay4type == "0")
    {
      digitalWrite(relay4pin.toInt(), HIGH);
    }
    else
    {
      digitalWrite(relay4pin.toInt(), LOW);
    }

  }


  // If no root page && AP mode, set root to simple upload page
  if (!SPIFFS.exists("/root.html") && WMode != "AP")
  {
      server.on("/", handle_fupload_html);
   }

  // If connected to AP mode, set root to simple wifi settings page
  if (WMode == "AP")
  {
      server.on("/", handle_wifim_html);
   }

  // Format Flash ROM - dangerous! Remove if you dont't want this option!
  server.on ( "/format", handleFormat );
  

  server.on("/ping", handle_ping);
  server.on("/loginm", handle_loginm_html);
  server.on("/login_ajax", handle_login_ajax);
  server.on("/root_ajax", handle_root_ajax);
  server.on("/ds18b20_ajax", handle_ds18b20_ajax);
  server.on("/wifi_ajax", handle_wifi_ajax);
  server.on("/pimatic_ajax", handle_pimatic_ajax);
  server.on("/ledmatrix_ajax", handle_ledmatrix_ajax);
  server.on("/irled_ajax", handle_irled_ajax);
  server.on("/relay_ajax", handle_relay_ajax);
  server.on("/dht_ajax", handle_dht_ajax);
  server.on("/esp_ajax", handle_esp_ajax);
  server.on("/adc_ajax", handle_adc_ajax);
  //server.on("/led_ajax", handle_led_ajax);
  server.on("/api", handle_api);
  server.on("/updatefwm", handle_updatefwm_html);
  server.on("/fupload", handle_fupload_html);
  server.on("/filemanager_ajax", handle_filemanager_ajax);
  server.on("/delete", handleFileDelete);

  // Upload firmware:
  server.on("/updatefw2", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();        
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
      fileName = upload.filename;
      Serial.setDebugOutput(true);
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
          Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true)) //true to set the size to the current progress
        {
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        }
        else
        {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);

    }
      yield();
  });

  // upload file to SPIFFS
  server.on("/fupload2", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");    
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
      fileName = upload.filename;
      Serial.setDebugOutput(true);
        //fileName = upload.filename;
        Serial.println("Upload Name: " + fileName);
        String path;
         if (fileName.indexOf(".css") >= 0)
         {
            path = "/css/" + fileName;
         }
         else if (fileName.indexOf(".js") >= 0)
         {
            path = "/js/" + fileName;
         }
         else if (fileName.indexOf(".otf") >= 0 || fileName.indexOf(".eot") >= 0 || fileName.indexOf(".svg") >= 0 || fileName.indexOf(".ttf") >= 0 || fileName.indexOf(".woff") >= 0 || fileName.indexOf(".woff2") >= 0)
         {
            path = "/fonts/" + fileName;
         }
         else 
         {
          path = "/" + fileName;
        }
        UploadFile = SPIFFS.open(path, "w");
        // already existing file will be overwritten!
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (UploadFile)
          UploadFile.write(upload.buf, upload.currentSize);
        Serial.println(fileName + " size: " + upload.currentSize);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        Serial.print("Upload Size: ");
        Serial.println(upload.totalSize);  // need 2 commands to work!
        if (UploadFile)
          UploadFile.close();
    }
      yield();
  });

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });


  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );
  server.begin();
  Serial.println("HTTP server started");

  EnableWebAuth = HandleEeprom(enablewebauth_Address, "read");

  DHTEnabled = HandleEeprom(enabledht_Address, "read");
  if (DHTEnabled == "1")
  {
    String dhtinterval = HandleEeprom(dhtinterval_Address, "read");
    String dhtpin = HandleEeprom(dhtpin_Address, "read");
    String dhttype = HandleEeprom(dhttype_Address, "read");
    dht_sendInterval = (dhtinterval.toInt() * 60000);
    uint8_t dhtpinU = atoi (dhtpin.c_str());
    if (dhttype == "1")
    {
      dht = DHT(dhtpin.toInt(), DHT11, 15);
      dht.begin();
    }
    if (dhttype == "2")
    {
      dht = DHT(dhtpin.toInt(), DHT22, 15);
      dht.begin();
    }    
    get_dht();
  }

  DS18B20Enabled = HandleEeprom(enableds18b20_Address, "read");
  if (DS18B20Enabled == "1")
  {
    String busStored = HandleEeprom(ds18b20pin_Address, "read");
    String resoStored = HandleEeprom(ds18b20resolution_Address, "read");

    ONE_WIRE_BUS = busStored.toInt();
    oneWire = OneWire(ONE_WIRE_BUS);

    sensors.begin();
    sensors.getAddress(insideThermometer, 0);

    int NumSens = sensors.getDeviceCount();
    if (NumSens == 0)
    {
      ErrorList[ErrorDs18b20] = 1;
    }
    else
    {
      int reso = sensors.getResolution(insideThermometer);
      if (reso != resoStored.toInt())
      {
        sensors.setResolution(insideThermometer, resoStored.toInt());
      }
    }
    String ds18b20_interval = HandleEeprom(ds18b20interval_Address, "read");
    ds18b20_sendInterval = (ds18b20_interval.toInt() * 60000);
    get_ds18b20;
  }

    ADCEnabled = HandleEeprom(enableadc_Address, "read");
}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path)
{
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "root.html";
  if (path == "/js/insert.js" && BSlocal != "1")
  {
    path = "/js/insert-web.js";
  }
  if (path == "/js/insert.js" && BSlocal == "1")
  {
    path = "/js/insert-local.js";
  }

  
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
  {
    // ingelogd?
    String header;
    if (!is_authenticated(1) && path != "/login.html" && EnableWebAuth == "1")
    {
      String header = "HTTP/1.1 301 OK\r\nLocation: /login.html\r\nCache-Control: no-cache\r\n\r\n";
      server.sendContent(header); 
    }
    else
    {
      // je komt hier als je ingelogd bent
      if (SPIFFS.exists(pathWithGz))
        path += ".gz";      
      File file = SPIFFS.open(path, "r");
      if ( (path.startsWith("/css/") || path.startsWith("/js/") || path.startsWith("/fonts/")) &&  !path.startsWith("/js/insert"))
      {
        server.sendHeader("Cache-Control"," max-age=31104000"); 
      }
      else
      {
        server.sendHeader("Connection", "close");
      }
      size_t sent = server.streamFile(file, contentType);
      size_t contentLength = file.size();
      file.close();
      return true;
    }
  }
  else
  {
    //Serial.println(path);
  }
  return false;
}

void handle_api()
{
  // Get vars for all commands
  String action = server.arg("action");
  String value = server.arg("value");
  String api = server.arg("api");
  String EspimaticApi = HandleEeprom(espimaticapikey_Address, "read");

  if (api != EspimaticApi && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
    delay(500);
  }
  else
  {

  if (action == "ir")
  {
    server.send ( 200, "text/html", "OK");
    unsigned int ArrayKey[512];
    char *tmp;
    int i = 0;
    tmp = strtok(&value[0], ",");
    while (tmp)
    {
      ArrayKey[i++] = atoi(tmp);
      tmp = strtok(NULL, ",");
    }

    int SizeOfArr = sizeof(ArrayKey);
    irsend.sendRaw(ArrayKey, 227, 38);
  }

  if (action == "reboot" && value == "true")
  {
    server.send ( 200, "text/html", "OK");
    delay(500);
    ESP.restart();
  }


  if (action == "matrix_brightness" && value != "status")
  {
    lc.setIntensity(0, value.toInt());
    lc.setIntensity(1, value.toInt());
    MatrixIntensity = value;
    server.send ( 200, "text/html", "OK");
  }

  if (action == "matrix_brightness" && value == "status")
  {
    server.send ( 200, "text/html", MatrixIntensity);
  }

  if (action == "matrix_display" && value == "on")
  {
    server.send ( 200, "text/html", "OK");
    lc.shutdown(0, false);
    lc.shutdown(1, false);
  }

  if (action == "matrix_display" && value == "off")
  {
    server.send ( 200, "text/html", "OK");
    lc.shutdown(0, true);
    lc.shutdown(1, true);
  }
  
  if (action == "relay1")
  {
    String relay1_pin = HandleEeprom(relay1pin_Address, "read");
    String relay1type = HandleEeprom(relay1type_Address, "read");
    if (value == "on")
    {
      if (relay1type == "0") { digitalWrite(relay1_pin.toInt(), LOW); }
      if (relay1type == "1") { digitalWrite(relay1_pin.toInt(), HIGH); }
      server.send ( 200, "text/html", "OK");
    }
    if (value == "off")
    {
      if (relay1type == "0") { digitalWrite(relay1_pin.toInt(), HIGH); }
      if (relay1type == "1") { digitalWrite(relay1_pin.toInt(), LOW); }
      server.send ( 200, "text/html", "OK");
    }
    if (value == "status")
    {
      int relay1_status = digitalRead(relay1_pin.toInt());
      if (relay1_status == 0 && relay1type == "0")
      {
        server.send ( 200, "text/html", "on");
      }
      if (relay1_status == 0 && relay1type == "1")
      {
        server.send ( 200, "text/html", "off");
      }
      if (relay1_status == 1 && relay1type == "0")
      {
        server.send ( 200, "text/html", "off");
      }
      if (relay1_status == 1 && relay1type == "1")
      {
        server.send ( 200, "text/html", "on");
      }
    }
  }

  if (action == "relay2")
  {
    String relay2_pin = HandleEeprom(relay2pin_Address, "read");
    String relay2type = HandleEeprom(relay2type_Address, "read");
    if (value == "on")
    {
      if (relay2type == "0") { digitalWrite(relay2_pin.toInt(), LOW); }
      if (relay2type == "1") { digitalWrite(relay2_pin.toInt(), HIGH); }
      server.send ( 200, "text/html", "OK");
    }
    if (value == "off")
    {
      if (relay2type == "0") { digitalWrite(relay2_pin.toInt(), HIGH); }
      if (relay2type == "1") { digitalWrite(relay2_pin.toInt(), LOW); }
      server.send ( 200, "text/html", "OK");
    }
    if (value == "status")
    {
      int relay2_status = digitalRead(relay2_pin.toInt());
      if (relay2_status == 0 && relay2type == "0")
      {
        server.send ( 200, "text/html", "on");
      }
      if (relay2_status == 0 && relay2type == "1")
      {
        server.send ( 200, "text/html", "off");
      }
      if (relay2_status == 1 && relay2type == "0")
      {
        server.send ( 200, "text/html", "off");
      }
      if (relay2_status == 1 && relay2type == "1")
      {
        server.send ( 200, "text/html", "on");
      }
    }
  }

  if (action == "relay3")
  {
    String relay3_pin = HandleEeprom(relay3pin_Address, "read");
    String relay3type = HandleEeprom(relay3type_Address, "read");
    if (value == "on")
    {
      if (relay3type == "0") { digitalWrite(relay3_pin.toInt(), LOW); }
      if (relay3type == "1") { digitalWrite(relay3_pin.toInt(), HIGH); }
      server.send ( 200, "text/html", "OK");
    }
    if (value == "off")
    {
      if (relay3type == "0") { digitalWrite(relay3_pin.toInt(), HIGH); }
      if (relay3type == "1") { digitalWrite(relay3_pin.toInt(), LOW); }
      server.send ( 200, "text/html", "OK");
    }
    if (value == "status")
    {
      int relay3_status = digitalRead(relay3_pin.toInt());
      if (relay3_status == 0 && relay3type == "0")
      {
        server.send ( 200, "text/html", "on");
      }
      if (relay3_status == 0 && relay3type == "1")
      {
        server.send ( 200, "text/html", "off");
      }
      if (relay3_status == 1 && relay3type == "0")
      {
        server.send ( 200, "text/html", "off");
      }
      if (relay3_status == 1 && relay3type == "1")
      {
        server.send ( 200, "text/html", "on");
      }
    }
  }

    if (action == "relay4")
  {
    String relay4_pin = HandleEeprom(relay4pin_Address, "read");
    String relay4type = HandleEeprom(relay4type_Address, "read");
    if (value == "on")
    {
      if (relay4type == "0") { digitalWrite(relay4_pin.toInt(), LOW); }
      if (relay4type == "1") { digitalWrite(relay4_pin.toInt(), HIGH); }
      server.send ( 200, "text/html", "OK");
    }
    if (value == "off")
    {
      if (relay4type == "0") { digitalWrite(relay4_pin.toInt(), HIGH); }
      if (relay4type == "1") { digitalWrite(relay4_pin.toInt(), LOW); }
      server.send ( 200, "text/html", "OK");
    }
    if (value == "status")
    {
      int relay4_status = digitalRead(relay4_pin.toInt());
      if (relay4_status == 0 && relay4type == "0")
      {
        server.send ( 200, "text/html", "on");
      }
      if (relay4_status == 0 && relay4type == "1")
      {
        server.send ( 200, "text/html", "off");
      }
      if (relay4_status == 1 && relay4type == "0")
      {
        server.send ( 200, "text/html", "off");
      }
      if (relay4_status == 1 && relay4type == "1")
      {
        server.send ( 200, "text/html", "on");
      }
    }
  }
  
  if (action == "clearerror" && value == "wifi")
  {
    ErrorList[ErrorWifi] = 0;
    server.send ( 200, "text/html", "OK");
  }
  if (action == "clearerror" && value == "ds18b20")
  {
    ErrorList[ErrorDs18b20] = 0;
    server.send ( 200, "text/html", "OK");
  }
  if (action == "clearerror" && value == "eeprom")
  {
    ErrorList[ErrorEeprom] = 0;
    server.send ( 200, "text/html", "OK");
  }
  if (action == "clearerror" && value == "upgrade")
  {
    ErrorList[ErrorUpgrade] = 0;
    server.send ( 200, "text/html", "OK");
  }

  if (action == "reset" && value == "true")
  {
    int SizeOfArr = (sizeof(EepromAdress) / 4 ) - 1 ;
    int StartAddress = EepromAdress[0];
    int LastAddress = EepromAdress[SizeOfArr];
    int LastLength = EepromLength[SizeOfArr];
    int LastByte = LastAddress + LastLength;
  
    for (int i = StartAddress; i < (LastByte + 1); ++i) {
      EEPROM.write(i, 0);
    }
    
    // Commit changes to EEPROM
    EEPROM.commit();
    server.send ( 200, "text/html", "OK");
    delay(500);
    ESP.restart();
  }
  }
}

void handle_ping()
{
  server.send ( 200, "text/html", "pong");
}


void handle_updatefwm_html()
{
  if (!is_authenticated(1) && EnableWebAuth == "1")
  {
    String header = "HTTP/1.1 301 OK\r\nLocation: /login.html\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header); 
  }
  else
  {
    server.send ( 200, "text/html", "<form method='POST' action='/updatefw2' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form><br<b>For firmware only!!</b>");
  }
}

void handle_wifim_html()
{
  server.send ( 200, "text/html", "<form method='POST' action='/wifi_ajax'><input type='hidden' name='form' value='wifi'><input type='text' name='ssid'><input type='password' name='password'><input type='submit' value='Submit'></form><br<b>Enter WiFi credentials</b>");
}


void handle_fupload_html()
{
  String HTML = "<br>Files on flash:<br>";
    Dir dir = SPIFFS.openDir("/");
    while (dir.next())
    {
      fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      HTML += fileName.c_str();
      HTML += " ";
      HTML += formatBytes(fileSize).c_str();
      HTML += " , ";
      HTML += fileSize;
      HTML += "<br>";
      //Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
  
  server.send ( 200, "text/html", "<form method='POST' action='/fupload2' enctype='multipart/form-data'><input type='file' name='update' multiple><input type='submit' value='Update'></form><br<b>For webfiles only!!</b>Multiple files possible<br>" + HTML);
}



void handle_update_upload()
{
  if (server.uri() != "/update2") return;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.setDebugOutput(true);
    Serial.printf("Update: %s\n", upload.filename.c_str());
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (!Update.begin(maxSketchSpace)) { //start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { //true to set the size to the current progress
      Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
    Serial.setDebugOutput(false);
  }
  yield();
}
void handle_update_html2()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  ESP.restart();
}


void handle_ledmatrix_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
	  String form = server.arg("form");
	  if (form != "matrix")
	  {
		String matrix_enable = HandleEeprom(enablematrix_Address, "read");
		String matrix_pin = HandleEeprom(matrixpin_Address, "read");
		String matrix_show = HandleEeprom(showonmatrix_Address, "read");
		String matrix_intensity = HandleEeprom(matrixintensity_Address, "read");
		String matrixshow_listbox = ListBox(0, 3, matrix_show.toInt(), "matrix_show");
		String matrixintensity_listbox = ListBox(0, 15, matrix_intensity.toInt(), "matrix_intensity");
		String matrix_listbox = "";
		if (matrix_pin != "")
		{
		  matrix_listbox = HWListBox(0, 16, matrix_pin.toInt(), "matrix_pin", "ledmatrix");
		}
		else
		{
		  matrix_listbox = HWListBox(0, 16, -1, "matrix_pin", "ledmatrix");
		}

		// Glue everything together and send to client
		server.send(200, "text/html", matrix_enable + sep + matrix_listbox + sep + matrixshow_listbox + sep + ErrorList[ErrorWifi] + sep + ErrorList[ErrorEeprom] + sep + ErrorList[ErrorDs18b20] + sep + ErrorList[ErrorUpgrade] + sep + matrixintensity_listbox);
	  }
	  if (form == "matrix")
	  {
		String matrix_boolArg = server.arg("matrix_bool");
		String matrix_pinArg = server.arg("matrix_pin");
		String matrix_showArg = server.arg("matrix_show");
		String matrix_intensityArg = server.arg("matrix_intensity");

		if (matrix_boolArg == "on")
		{
		  matrix_boolArg = "1";
		}
		else
		{
		  matrix_boolArg = "0";
		  lc.shutdown(0, true);
		  lc.shutdown(1, true);
		}

		HandleEeprom(enablematrix_Address, "write", matrix_boolArg);
		HandleEeprom(matrixpin_Address, "write", matrix_pinArg);
		HandleEeprom(showonmatrix_Address, "write", matrix_showArg);
		HandleEeprom(matrixintensity_Address, "write", matrix_intensityArg);

		server.send ( 200, "text/html", "OK");
		delay(500);
		ESP.restart();
	  }
	}
}

void handle_irled_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
	  String form = server.arg("form");
	  if (form != "irled")
	  {
		String irled_enable = HandleEeprom(enableir_Address, "read");
		String irled_pin = HandleEeprom(irpin_Address, "read");
		String irled_listbox = "";
		if (irled_pin != "")
		{
		  irled_listbox = HWListBox(0, 16, irled_pin.toInt(), "irled_pin", "irled");
		}
		else
		{
		  irled_listbox = HWListBox(0, 16, -1, "irled_pin", "irled");
		}

		// Glue everything together and send to client
		server.send(200, "text/html", irled_enable + sep + irled_listbox + sep + ErrorList[ErrorWifi] + sep + ErrorList[ErrorEeprom] + sep + ErrorList[ErrorDs18b20] + sep + ErrorList[ErrorUpgrade]);
	  }
	  if (form == "irled")
	  {
		String irled_boolArg = server.arg("irled_bool");
		String irled_pinArg = server.arg("irled_pin");

		if (irled_boolArg == "on")
		{
		  irled_boolArg = "1";
		}
		else
		{
		  irled_boolArg = "0";
		}

		HandleEeprom(enableir_Address, "write", irled_boolArg);
		HandleEeprom(irpin_Address, "write", irled_pinArg);

		server.send ( 200, "text/html", "OK");
		delay(500);
		ESP.restart();
	  }
	}
}

void handle_relay_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
	  String form = server.arg("form");
	  if (form != "relay")
	  {
		String relay_enable = HandleEeprom(enablerelay_Address, "read");
		String relay1_pin = HandleEeprom(relay1pin_Address, "read");
		String relay2_pin = HandleEeprom(relay2pin_Address, "read");
		String relay3_pin = HandleEeprom(relay3pin_Address, "read");
		String relay4_pin = HandleEeprom(relay4pin_Address, "read");
		String relay1type = HandleEeprom(relay1type_Address, "read");
		String relay2type = HandleEeprom(relay2type_Address, "read");
		String relay3type = HandleEeprom(relay3type_Address, "read");
		String relay4type = HandleEeprom(relay4type_Address, "read");

		String relay1type_listbox = ListBox(0, 1, relay1type.toInt(), "relay1_type");
		String relay2type_listbox = ListBox(0, 1, relay2type.toInt(), "relay2_type");
		String relay3type_listbox = ListBox(0, 1, relay3type.toInt(), "relay3_type");
		String relay4type_listbox = ListBox(0, 1, relay4type.toInt(), "relay4_type");

		String relay1_listbox = "";
		if (relay1_pin != "")
		{
			relay1_listbox = HWListBox(0, 16, relay1_pin.toInt(), "relay1_pin", "relay");
		}
		else
		{
		  relay1_listbox = HWListBox(0, 16, -1, "relay1_pin", "relay");
		}

		String relay2_listbox = "";
		if (relay2_pin != "")
		{
			relay2_listbox = HWListBox(0, 16, relay2_pin.toInt(), "relay2_pin", "relay");
		}
		else
		{
		  relay2_listbox = HWListBox(0, 16, -1, "relay2_pin", "relay");
		}

		String relay3_listbox = "";
		if (relay3_pin != "")
		{
			relay3_listbox = HWListBox(0, 16, relay3_pin.toInt(), "relay3_pin", "relay");
		}
		else
		{
		  relay3_listbox = HWListBox(0, 16, -1, "relay3_pin", "relay");
		}

		String relay4_listbox = "";
		if (relay4_pin != "")
		{
			relay4_listbox = HWListBox(0, 16, relay4_pin.toInt(), "relay4_pin", "relay");
		}
		else
		{
		  relay4_listbox = HWListBox(0, 16, -1, "relay4_pin", "relay");
		}


		//String relay1_listbox = HWListBox(0, 16, relay1_pin.toInt(), "relay1_pin", "relay");
		//String relay2_listbox = HWListBox(0, 16, relay2_pin.toInt(), "relay2_pin", "relay");
		//String relay3_listbox = HWListBox(0, 16, relay3_pin.toInt(), "relay3_pin", "relay");
		//String relay4_listbox = HWListBox(0, 16, relay4_pin.toInt(), "relay4_pin", "relay");
		

		// Glue everything together and send to client
		server.send(200, "text/html", relay_enable + sep + relay1_listbox + sep + relay2_listbox + sep + relay3_listbox + sep + relay4_listbox + sep + ErrorList[ErrorWifi] + sep + ErrorList[ErrorEeprom] + sep + ErrorList[ErrorDs18b20] + sep + ErrorList[ErrorUpgrade] + sep + relay1type_listbox + sep + relay2type_listbox + sep + relay3type_listbox + sep + relay4type_listbox);
	  }
	  if (form == "relay")
	  {
		String relay_boolArg = server.arg("relay_bool");
		String relay1_pinArg = server.arg("relay1_pin");
		String relay2_pinArg = server.arg("relay2_pin");
		String relay3_pinArg = server.arg("relay3_pin");
		String relay4_pinArg = server.arg("relay4_pin");
		String relay1_typeArg = server.arg("relay1_type");
		String relay2_typeArg = server.arg("relay2_type");
		String relay3_typeArg = server.arg("relay3_type");
		String relay4_typeArg = server.arg("relay4_type");

		if (relay_boolArg == "on")
		{
		  relay_boolArg = "1";
		}
		else
		{
		  relay_boolArg = "0";
		  // if relay is disabled, turn off *current* relay pins
		  String relay1_pin = HandleEeprom(relay1pin_Address, "read");
		  String relay2_pin = HandleEeprom(relay2pin_Address, "read");
		  String relay3_pin = HandleEeprom(relay3pin_Address, "read");
		  String relay4_pin = HandleEeprom(relay3pin_Address, "read");
		  //digitalWrite(relay1_pin.toInt(), LOW);
		  //digitalWrite(relay2_pin.toInt(), LOW);
		  //digitalWrite(relay3_pin.toInt(), LOW);
		  //digitalWrite(relay4_pin.toInt(), LOW);
		}

		HandleEeprom(enablerelay_Address, "write", relay_boolArg);
		HandleEeprom(relay1pin_Address, "write", relay1_pinArg);
		HandleEeprom(relay2pin_Address, "write", relay2_pinArg);
		HandleEeprom(relay3pin_Address, "write", relay3_pinArg);
		HandleEeprom(relay4pin_Address, "write", relay4_pinArg);
		HandleEeprom(relay1type_Address, "write", relay1_typeArg);
		HandleEeprom(relay2type_Address, "write", relay2_typeArg);
		HandleEeprom(relay3type_Address, "write", relay3_typeArg);
		HandleEeprom(relay4type_Address, "write", relay4_typeArg);

		server.send ( 200, "text/html", "OK");
		delay(500);
		ESP.restart();
	  }
	}
}

void handle_root_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
    String disabled = "<span class='glyphicon glyphicon-ban-circle pull-right'>";
    String enabled =  "<span class='glyphicon glyphicon-ok-circle pull-right'>";
    String relay_on = "<span class='glyphicon glyphicon-resize-small pull-right'>";
    String relay_off = "<span class='glyphicon glyphicon-resize-full pull-right'>";

    // Collect everything for DS18B20
    //String DS18B20Enabled = HandleEeprom(enableds18b20_Address, "read");
    String temperature = disabled;
    if (DS18B20Enabled == "1")
    {
      //temperature = get_ds18b20() + String(" °C");
      temperature = LastDS18B20 + String(" °C");
    }

    // Collect everything for DHT
    String dht_temp = disabled;
    String dht_hum = disabled;
    if (DHTEnabled == "1")
    {
      //temperature = get_ds18b20() + String(" °C");
      dht_temp = LastDHTtemp + String(" °C");
      dht_hum = LastDHThum + String(" &nbsp;%");
    }


    // Collect everything for uptime
    long milliseconds   = (long) (timeElapsed / 1000000) % 1000;
    long seconds    = (long) ((timeElapsed / (1000)) % 60);
    long minutes    = (long) ((timeElapsed / (60000)) % 60);
    long hours      = (long) ((timeElapsed / (3600000)) % 24);
    long days       = (long) ((timeElapsed / (86400000)) % 10);
  
    String Uptime     = days + String (" d ") + hours + String(" h ") + minutes + String(" min ") + seconds + String(" sec");

    // Collect everything for LED Matrix
    String MatrixEnabled = HandleEeprom(enablematrix_Address, "read");
    String matrix  = disabled;
    if (MatrixEnabled == "1")
    {
      matrix = enabled;
    }
    
    // Collect everything for IR LED
    String IREnabled = HandleEeprom(enableir_Address, "read");
    String ir  = disabled;
    if (IREnabled == "1")
    {
      ir = enabled;
    }

    // Collect everything for relay
    String RelayEnabled = HandleEeprom(enablerelay_Address, "read");
    String relay1 = relay_off;
    String relay2 = relay_off;
    String relay3 = relay_off;
    String relay4 = relay_off;
    String relay  = disabled;
    if (RelayEnabled == "1")
    {
      relay = enabled;

      String relay1pin = HandleEeprom(relay1pin_Address, "read");
      String relay2pin = HandleEeprom(relay2pin_Address, "read");
      String relay3pin = HandleEeprom(relay3pin_Address, "read");
      String relay4pin = HandleEeprom(relay4pin_Address, "read");
      String relay1type = HandleEeprom(relay1type_Address, "read");
      String relay2type = HandleEeprom(relay2type_Address, "read");
      String relay3type = HandleEeprom(relay3type_Address, "read");
      String relay4type = HandleEeprom(relay4type_Address, "read");

      int relay1_status = digitalRead(relay1pin.toInt());
      int relay2_status = digitalRead(relay2pin.toInt());
      int relay3_status = digitalRead(relay3pin.toInt());
      int relay4_status = digitalRead(relay4pin.toInt());

        if (relay1_status == 0 && relay1type == "0")
        {
          relay1 = relay_on;
        }
        if (relay1_status == 1 && relay1type == "1")
        {
          relay1 = relay_on;
        }

        if (relay2_status == 0 && relay2type == "0")
        {
          relay2 = relay_on;
        }
        if (relay2_status == 1 && relay2type == "1")
        {
          relay2 = relay_on;
        }

        if (relay3_status == 0 && relay3type == "0")
        {
          relay3 = relay_on;
        }
        if (relay3_status == 1 && relay3type == "1")
        {
          relay3 = relay_on;
        }

        if (relay4_status == 0 && relay4type == "0")
        {
          relay4 = relay_on;
        }
        if (relay4_status == 1 && relay4type == "1")
        {
          relay4 = relay_on;
        }
    }

    // Collect free memory
    int FreeHeap = ESP.getFreeHeap();

    // Glue everything together and send to client
    server.send(200, "text/html", temperature + sep + Uptime + sep + matrix + sep + ir + sep + relay + sep + relay1 + sep + relay2 + sep + relay3 + sep + relay4 + sep + ESPimaticVersion + sep + ErrorList[ErrorWifi] + sep + ErrorList[ErrorEeprom] + sep + ErrorList[ErrorDs18b20] + sep + ErrorList[ErrorUpgrade] + sep + dht_temp + sep + dht_hum + sep + FSTotal + sep + FSUsed + sep + FreeHeap + sep + DeviceName + sep + EnableWebAuth + sep + ADC);
  }
}


void handle_esp_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
	  String form = server.arg("form");
	  if (form != "esp" && form != "gpio" && form != "security"  && form != "bslocal")
	  {
		// Get device name
		DeviceName = HandleEeprom(devicename_Address, "read");

		// Get WebAuth
		EnableWebAuth = HandleEeprom(enablewebauth_Address, "read");
		String WebUser = HandleEeprom(webuser_Address, "read");
		String WebPass = HandleEeprom(webpass_Address, "read");

		// Get API key
		String EspimaticApi = HandleEeprom(espimaticapikey_Address, "read");

		// Get setting to use local Bootstrap files
		BSlocal = HandleEeprom(bslocal_Address, "read");
		
		String AllGpio = HandleEeprom(availablegpio_Address, "read");
		// Length (with one extra character for the null terminator)
		int str_len = AllGpio.length() + 1; 
		// Prepare the character array (the buffer) 
		char char_array[str_len];
		// Copy it over 
		AllGpio.toCharArray(char_array, str_len);

		char gpio0 = AllGpio[0];
		char gpio1 = AllGpio[1];
		char gpio2 = AllGpio[2];
		char gpio3 = AllGpio[3];
		char gpio4 = AllGpio[4];
		char gpio5 = AllGpio[5];
		char gpio6 = AllGpio[6];
		char gpio7 = AllGpio[7];
		char gpio8 = AllGpio[8];
		char gpio9 = AllGpio[9];
		char gpio10 = AllGpio[10];
		char gpio11 = AllGpio[11];
		char gpio12 = AllGpio[12];
		char gpio13 = AllGpio[13];
		char gpio14 = AllGpio[14];
		char gpio15 = AllGpio[15];
		char gpio16 = AllGpio[16];

		// Glue everything together and send to client
		server.send(200, "text/html", gpio0 + sep + gpio1 + sep + gpio2 + sep + gpio3 + sep + gpio4 + sep + gpio5 + sep + gpio6 + sep + gpio7 + sep + gpio8 + sep + gpio9 + sep + gpio10 + sep + gpio11 + sep + gpio12 + sep + gpio13 + sep + gpio14 + sep + gpio15 + sep + gpio16 + sep + DeviceName + sep + EnableWebAuth + sep + WebUser + sep + WebPass  + sep + ErrorList[ErrorWifi] + sep + ErrorList[ErrorEeprom] + sep + ErrorList[ErrorDs18b20] + sep + ErrorList[ErrorUpgrade] + sep + EspimaticApi + sep + BSlocal);
	  }
	  if (form == "devicename")
	  {
		String devicenameArg = server.arg("devicename");

		HandleEeprom(devicename_Address, "write", devicenameArg);
		server.send ( 200, "text/html", "OK");
		delay(500);
		//ESP.restart();
	  }

	  if (form == "bslocal")
	  {
		String bslocalArg = server.arg("bslocal_bool");

		if (bslocalArg == "on")
		{
		  bslocalArg = "1";
		}
		else
		{
		  bslocalArg = "0";
		}


		HandleEeprom(bslocal_Address, "write", bslocalArg);
		BSlocal == bslocalArg;
		server.send ( 200, "text/html", "OK");
		delay(500);
		//ESP.restart();
	  }

	  if (form == "gpio")
	  {
		String gpio0_boolArg = server.arg("gpio0_bool");
		String gpio1_boolArg = server.arg("gpio1_bool");
		String gpio2_boolArg = server.arg("gpio2_bool");
		String gpio3_boolArg = server.arg("gpio3_bool");
		String gpio4_boolArg = server.arg("gpio4_bool");
		String gpio5_boolArg = server.arg("gpio5_bool");
		String gpio6_boolArg = server.arg("gpio6_bool");
		String gpio7_boolArg = server.arg("gpio7_bool");
		String gpio8_boolArg = server.arg("gpio8_bool");
		String gpio9_boolArg = server.arg("gpio9_bool");
		String gpio10_boolArg = server.arg("gpio10_bool");
		String gpio11_boolArg = server.arg("gpio11_bool");
		String gpio12_boolArg = server.arg("gpio12_bool");
		String gpio13_boolArg = server.arg("gpio13_bool");
		String gpio14_boolArg = server.arg("gpio14_bool");
		String gpio15_boolArg = server.arg("gpio15_bool");
		String gpio16_boolArg = server.arg("gpio16_bool");

		if (gpio0_boolArg != "1") { gpio0_boolArg = "0"; }
		if (gpio1_boolArg != "1") { gpio1_boolArg = "0"; }
		if (gpio2_boolArg != "1") { gpio2_boolArg = "0"; }
		if (gpio3_boolArg != "1") { gpio3_boolArg = "0"; }
		if (gpio4_boolArg != "1") { gpio4_boolArg = "0"; }
		if (gpio5_boolArg != "1") { gpio5_boolArg = "0"; }
		if (gpio6_boolArg != "1") { gpio6_boolArg = "0"; }
		if (gpio7_boolArg != "1") { gpio7_boolArg = "0"; }
		if (gpio8_boolArg != "1") { gpio8_boolArg = "0"; }
		if (gpio9_boolArg != "1") { gpio9_boolArg = "0"; }
		if (gpio10_boolArg != "1") { gpio10_boolArg = "0"; }
		if (gpio11_boolArg != "1") { gpio11_boolArg = "0"; }
		if (gpio12_boolArg != "1") { gpio12_boolArg = "0"; }
		if (gpio13_boolArg != "1") { gpio13_boolArg = "0"; }
		if (gpio14_boolArg != "1") { gpio14_boolArg = "0"; }
		if (gpio15_boolArg != "1") { gpio15_boolArg = "0"; }
		if (gpio16_boolArg != "1") { gpio16_boolArg = "0"; }

		String AllGpio = gpio0_boolArg + gpio1_boolArg + gpio2_boolArg + gpio3_boolArg + gpio4_boolArg + gpio5_boolArg + gpio6_boolArg + gpio7_boolArg + gpio8_boolArg + gpio9_boolArg + gpio10_boolArg + gpio11_boolArg + gpio12_boolArg + gpio13_boolArg + gpio14_boolArg + gpio15_boolArg + gpio16_boolArg;
		HandleEeprom(availablegpio_Address, "write", AllGpio);

		server.send ( 200, "text/html", "OK");
	  }

	  if (form == "security")
	  {
		String securityArg = server.arg("security_bool");
		String usernameArg = server.arg("username");
		String passwordArg = server.arg("password");
		String apikeyArg = server.arg("apikey");

		if (securityArg == "on")
		{
		  securityArg = "1";
		}
		else
		{
		  securityArg = "0";
		}

		HandleEeprom(enablewebauth_Address, "write", securityArg);
		HandleEeprom(webuser_Address, "write", usernameArg);
		HandleEeprom(webpass_Address, "write", passwordArg);
		HandleEeprom(espimaticapikey_Address, "write", apikeyArg);
		EnableWebAuth = "1";
		server.send ( 200, "text/html", "OK");
		delay(500);
		//ESP.restart();

	  }
	}
}

/*
void handle_led_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
    String form = server.arg("form");
    if (form != "led")
    {
      String led_enable = HandleEeprom(enableled_Address, "read");
      String led1pin = HandleEeprom(led1pin_Address, "read");
      String led2pin = HandleEeprom(led2pin_Address, "read");
      String led3pin = HandleEeprom(led3pin_Address, "read");

    String led1pin_listbox = "";
    String led2pin_listbox = "";
    String led3pin_listbox = "";
    
    if (led1pin != "")
    {
      led1pin_listbox = HWListBox(0, 16, led1pin.toInt(), "led1_pin", "led");
    }
    else
    {
      led1pin_listbox = HWListBox(0, 16, -1, "led1_pin", "led1");
    }

    if (led2pin != "")
    {
      led2pin_listbox = HWListBox(0, 16, led2pin.toInt(), "led2_pin", "led");
    }
    else
    {
      led2pin_listbox = HWListBox(0, 16, -1, "led2_pin", "led2");
    }

    if (led3pin != "")
    {
      led3pin_listbox = HWListBox(0, 16, led3pin.toInt(), "led3_pin", "led");
    }
    else
    {
      led3pin_listbox = HWListBox(0, 16, -1, "led3_pin", "led3");
    }

      // Glue everything together and send to client
//      server.send(200, "text/html", adc_enable + sep + adc_var + sep + adc_intlistbox + sep + ErrorList[ErrorWifi] + sep + ErrorList[ErrorEeprom] + sep + ErrorList[ErrorDs18b20] + sep + ErrorList[ErrorUpgrade]);
    }
    if (form == "adc")
    {
      String adc_boolArg = server.arg("adc_bool");
      String adc_varArg = server.arg("adc_var");
      String adc_intervalArg = server.arg("adc_interval");
  
      if (adc_boolArg == "on")
      {
        adc_boolArg = "1";
      }
      else
      {
        adc_boolArg = "0";
      }

      HandleEeprom(enableadc_Address, "write", adc_boolArg);
      HandleEeprom(adcvar_Address, "write", adc_varArg);
      HandleEeprom(adcinterval_Address, "write", adc_intervalArg);

      server.send ( 200, "text/html", "OK");
      delay(500);
      //ESP.restart();
    }
  }
}
*/

void handle_adc_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
    String form = server.arg("form");
    if (form != "adc")
    {
      String adc_var = HandleEeprom(adcvar_Address, "read");
      String adc_enable = HandleEeprom(enableadc_Address, "read");
      String adc_interval = HandleEeprom(adcinterval_Address, "read");

      String adc_intlistbox = ListBox(1, 5, adc_interval.toInt(), "adc_interval");

      // Glue everything together and send to client
      server.send(200, "text/html", adc_enable + sep + adc_var + sep + adc_intlistbox + sep + ErrorList[ErrorWifi] + sep + ErrorList[ErrorEeprom] + sep + ErrorList[ErrorDs18b20] + sep + ErrorList[ErrorUpgrade]);
    }
    if (form == "adc")
    {
      String adc_boolArg = server.arg("adc_bool");
      String adc_varArg = server.arg("adc_var");
      String adc_intervalArg = server.arg("adc_interval");
  
      if (adc_boolArg == "on")
      {
        adc_boolArg = "1";
      }
      else
      {
        adc_boolArg = "0";
      }

      HandleEeprom(enableadc_Address, "write", adc_boolArg);
      HandleEeprom(adcvar_Address, "write", adc_varArg);
      HandleEeprom(adcinterval_Address, "write", adc_intervalArg);

      server.send ( 200, "text/html", "OK");
      delay(500);
      //ESP.restart();
    }
  }
}

void handle_ds18b20_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
    String form = server.arg("form");
    if (form != "ds18b20")
    {
      String ds18b20_var = HandleEeprom(ds18b20var_Address, "read");
      String ds18b20_enable = HandleEeprom(enableds18b20_Address, "read");
      String ds18b20_pin = HandleEeprom(ds18b20pin_Address, "read");
      String ds18b20_listbox = "";
      if (ds18b20_pin != "")
      {
          ds18b20_listbox = HWListBox(0, 16, ds18b20_pin.toInt(), "DS18B20_pin", "ds18b20");
      }
      else
      {
        ds18b20_listbox = HWListBox(0, 16, -1, "DS18B20_pin", "ds18b20");
      }
      
      String ds18b20_interval = HandleEeprom(ds18b20interval_Address, "read");
      String ds18b20_resolution = HandleEeprom(ds18b20resolution_Address, "read");
  
      String ds18b20_intlistbox = ListBox(1, 5, ds18b20_interval.toInt(), "DS18B20_interval");
      String ds18b20_resolistbox = ListBox(9, 12, ds18b20_resolution.toInt(), "DS18B20_resolution");

      // Glue everything together and send to client
      server.send(200, "text/html", ds18b20_enable + sep + ds18b20_listbox + sep + ds18b20_intlistbox + sep + ds18b20_resolistbox + sep + ds18b20_var + sep + ErrorList[ErrorWifi] + sep + ErrorList[ErrorEeprom] + sep + ErrorList[ErrorDs18b20] + sep + ErrorList[ErrorUpgrade]);
    }
    if (form == "ds18b20")
    {
      String DS18B20_boolArg = server.arg("DS18B20_bool");
      String DS18B20_pinArg = server.arg("DS18B20_pin");
      String DS18B20_varArg = server.arg("DS18B20_var");
      String DS18B20_intervalArg = server.arg("DS18B20_interval");
      String DS18B20_resolutionArg = server.arg("DS18B20_resolution");
  
      if (DS18B20_boolArg == "on")
      {
        DS18B20_boolArg = "1";
      }
      else
      {
        DS18B20_boolArg = "0";
      }

      HandleEeprom(enableds18b20_Address, "write", DS18B20_boolArg);
      HandleEeprom(ds18b20pin_Address, "write", DS18B20_pinArg);
      HandleEeprom(ds18b20var_Address, "write", DS18B20_varArg);
      HandleEeprom(ds18b20interval_Address, "write", DS18B20_intervalArg);
      HandleEeprom(ds18b20resolution_Address, "write", DS18B20_resolutionArg);

      server.send ( 200, "text/html", "OK");
      delay(500);
      ESP.restart();
    }
  }
}

void handle_dht_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
	  String form = server.arg("form");
	  if (form != "dht")
	  {
		String dht_enable = HandleEeprom(enabledht_Address, "read");
		String dht_pin = HandleEeprom(dhtpin_Address, "read");
		String dht_type = HandleEeprom(dhttype_Address, "read");
		String dht_interval = HandleEeprom(dhtinterval_Address, "read");
		String dhttemp_var = HandleEeprom(dhttempvar_Address, "read");
		String dhthum_var = HandleEeprom(dhthumvar_Address, "read");
		
		String dht_listbox = "";
		if (dht_pin != "")
		{
			dht_listbox = HWListBox(0, 16, dht_pin.toInt(), "dht_pin", "dht");
		}
		else
		{
		  dht_listbox = HWListBox(0, 16, -1, "dht_pin", "dht");
		}
		
		String dht_intlistbox = ListBox(1, 5, dht_interval.toInt(), "dht_interval");
		String dht_typelistbox = ListBox(1, 2, dht_type.toInt(), "dht_type");

		// Glue everything together and send to client
		server.send(200, "text/html", dht_enable + sep + dht_listbox + sep + dht_intlistbox + sep + dht_typelistbox + sep + dhttemp_var + sep + dhthum_var + sep + ErrorList[ErrorWifi] + sep + ErrorList[ErrorEeprom] + sep + ErrorList[ErrorDs18b20] + sep + ErrorList[ErrorUpgrade]);
	  }
	  if (form == "dht")
	  {
		String dht_boolArg = server.arg("dht_bool");
		String dht_pinArg = server.arg("dht_pin");
		String dht_tempvarArg = server.arg("dhttemp_var");
		String dht_humvarArg = server.arg("dhthum_var");
		String dht_typeArg = server.arg("dht_type");
		String dht_intervalArg = server.arg("dht_interval");

		if (dht_boolArg == "on")
		{
		  dht_boolArg = "1";
		}
		else
		{
		  dht_boolArg = "0";
		}
		HandleEeprom(enabledht_Address, "write", dht_boolArg);
		HandleEeprom(dhtpin_Address, "write", dht_pinArg);
		HandleEeprom(dhttype_Address, "write", dht_typeArg);
		HandleEeprom(dhtinterval_Address, "write", dht_intervalArg);
		HandleEeprom(dhttempvar_Address, "write", dht_tempvarArg);
		HandleEeprom(dhthumvar_Address, "write", dht_humvarArg);
		
		server.send ( 200, "text/html", "OK");
		delay(500);
		ESP.restart();
	  }
	}
}

void handle_wifi_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
	  String form = server.arg("form");
	  if (form != "wifi")
	  {
		String ssidStored = HandleEeprom(ssid_Address, "read");
		String passStored = HandleEeprom(password_Address, "read");

		// Glue everything together and send to client
		server.send(200, "text/html", ssidStored + sep + passStored);
	  }
	  if (form == "wifi")
	  {
		String ssidArg = server.arg("ssid");
		String passArg = server.arg("password");

		HandleEeprom(ssid_Address, "write", ssidArg);
		HandleEeprom(password_Address, "write", passArg);
		server.send ( 200, "text/html", "OK");
		delay(500);
		ESP.restart();
	  }
	}
}

void handle_login_ajax()
{
  String form = server.arg("form");
  String action = server.arg("action");
  if (action == "logoff")
  {
    String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPIMATIC=0\r\nLocation: /login.html\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
  }
  else if (form == "login")
  {
    String UserArg = server.arg("user");
    String passArg = server.arg("password");

    String WebPass = HandleEeprom(webpass_Address, "read");
    String WebUser = HandleEeprom(webuser_Address, "read");

    MD5Builder md5;
    md5.begin();
    md5.add(WebPass);
    md5.calculate();
    String WebPassMD5 = md5.toString();

    if (UserArg == WebUser && passArg == WebPass)
    {
      String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPIMATIC=" + WebPassMD5 + "; max-age=86400\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";      
      server.sendContent(header);
    }
    else
    {
		server.send ( 200, "text/html", "Username and/or password wrong");
    }
  }
  else
  {
	server.send ( 200, "text/html", "nothing");
  }
  
}

void handle_filemanager_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
	  String form = server.arg("form");
	  if (form != "filemanager")
	  {
		String HTML;
		Dir dir = SPIFFS.openDir("/");
		while (dir.next())
		{
		  fileName = dir.fileName();
		  size_t fileSize = dir.fileSize();
		  HTML += String("<option>") + fileName + String("</option>");
		}

		// Glue everything together and send to client
		server.send(200, "text/html", HTML);
	  }
	}
}

void handle_pimatic_ajax()
{
  if (!is_authenticated(0) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
	  String form = server.arg("form");
	  if (form != "pimatic")
	  {
		String PimaticHostStored = HandleEeprom(pimhost_Address, "read");
		String PimaticPortStored = HandleEeprom(pimport_Address, "read");
		String PimaticUserStored = HandleEeprom(pimuser_Address, "read");
		String PimaticPassStored = HandleEeprom(pimpass_Address, "read");

		// Glue everything together and send to client
		server.send(200, "text/html", PimaticHostStored + sep + PimaticPortStored + sep + PimaticUserStored + sep + PimaticPassStored + sep + ErrorList[ErrorWifi] + sep + ErrorList[ErrorEeprom] + sep + ErrorList[ErrorDs18b20] + sep + ErrorList[ErrorUpgrade]);
	  }
	  if (form == "pimatic")
	  {
		String PimaticHostArg = server.arg("pimatichost");
		String PimaticPortArg = server.arg("pimaticport");
		String PimaticUserArg = server.arg("pimaticuser");
		String PimaticPassArg = server.arg("pimaticpassword");

		HandleEeprom(pimhost_Address, "write", PimaticHostArg);
		HandleEeprom(pimport_Address, "write", PimaticPortArg);
		HandleEeprom(pimuser_Address, "write", PimaticUserArg);
		HandleEeprom(pimpass_Address, "write", PimaticPassArg);

		server.send ( 200, "text/html", "OK");
	  }
	}
}

void send_data(String data, String sensor)
{
  String PimaticHostStored = HandleEeprom(pimhost_Address, "read");
  String PimaticPortStored = HandleEeprom(pimport_Address, "read");
  String PimaticUserStored = HandleEeprom(pimuser_Address, "read");
  String PimaticPassStored = HandleEeprom(pimpass_Address, "read");

  String yourdata;
  char uname[BASE64_LEN];
  String str = String(PimaticUserStored) + ":" + String(PimaticPassStored);
  str.toCharArray(uname, BASE64_LEN);
  memset(unameenc, 0, sizeof(unameenc));
  base64_encode(unameenc, uname, strlen(uname));


  const char* Hostchar = PimaticHostStored.c_str();
  const char* Portchar = PimaticPortStored.c_str();
  if (!client.connect(PimaticHostStored.c_str(), PimaticPortStored.toInt()))
  {
    Serial.println("connection failed");
    return;
  }

  yourdata = "{\"type\": \"value\", \"valueOrExpression\": \"" + data + "\"}";

  client.print("PATCH /api/variables/");
  client.print(sensor);
  client.print(" HTTP/1.1\r\n");
  client.print("Authorization: Basic ");
  client.print(unameenc);
  client.print("\r\n");
  client.print("Host: " + PimaticHostStored +"\r\n");
  client.print("Content-Type:application/json\r\n");
  client.print("Content-Length: ");
  client.print(yourdata.length());
  client.print("\r\n\r\n");
  client.print(yourdata);
  const char* status = "true";

  delay(500);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.print(line);
  }
}

String get_dht()
{    
  String dht_type = HandleEeprom(dhttype_Address, "read");
  String dhttemp_var = HandleEeprom(dhttempvar_Address, "read");
  String dhthum_var = HandleEeprom(dhthumvar_Address, "read");
  String dhtpin = HandleEeprom(dhtpin_Address, "read");
  float t;
  float h;

  int retry = 1;
  while( true && retry <= 3 )
  {
    t = dht.readTemperature();
    h = dht.readHumidity();
    if (!isnan(h) && !isnan(t) )
    {
      int temperature = t;
      if (MatrixEnabled == "1" && ShowOnMatrix == "2")
      {
        int temperature = t;
        int ones = (temperature % 10); // extract ones from temperature
        int tens = ((temperature / 10) % 10); // extract tens from temperature
        CharOnLED(ones, 0);
        CharOnLED(tens, 1);
      }
      if (MatrixEnabled == "1" && ShowOnMatrix == "3")
      {
        int hum = h;
        int ones = (hum % 10); // extract ones from temperature
        int tens = ((hum / 10) % 10); // extract tens from temperature
        CharOnLED(ones, 0);
        CharOnLED(tens, 1);
      }
      
      LastDHTtemp = String(t);
      LastDHThum = String(h);
      break;
    }
    if (retry == 3)
    {
      ErrorList[ErrorDht] = 1;
    }
    ++retry;
  }
  

  String myString = String(t) + "," + String(h);
  return myString;
}

String get_ds18b20()
{
  // Read DS18B20 and transmit value as sensor 1
  float temperaturefloat;
  //sensors.begin(); //start up temp sensor
  sensors.requestTemperatures(); // Get the temperature
  temperaturefloat = sensors.getTempCByIndex(0); // Get temperature in Celcius


  int SendTemp;
  int type = 1;

  int temperature = temperaturefloat;

  
  if (MatrixEnabled == "1" && ShowOnMatrix == "1")
  {
    int   ones = (temperature % 10); // extract ones from temperature
    int tens = ((temperature / 10) % 10); // extract tens from temperature
    String tt = "10";
    CharOnLED(ones, 0);
    CharOnLED(tens, 1);
  }

  String myString = String(temperaturefloat);
  LastDS18B20 = myString;
  return myString;
}

void loop (void)
{
  if (millis() - ds18b20_lastInterval > ds18b20_sendInterval && DS18B20Enabled == "1")
  {
    String temp = get_ds18b20();
    String ds18b20_var = HandleEeprom(ds18b20var_Address, "read");
    send_data(temp, ds18b20_var);
    ds18b20_lastInterval = millis();    
  }
  
  if (millis() - dht_lastInterval > dht_sendInterval && DHTEnabled == "1")
  {
    String dhttemp_var = HandleEeprom(dhttempvar_Address, "read");
    String dhthum_var = HandleEeprom(dhthumvar_Address, "read");
    String dhtTempHum = get_dht();

    int commaIndex = dhtTempHum.indexOf(",");
    int secondCommaIndex = dhtTempHum.indexOf(",", commaIndex+1);
    String dht_temp = dhtTempHum.substring(0, commaIndex);
    String dht_hum = dhtTempHum.substring(commaIndex+1, secondCommaIndex);
    
    if(dht_temp != "nan")
    {
      send_data(dht_temp, dhttemp_var);
    }
    if(dht_hum != "nan")
    {
      send_data(dht_hum, dhthum_var);
    }
    
    dht_lastInterval = millis();
  }

  if (millis() - adc_lastInterval > adc_sendInterval && ADCEnabled == "1")
  {
    ADC = analogRead(A0);
    String adc_var = HandleEeprom(adcvar_Address, "read");
    send_data(String(ADC), adc_var);
    adc_lastInterval = millis();    
  }
  
  server.handleClient();
}

void CharOnLED(int ch, int led)
{
  if ((ch == ' ') || (ch == '+')) int FromAlph = 10;
  if (ch == ':') int FromAlph =  11;
  if (ch == '-') int FromAlph =  12;
  if (ch == '.') int FromAlph =  13;
  if ((ch == '(')) int FromAlph =   14; //replace by 'ñ'
  if ((ch >= '0') && (ch <= '9')) int FromAlph =  (ch - '0');
  if ((ch >= 'A') && (ch <= 'Z')) int FromAlph =  (ch - 'A' + 15);
  if ((ch >= 'a') && (ch <= 'z')) int FromAlph =  (ch - 'a' + 15);

  for (int i = 0 ; i <= 7; i += 1)
  {
    lc.setRow(led, i, alphabetBitmap[ch][i]);
  }
}


String HWListBox(int first, int last, int selected, String ListName, String Hardware)
{
  String HTML = "";
  int UsedPins[] = {99, 99, 99, 99, 99, 99, 99, 99 , 99};
  int ds18b20_pos = 0;
  int matrix1_pos = 1;
  int matrix2_pos = 2;
  int matrix3_pos = 3;
  int irled_pos = 4;
  int relay1_pos = 5;
  int relay2_pos = 6;
  int relay3_pos = 7;
  int relay4_pos = 8;

  String matrix_enable = HandleEeprom(enablematrix_Address, "read");
  String ds18b20_enable = HandleEeprom(enableds18b20_Address, "read");
  String relay_enable = HandleEeprom(enablerelay_Address, "read");
  String irled_enable = HandleEeprom(enableir_Address, "read");

  String matrix_pin = HandleEeprom(matrixpin_Address, "read");
  String irled_pin = HandleEeprom(irpin_Address, "read");
  String ds18b20_pin = HandleEeprom(ds18b20pin_Address, "read");
  String relay1_pin = HandleEeprom(relay1pin_Address, "read");
  String relay2_pin = HandleEeprom(relay2pin_Address, "read");
  String relay3_pin = HandleEeprom(relay3pin_Address, "read");
  String relay4_pin = HandleEeprom(relay4pin_Address, "read");


    String AllGpio = HandleEeprom(availablegpio_Address, "read");
    // Length (with one extra character for the null terminator)
    int str_len = AllGpio.length() + 1; 
    // Prepare the character array (the buffer) 
    char char_array[str_len];
    // Copy it over 
    AllGpio.toCharArray(char_array, str_len);

    char gpio0 = AllGpio[0];
    char gpio1 = AllGpio[1];
    char gpio2 = AllGpio[2];
    char gpio3 = AllGpio[3];
    char gpio4 = AllGpio[4];
    char gpio5 = AllGpio[5];
    char gpio6 = AllGpio[6];
    char gpio7 = AllGpio[7];
    char gpio8 = AllGpio[8];
    char gpio9 = AllGpio[9];
    char gpio10 = AllGpio[10];
    char gpio11 = AllGpio[11];
    char gpio12 = AllGpio[12];
    char gpio13 = AllGpio[13];
    char gpio14 = AllGpio[14];
    char gpio15 = AllGpio[15];
    char gpio16 = AllGpio[16];
  

  if (matrix_enable == "1")
  {
    //GPIO14 = Clk   is connected to CLK  [pin 13 on max7219]
    //GPIO13 = MOSI  is connected to DATA in [pin 1 on max7219]
    UsedPins[matrix2_pos] = 13;
    UsedPins[matrix3_pos] = 14;
  }

  if (matrix_enable == "1" && Hardware != "ledmatrix")
  {
    UsedPins[matrix1_pos] = matrix_pin.toInt();
  }

  if (ds18b20_enable == "1" && Hardware != "ds18b20")
  {
    UsedPins[ds18b20_pos] = ds18b20_pin.toInt();
  }

  if (irled_enable == "1" && Hardware != "irled")
  {
    UsedPins[irled_pos] = irled_pin.toInt();
  }

  if (relay_enable == "1" && Hardware != "relay")
  {
    UsedPins[relay1_pos] = relay1_pin.toInt();
    UsedPins[relay2_pos] = relay2_pin.toInt();
    UsedPins[relay3_pos] = relay3_pin.toInt();
    UsedPins[relay4_pos] = relay4_pin.toInt();
  }

  //if (led_enable == "1" && Hardware != "led")
  //{
    //UsedPins[led_pos] = led_pin.toInt();
  //}

  if (selected == -1)
  {
    HTML += String("<option disabled selected> -- Select GPIO --</option>");
  }

  int ArrPos = -1 ;
  int SizeOfArr = (sizeof(UsedPins) / 4 ) - 1 ;

  for (int i = first; i < (last + 1); i++)
  {
    String option = String(i);
    ArrPos = -1 ;
    for (int p = 0; p < (SizeOfArr + 1); p++)
    {
      if (UsedPins[p] == i)
      {
        ArrPos = p;
        break;
      }
    }

    String disabled = "";
    if (ArrPos >= 0 )
    {
      disabled = " disabled ";
      option += " (In Use)";
    }

    int someInt = AllGpio[i] - '0';
    if (someInt == 0)
    {
      disabled = " disabled ";
      option += " (NA)";
    }



    if (i == selected)
    {
      HTML += String("<option selected" + disabled + ">") + option + String("</option>");
    }
    else
    {
      HTML += String("<option" + disabled + ">") + option + String("</option>");
    }
  }
  return HTML;
}

String ListBox(int first, int last, int selected, String ListName)
{
  String HTML = "";
  for (int i = first; i < (last + 1); i++)
  {
    if (i == selected)
    {
      HTML += String("<option selected value='") + i + String("'>") + i + String("</option>");
    }
    else
    {
      HTML += String("<option value='") + i + String("'>") + i + String("</option>");
    }
  }
  return HTML;
}

// An empty ESP8266 Flash ROM must be formatted before using it, actual a problem
void handleFormat()
{
  if (!is_authenticated(1) && EnableWebAuth == "1")
  {
    String header = "HTTP/1.1 301 OK\r\nLocation: /login.html\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header); 
  }
  else
  {
    server.send ( 200, "text/html", "OK");
    Serial.println("Format SPIFFS");
    if (SPIFFS.format())
    {
      if (!SPIFFS.begin())
      {
        Serial.println("Format SPIFFS failed");
      }
    }
    else
    {
      Serial.println("Format SPIFFS failed");
    }
    if (!SPIFFS.begin())
    {
      Serial.println("SPIFFS failed, needs formatting");
    }
    else
    {
      Serial.println("SPIFFS mounted");
    }
  }
}

void handleFileDelete()
{
  if (!is_authenticated(1) && EnableWebAuth == "1")
  {
    server.send ( 200, "text/html", "unauthorized");
  }
  else
  {
	  if (server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
	  String path = server.arg(0);
	  if (!path.startsWith("/")) path = "/" + path;
	  Serial.println("handleFileDelete: " + path);
	  if (path == "/")
		return server.send(500, "text/plain", "BAD PATH");
	  if (!SPIFFS.exists(path))
		return server.send(404, "text/plain", "FileNotFound");
	  SPIFFS.remove(path);
	  server.send(200, "text/plain", "");
	  path = String();
	}
}


void handle_loginm_html()
{
  String content = "<html><body><form action='/login_ajax' method='POST'>Please login<br>";
  content += "<input type='hidden' name='form' value='login'>";
  content += "User:<input type='text' name='user' placeholder='user name'><br>";
  content += "Password:<input type='password' name='password' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form><br>";
  server.send(200, "text/html", content);
}


bool is_authenticated(int SetCookie)
{
  String WebUser = HandleEeprom(webuser_Address, "read");
  String WebPass = HandleEeprom(webpass_Address, "read");

  MD5Builder md5;
  md5.begin();
  md5.add(WebPass);
  md5.calculate();
  String WebPassMD5 = md5.toString();
  String ValidCookie = "ESPIMATIC=" + WebPassMD5;
  if (server.hasHeader("Cookie"))
  {
    String cookie = server.header("Cookie");
    if (cookie.indexOf(ValidCookie) != -1)
    {
      if (SetCookie == 1)
      {
        server.sendHeader("Set-Cookie","ESPIMATIC=" + WebPassMD5 + "; max-age=86400"); 
      }
      return true;
    }
  }
  return false;  
}

