
/////////////////
//General
///////////////////////
#include <BlynkSimpleEsp8266.h> //#include <SimpleTimer.h> 
BlynkTimer timer;//SimpleTimer timer;

const int rPin =  D6; //relay PIN
bool relay_ST;//Relay status OFF ON

bool modbus_enabled=true, modbus_OK;
bool blynk_enabled = true, blynk_OK;
bool thingspeak_enabled = true, thingspeak_OK;
bool NTP_enabled = true, NTP_OK;

///////////////////
//Display
/////////////////////
//Default D1 SCL, D2 SDA. OLED Reversed SDA=D1, SCL=D2. Revertir tambien otros I2C en el bus.
#include <Wire.h>
#include <Adafruit_SSD1306.h>
//#include <Fonts/FreeMono9pt7b.h> //Opcional
Adafruit_SSD1306 display(128, 64, &Wire, D0);//WIDTH, HEIGHT,, RESET

void Display_Ini() {
  Wire.begin(D1, D2); Wire.setClock(400000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  //display.setFont(&FreeMono9pt7b);
  display.setTextSize(1);             
  display.setTextColor(WHITE);
}

////////////////////////////
// WIFI +MDNS
///////////////////////////
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>//Para mDNS y que la IDE descubra las IPs OTA

const char* WIFI_SSID = "WIFI SSID";
const char* WIFI_PWD = "WIFI PASSWORD";
const char* HOST_NAME="ESP";
const String HOST_DM="http://"+String(HOST_NAME)+".local";
//Static IP address configuration
IPAddress staticIP(192, 168, 2, 22);
IPAddress gateway(192, 168, 2, 1);   
IPAddress subnet(255, 255, 255, 0); 
IPAddress dns(192, 168, 2, 1);

#define WIFI_TRY 40 //Secs Timeout

bool WIFI_OK, WIFI_OK_last;//WIFI status 
int WIFI_DisCounter; //WIFI Disconnections counter

void WIFI_Ini() {
  
  Serial.println("Connecting WiFi ");
  display.setTextSize(1); display.clearDisplay(); display.print("WIFI ");display.display();
  
  WiFi.persistent(false);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);//Supposedly to improve wifi stability
  WiFi.mode(WIFI_STA);
  WiFi.config(staticIP, gateway, subnet,dns);
  WiFi.hostname(HOST_NAME);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  WiFi.setAutoReconnect (true);
  
  int _try = 0;
  do {
    delay(500);
    _try++;
    Serial.print(".");
    display.print(".");display.display(); 
    WIFI_OK = (WiFi.status() == WL_CONNECTED);
  } while (!WIFI_OK && _try < WIFI_TRY*2);

  Serial.println();Serial.println(WiFi.localIP());
  display.println();display.println(WiFi.localIP());
  
  //Multicast DNS domain name server
  MDNS.begin(HOST_NAME);
  Serial.println(HOST_DM);
  display.println(HOST_DM);

  NTP_Ini();
  Blynk_Ini();
  
  display.display();
  delay(2000);
  
}

////////////////////
// NTP Time
//////////////////
#include <TimeLib.h>
#include <NtpClientLib.h>

#define NTP_TIMEOUT 1000 // ms
#define NTP_INTERVAL 86400 //secs (24h) (normal)
#define NTP_SHORT_INTERVAL 300 //secs (in case of failure)
#define NTP_TZ 1 // UTC +1h Central Europe
#define NTP_DST DST_ZONE_EU //Summer Winter time

bool NTP_i;//Flag Initialized
unsigned int NTP_c;//Sync. Counter

void NTP_Ini() {
  if (!NTP_enabled || !WIFI_OK ) return;
  ESP.wdtEnable(3000);
  NTP.setInterval(NTP_SHORT_INTERVAL, NTP_INTERVAL);
  NTP.setNTPTimeout (NTP_TIMEOUT);
  NTP.setDSTZone(NTP_DST);
  NTP.begin ("pool.ntp.org", NTP_TZ, true, 0);
  NTP.onNTPSyncEvent ([](NTPSyncEvent_t event) {
    if (event < 0) NTP_OK = false;
    if (event == timeSyncd) {NTP_OK = true; NTP_c++; }
  });
  NTP_i = true;
}

//////////////////////////////
//REMOTE DEBUG
//////////////////////////////

#define DEBUG_DISABLED

#ifdef DEBUG_DISABLED
  void Log (char*,...) {}
  void Log_(char*,...) {}
  void RemoteDebug_Ini() {}
  void RemoteDebug_Handle() {}
#else
  #include "RemoteDebug.h"
  RemoteDebug Debug;
  
  #define Log(fmt, ...) debugD(fmt, ##__VA_ARGS__) //Ln
  #define Log_(fmt, ...) rdebugD(fmt, ##__VA_ARGS__) //Misma linea
  
  void RemoteDebug_Ini() {
    Debug.begin(HOST_NAME);
    Debug.setResetCmdEnabled(true); 
    Debug.showProfiler(false); 
    Debug.showColors(true);
    Debug.setSerialEnabled(false);//Pasa la info al puerto serie
  }
  void RemoteDebug_Handle() {Debug.handle();}
#endif
          

//////////////////////////////
//OTA Over The Air Download
//////////////////////////////

#include <WiFiUdp.h>
#include <ArduinoOTA.h>

void OTA_Ini() {

  //ArduinoOTA.setHostname(HOST_NAME);
  //ArduinoOTA.setPassword("");
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesytem";
    }
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("OTA Started");
}


/////////////////
//Aux functions
///////////////////////

char* f2s(float x) {
  char buf[10];//Buffer Float a char string (3 decs). Used in Logs
  dtostrf(x, 2, 3, buf);
  return buf; 
}


float absf(float f) {return (f > 0) ? f: -f;} //Absolute float. Warning!! abs() macro fails with <softwareserial.h>

String getTimeStr10 (time_t u) {//Uptime secs formatted 10 string
    uint16_t days; uint8_t hours, minutes;
    char buf[12];
    days = u / 86400;
    u -= days * 86400;
    hours = u / 3600;
    u -= hours * 3600;
    minutes = u / 60; 
    sprintf (buf, "%4uD%02d:%02d", days, hours, minutes);
    return buf;
}

String getDTStr(time_t t) { //UNIX Time secs. to String, Thingspeak format
  char buf[32];
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
  return buf;
}

////////////////// Debug Time Processes
#define D_SIZE 8
long unsigned delta_t[D_SIZE], t_0[D_SIZE],delta_t_max[D_SIZE], d_n[D_SIZE];
float delta_t_med[D_SIZE];

void t_start(int ix) {
  t_0[ix]=millis();
}
void t_end(int ix) {
  delta_t[ix]=millis()-t_0[ix]; 
  if (delta_t[ix]>delta_t_max[ix]) delta_t_max[ix]=delta_t[ix]; 
  delta_t_med[ix] = ( delta_t_med[ix] * d_n[ix] + delta_t[ix] )/(d_n[ix]+1);
  d_n[ix]++;if (d_n[ix]>=10000) d_n[ix]=0; 
}
//////////////////////////////////////////////////

////////////////////////////
//DHT11 Temperature and Humidity
//////////////////////////////
#include <Adafruit_Sensor.h>
#include <DHT.h>
#define DHTTYPE DHT11
DHT dht(D5, DHTTYPE);

float dht_temp,dht_hum;

void DHT_Ini() {
  getDHT();
}

  
void getDHT() { //aprox. 30ms
  float x; 
  float w_t=0.05, w_h=0.05;//Weight exponential filters
  x = dht.readTemperature();if (!isnan(x) && x>=0 && x<=50) dht_temp = (dht_temp == 0.f) ? x: w_t*x+(1-w_t)*dht_temp; //Range 0-50C
  x = dht.readHumidity();if (!isnan(x) && x>=0 && x<=100) dht_hum = (dht_hum == 0.f) ? x: w_h*x+(1-w_h)*dht_hum;    //Range 20-95%
  
  Log("DHT_Temp: %s",f2s(dht_temp));
  Log("DHT_Hum: %s",f2s(dht_hum));
}

////////////////////////////
//DS18B20 Temperature Sensor
//////////////////////////////
#include <OneWire.h>
#include<DallasTemperature.h>  
OneWire oneWire(D7);//Pin
DallasTemperature DS18B20(&oneWire);
float ds_temp;

void DSTemp_Ini() {
  DS18B20.setResolution(12);//Resolution  9 (0.5C), 10 (0.25C), 11 o 12. //9 spends 100ms,  12 750ms.
  DS18B20.begin();
  getDSTemp();
  DS18B20.requestTemperatures();
  DS18B20.setWaitForConversion(false); // Don't block the program while the temperature sensor is reading
}

void getDSTemp() { //Aprox. 100ms
  ds_temp = DS18B20.getTempCByIndex(0);
  DS18B20.requestTemperatures();
  Log("DS_Temp: %s",f2s(ds_temp));
}

///////////////////////////////
//Analog Inputs Volts and Amps
////////////////////////////////
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads1(0x48);
Adafruit_ADS1115 ads2(0x4A);

float v[8],e[8];

void ADS_Ini() { //Analog Inputs Init. 

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  //ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  ads1.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads2.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads1.begin();
  ads2.begin();
}
  
void getADS() { //Aprox. 75ms
  
  int16_t adc[8];
  float x[8];
  float w=0.10;//Weight exponential filters
  
  adc[0] = ads1.readADC_SingleEnded(0); //Log("AIN0: %d", adc0); 
  adc[1] = ads1.readADC_SingleEnded(1); 
  adc[2] = ads1.readADC_SingleEnded(2); 
  adc[3] = ads1.readADC_SingleEnded(3); 
  adc[4] = ads2.readADC_SingleEnded(0); 
  adc[5] = ads2.readADC_SingleEnded(1); 
  adc[6] = ads2.readADC_SingleEnded(2); 
  adc[7] = ads2.readADC_SingleEnded(3); 
  
  x[0]=  (adc[0] * 0.1875)/1000;//Voltage BASE 5V
  x[1]=  - 1.23 * 20 * 2/x[0] * ((adc[1] * 0.1875)/1000-x[0]/2-0.0000);//Amp 
  x[2]=  - 1.23 * 20 * 2/x[0] * ((adc[2] * 0.1875)/1000-x[0]/2+0.0010);//Amp 
  x[3]=  - 1.23 * 20 * 2/x[0] * ((adc[3] * 0.1875)/1000-x[0]/2-0.0014);//Amp 
  x[4]=  - 1.24 * 20 * 2/x[0] * ((adc[4] * 0.1875)/1000-x[0]/2+0.0004);//Amp 
  x[5]=  - 1.25 * 20 * 2/x[0] * ((adc[5] * 0.1875)/1000-x[0]/2-0.0024);//Amp 
  x[6]=  0.993 * 5 * (adc[6] * 0.1875)/1000;//Voltage IN
  x[7]=  (adc[7] * 0.1875)/1000;//Reserve 

  //Exponencial Filter (Only after first measurement)
  for (int i=0; i<8 ; i++) e[i] = (e[i]==0.f) ? x[i] : w*x[i]+(1-w)*e[i];

  for (int i=0; i<8 ; i++) v[i] = ( absf(e[i]) < 0.01 ) ? 0 : v[i] = e[i];

}


//////////////////////////////
//MODBUS RTU
//////////////////////////////
#include <ModbusMaster.h>
#include <SoftwareSerial.h>//To leave free UART0 for USB upload and Serial Monitor.
SoftwareSerial swSer(D3,D4,false,128);
#define MB_TIME 5000 //ModBus ms polling frequency

struct Tracer {
  time_t T;String TimeStr;//Data Time UNIX secs, String TimeStamp
  float PV_volt; float PV_cur;float PV_pw;//Panels voltage (V), current (A) and input Power (W)
  float BAT_volt; float BAT_cur; float BAT_pw; //Battery voltage (V), current (A), charge Power (W)
  float LD_volt; float LD_cur; float LD_pw;//DC LOAD voltage (V), current (A), Power (W)
  float T_bat; float T_inside;   //Battery Temp., Box temp. (‎°C)
  float BAT_SOC;float T_rbat;//Battery charge status (%), Battery remote temp. (‎°C)
  unsigned int batstatus; unsigned int sysstatus;unsigned int ldstatus;//battery status (bits), system status (bits). load status (bits)
  float pv_max_volt_today; float pv_min_volt_today; //Max. and Min. Panels voltages today (V)
  float bat_max_volt_today; float bat_min_volt_today;// Max. and Min. Battery voltages today (V)
  float conEnT; float conEnM; float conEnY; float conEnTot; //Comsumed Energy Today, Month, Year, Total ( LOAD DC?) KWH
  float genEnT; float genEnM; float genEnY; float genEnTot; //Generated Energy Today, Month, Year, Total KWH
  bool LD_ST;//Status DC LOAD OFF ON
};

ModbusMaster node;
Tracer EP;
uint8_t result;

// tracer requires no handshaking
void preTransmission() {}
void postTransmission() {}

// Modbus slave ID 1
void Modbus_Ini() {
  EP.LD_ST = true;
  swSer.begin(115200);
  ESP.wdtEnable(3000);//Evitar Software WatchDog si no hay respuestas modbus. Ademas ku16MBResponseTimeout de 2000ms a 500ms en libreria modbusmaster.
  node.begin(1, swSer);//Nodo ID 1
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  getModbus_fast();
  getModbus_slow();
}

void AddressCoils() {
  Log("Modbus Read Coils ... ");    
  result = node.readCoils(0x2,1);
  if (result == node.ku8MBSuccess)
  {
    Log("OK");
    EP.LD_ST = node.getResponseBuffer(0);
  } else {
    Log("Not OK");
  }
}

void EP_LD_SET(bool v) {//Set DC LOAD OFF=0 ON=1
  //if (!modbus_enabled) {modbus_OK = false; return; };
  t_start(2);
  Log("Modbus DC LOAD Write ... ");
  result = node.writeSingleCoil(0x2,v);
  if (result == node.ku8MBSuccess)
  {
    Log("OK");
    EP.LD_ST=v;
    modbus_OK = true;
  } else {
    Log("Not OK");
  }
  t_end(2);
}

void AddressRegistry_3100() {
  Log("Modbus 3100 Read ... ");  
  result = node.readInputRegisters(0x3100, 8);
  
  if (result == node.ku8MBSuccess)
  {
    Log("OK");
    
    EP.PV_volt = (long)node.getResponseBuffer(0x00) / 100.0;
    Log("EP.PV_volt: %s V",f2s(EP.PV_volt));
    
    EP.PV_cur = (long)node.getResponseBuffer(0x01) / 100.0;
    Log("EP.PV_cur: %s A",f2s(EP.PV_cur));

    EP.PV_pw = ((long)node.getResponseBuffer(0x03) << 16 | node.getResponseBuffer(0x02)) / 100.0;
    Log("EP.PV_pw: %s W",f2s(EP.PV_pw));

    EP.BAT_volt = (long)node.getResponseBuffer(0x04) / 100.0;
    Log("EP.BAT_volt: %s V",f2s(EP.BAT_volt));

    EP.BAT_cur = (long)node.getResponseBuffer(0x05) / 100.0;
    Log("EP.BAT_cur: %s A",f2s(EP.BAT_cur));

    EP.BAT_pw = ((long)node.getResponseBuffer(0x07) << 16 | node.getResponseBuffer(0x06)) / 100.0;
    Log("EP.BAT_pw: %s W",f2s(EP.BAT_pw));
   
  } else {
    Log("Not OK");
  }
}

void AddressRegistry_310C() {
  Log("Modbus 310C Read ... ");  
  result = node.readInputRegisters(0x310C, 7);
  
  if (result == node.ku8MBSuccess)
  {
    Log("OK");

    EP.LD_volt = (long)node.getResponseBuffer(0x00) / 100.0;
    Log("EP.LD_volt: %s V",f2s(EP.LD_volt));

    EP.LD_cur = (long)node.getResponseBuffer(0x01) / 100.0;
    Log("EP.LD_cur: %s A",f2s(EP.LD_cur));

    EP.LD_pw = ((long)node.getResponseBuffer(0x03) << 16 | node.getResponseBuffer(0x02)) / 100.0;
    Log("EP.LD_pw: %s W",f2s(EP.LD_pw));
    
    EP.T_bat = (long)node.getResponseBuffer(0x04) / 100.0;
    Log("EP.T_bat: %s °C",f2s(EP.T_bat));

    EP.T_inside = (long)node.getResponseBuffer(0x05) / 100.0;
    Log("EP.T_inside: %s °C",f2s(EP.T_inside));
    
  } else {
    Log("Not OK");
  }
}

void AddressRegistry_311A() {
  Log("Modbus 311A Read ... ");  
  result = node.readInputRegisters(0x311A, 2);
  
  if (result == node.ku8MBSuccess)
  {
    Log("OK");

    EP.BAT_SOC = node.getResponseBuffer(0x00) / 1.0;
    Log("EP.BAT_SOC: %s %",f2s(EP.BAT_SOC));

    EP.T_rbat = node.getResponseBuffer(0x01) / 100.0;
    Log("EP.T_rbat: %s °C",f2s(EP.T_rbat));
    
  } else {
    Log("Not OK");
  }
}

void AddressRegistry_3200() {
  Log("Modbus 3200 Read ... ");  
  result = node.readInputRegisters(0x3200, 3);
  
  if (result == node.ku8MBSuccess)
  {
    Log("OK");

    EP.batstatus = node.getResponseBuffer(0x00);
    Log("EP.batstatus: %u",EP.batstatus);

    EP.sysstatus = node.getResponseBuffer(0x01);
    Log("EP.sysstatus: %u",EP.sysstatus);

    EP.ldstatus = node.getResponseBuffer(0x02);
    Log("EP.ldstatus: %u",EP.ldstatus);
    
  } else {
    Log("Not OK");
  } 
}


void AddressRegistry_3300() {
  Log("Modbus 3300 Read ... ");  
  result = node.readInputRegisters(0x3300, 20);
  
  if (result == node.ku8MBSuccess)
  {
    Log("OK");

    EP.pv_max_volt_today = node.getResponseBuffer(0x00)/100.0;
    Log("EP.pv_max_volt_today: %s V",f2s(EP.pv_max_volt_today));

    EP.pv_min_volt_today = node.getResponseBuffer(0x01)/100.0;
    Log("EP.pv_min_volt_today: %s V",f2s(EP.pv_min_volt_today));

    EP.bat_max_volt_today = node.getResponseBuffer(0x02)/100.0;
    Log("EP.bat_max_volt_today: %s V",f2s(EP.bat_max_volt_today));

    EP.bat_min_volt_today = node.getResponseBuffer(0x03)/100.0;
    Log("EP.bat_min_volt_today: %s V",f2s(EP.bat_min_volt_today));

    EP.conEnT = ((long)node.getResponseBuffer(0x05) << 16 | node.getResponseBuffer(0x04)) / 100.0;
    Log("EP.conEnT: %s KWH",f2s(EP.conEnT));

    EP.conEnM = ((long)node.getResponseBuffer(0x07) << 16 | node.getResponseBuffer(0x06)) / 100.0;
    Log("EP.conEnM: %s KWH",f2s(EP.conEnM));

    EP.conEnY = ((long)node.getResponseBuffer(0x09) << 16 | node.getResponseBuffer(0x08)) / 100.0;
    Log("EP.conEnY: %s KWH",f2s(EP.conEnY));

    EP.conEnTot = ((long)node.getResponseBuffer(0x0B) << 16 | node.getResponseBuffer(0x0A)) / 100.0;
    Log("EP.conEnTot: %s KWH",f2s(EP.conEnTot));

    EP.genEnT = ((long)node.getResponseBuffer(0x0D) << 16 | node.getResponseBuffer(0x0C)) / 100.0;
    Log("EP.genEnT: %s KWH",f2s(EP.genEnT));

    EP.genEnM = ((long)node.getResponseBuffer(0x0F) << 16 | node.getResponseBuffer(0x0E)) / 100.0;
    Log("EP.genEnM: %s KWH",f2s(EP.genEnM));

    EP.genEnY = ((long)node.getResponseBuffer(0x11) << 16 | node.getResponseBuffer(0x10)) / 100.0;
    Log("EP.genEnY: %s KWH",f2s(EP.genEnY));

    EP.genEnTot = ((long)node.getResponseBuffer(0x13) << 16 | node.getResponseBuffer(0x12)) / 100.0;
    Log("EP.genEnTot: %s KWH",f2s(EP.genEnTot));
    
  } else {
    Log("Not OK");
  }
}


String batstatus_str (unsigned int v) {//Translate Bat Status
  
  String s = String(v) + " ";
 
  switch (v & 15) {
    case 0: s+=""; break;//Normal
    case 1: s+="/Overvolt" ; break;
    case 2: s+="/Undervolt" ; break;
    case 3: s+="/Low Volt Disc."; break;
    case 4: s+="/Fault"; break;
  }
  
  return s;
}

String sysstatus_str (unsigned int v) {//Translate Sys Status
  
  String s = String(v) + " ";
  if (!(v & 1)) s+="/Standby";
  if ((v >> 1) & 1) s+="/Fault"; 

  switch ((v >> 2) & 3) {
    case 0: s+="/No Charging"; break;
    case 1: s+="/Float" ; break;
    case 2: s+="/Boost" ; break;
    case 3: s+="/Equalization";
  }
  
  return s;
}

void getModbus_fast() { //Aprox, 150ms.
  if (!modbus_enabled) {modbus_OK = false; return; };
  t_start(3);
  AddressCoils();
  if (result == 0) {
    delay(100);
    AddressRegistry_3100();
    if (result == 0) {
      delay(100);
      AddressRegistry_310C();
      EP.T = now();EP.TimeStr = getDTStr(EP.T);
    }
  }
  modbus_OK = (result == 0); 
  t_end(3);
}

void getModbus_slow() {//Aprox, 100ms.
  if (!modbus_enabled) {modbus_OK = false; return; };
  t_start(4);
  if (result == 0 || true) {
    AddressRegistry_311A();
    if (result == 0) {
      delay(100);
      AddressRegistry_3200();
      if (result == 0) {
        delay(100);
        AddressRegistry_3300();
      }
    } 
  }
  modbus_OK = (result == 0); 
  t_end(4);
}

//////////////////////
//PROCESSED DATA
///////////////////////

#define P_TOL 0.4f //Tolerance Power

struct PRDAT {
  time_t T; String TimeStr;//Data Time UNIX secs & String TimeStamp
  float Temp;float Hum;//External Temp, Humidity
  float V;//Load Voltage
  float P_PV; float P_DLD; float P_DGEN; float P_BANK; float P_H; float P_B1; float P_B2; float  P_B3; float P_B4; float P_B5;//Power Panels, DC Load, DC Charger, Battery Bank, Bat. 1-5
  float E_PV; float E_DLD; float E_DGEN; float E_BANK; float E_H; float E_B1; float E_B2; float  E_B3; float E_B4; float E_B5;//Energy "
};

PRDAT PD;

float DT; //secs, Integration Interval
unsigned long mt,mt_last;
bool pfirst;

void proData_Ini() {PD.E_PV =  PD.E_DLD = PD.E_DGEN = PD.E_B1 = PD.E_B2 = PD.E_B3 = PD.E_B4 = PD.E_B5 = PD.E_BANK = PD.E_H = 0; pfirst = true;}

void proData() {
  
  mt=millis(); 
  DT = (pfirst || mt < mt_last) ? 0 : (mt - mt_last)/1000.0;
  mt_last = mt;
  pfirst = false;

  PD.T = now();
  PD.TimeStr = getDTStr(PD.T);
  PD.Temp = ds_temp;//Ext. Temp
  PD.Hum = dht_hum; //Humidity
  PD.V = v[6];//Load Voltage
  
  bool mbr = (PD.T - EP.T) < 30;//Modbus data refresh < 30 secs 
  //Log("mbr mt:%u last:%u",mt, modbus_last_comm);

  PD.P_PV = !mbr ? 0 : EP.PV_pw; 
  PD.P_DLD = !mbr ? 0 : EP.LD_pw;
  PD.P_DGEN = !mbr ? 0 : EP.BAT_pw - PD.P_DLD;
  PD.P_B1 = -v[1]*PD.V;if (absf(PD.P_B1) < P_TOL) PD.P_B1 = 0;  
  PD.P_B2 = -v[2]*PD.V;if (absf(PD.P_B2) < P_TOL) PD.P_B2 = 0;
  PD.P_B3 = -v[3]*PD.V;if (absf(PD.P_B3) < P_TOL) PD.P_B3 = 0;
  PD.P_B4 = -v[4]*PD.V;if (absf(PD.P_B4) < P_TOL) PD.P_B4 = 0;
  PD.P_B5 = -v[5]*PD.V;if (absf(PD.P_B5) < P_TOL) PD.P_B5 = 0;
  PD.P_BANK = PD.P_B1 + PD.P_B2 + PD.P_B3 + PD.P_B4 + PD.P_B5; 
  PD.P_H = PD.P_DGEN - PD.P_BANK; if (PD.P_H < P_TOL) PD.P_H = 0;  
   
  PD.E_PV += PD.P_PV * DT /3600.0 / 1000.0; 
  PD.E_DLD += PD.P_DLD * DT /3600.0 / 1000.0; 
  PD.E_DGEN += PD.P_DGEN * DT /3600.0 / 1000.0;
  PD.E_B1 += PD.P_B1 * DT /3600.0 / 1000.0;
  PD.E_B2 += PD.P_B2 * DT /3600.0 / 1000.0;
  PD.E_B3 += PD.P_B3 * DT /3600.0 / 1000.0;
  PD.E_B4 += PD.P_B4 * DT /3600.0 / 1000.0;
  PD.E_B5 += PD.P_B5 * DT /3600.0 / 1000.0;
  PD.E_BANK += PD.P_BANK * DT /3600.0 / 1000.0;
  PD.E_H += PD.P_H * DT /3600.0 / 1000.0;        
}

///////////////////////////////
//Historical Data for Trends
////////////////////////////////

#define HIST_PER 5 //Period X minutes
#define HIST_VAR 10 //Number of variables to save
#define HIST_SIZE 24*60/HIST_PER // Data for 24h
#define HIST_MEM  HIST_SIZE * (4 + 2 * HIST_VAR) //Required Memory

time_t hist_t[HIST_SIZE];//4 Bytes time
int hist_d[HIST_VAR][HIST_SIZE];//2 Bytes data
int hist_n, hist_last_sent;

void Hist_Ini() {
  Serial.printf("Hist.Size: %u Bytes",HIST_MEM );
}
  
void Hist_Add(){
    
  int n;
  if (hist_n < HIST_SIZE) {
    n = hist_n++;
  } else { //Shift
    n= hist_n - 1;
    for (int i = 0; i<n ; i++) {         
      hist_t[i] = hist_t[i+1];
      for (int m = 0; m < HIST_VAR ; m++) hist_d[m][i] = hist_d[m][i+1];
    }
    if (hist_last_sent > 0) hist_last_sent--; 
  }
  
  hist_t[n] = PD.T;//Time UNIX secs.
  hist_d[0][n]= PD.Temp * 100;//Temp
  hist_d[1][n]= PD.Hum * 100;//Hum
  hist_d[2][n]= PD.P_DLD * 10;//Power DC Load
  hist_d[3][n]= PD.P_DGEN * 10;// Power PV - DC Load
  hist_d[4][n]= PD.P_B1 * 10;//Power Batt. 1
  hist_d[5][n]= PD.P_B2 * 10;
  hist_d[6][n]= PD.P_B3 * 10;
  hist_d[7][n]= PD.P_B4 * 10;
  hist_d[8][n]= PD.P_B5 * 10;//Power Batt. 5
  hist_d[9][n]= PD.V * 100;// Load Voltage
}

PRDAT HD;
void Hist_Decode(int i) {
   HD.T = hist_t[i];
   HD.TimeStr = getDTStr(HD.T);
   HD.Temp = (float)hist_d[0][i]/100.0;
   HD.Hum = (float)hist_d[1][i]/100.0;
   HD.P_DLD = (float)hist_d[2][i]/10.0;//Power DC Load
   HD.P_DGEN = (float)hist_d[3][i]/10.0;//Power DC Charger
   HD.P_PV = HD.P_DLD + HD.P_DGEN ;//Power Panels
   HD.P_B1 = (float)hist_d[4][i]/10.0;
   HD.P_B2 = (float)hist_d[5][i]/10.0;
   HD.P_B3 = (float)hist_d[6][i]/10.0;
   HD.P_B4 = (float)hist_d[7][i]/10.0; 
   HD.P_B5 = (float)hist_d[8][i]/10.0;
   HD.P_BANK = HD.P_B1 + HD.P_B2 + HD.P_B3 + HD.P_B4 + HD.P_B5; //Sum (P_Bi) Batteries
   HD.P_H = HD.P_DGEN - HD.P_BANK; if (HD.P_H < P_TOL) HD.P_H = 0;//Power Home
   HD.V = (float)hist_d[9][i]/100.0;//Load Voltage
}

///////////////////////
//Data to Display, switching screens
////////////////////////
int pan=0;//Screen number in display  

void Display_N() { //Next Screen
   pan++; if (pan>4) pan=0;
}

void Display_D() { //Show Info
    
  display.setTextSize(2); 
  display.clearDisplay();
  display.setCursor(0,0);  
  
  switch (pan) {
  case 0:
    display.print("V0: ");display.print(v[0]);display.println("V");
    display.print("A1: ");display.print(v[1]);display.println("A");
    display.print("A2: ");display.print(v[2]);display.println("A");
    display.print("A3: ");display.print(v[3]);display.println("A");
    break;
  case 1:
    display.print("A4: ");display.print(v[4]);display.println("A");
    display.print("A5: ");display.print(v[5]);display.println("A");
    display.print("V6: ");display.print(v[6]);display.println("V");
    display.print("HIST = ");display.println(hist_n);
    break;
  case 2:
    display.print("PV: ");display.print(PD.P_PV,0);display.println("W");
    display.print("LD: ");display.print(PD.P_DLD,0);display.println("W");
    display.print("H : ");display.print(PD.P_H,0);display.println("W");
    display.print("BK: ");display.print(PD.P_BANK,0);display.println("W");
    break;
  case 3:  
    display.print("Ti: ");display.print(dht_temp);display.println("C");
    display.print("Hu: ");display.print(dht_hum);display.println("%");
    display.print("Te: ");display.print(ds_temp);display.println("C");
    relay_ST = digitalRead(rPin);   
    //Info WNM BT RL ([W]IFI_OK/[N]TP_COMS/[M]ODBUS_COMMS [B]LYNK_COMMS/[T]HINGSPEAK_COMMS [R]ELAY_ON/[L]OAD_ON)
    display.printf("%s%s%s %s%s %s%s ",(WIFI_OK) ? "W":"-",  (NTP_OK) ? "N":"-", (modbus_OK) ? "M":"-", (blynk_OK) ? "B":"-", (thingspeak_OK) ? "T":"-", (relay_ST) ? "R":"-", (EP.LD_ST) ? "L":"-" );
  default: 
    display.println(NTP.getDateStr());//Date
    display.print(" ");display.print(NTP.getTimeStr());display.println(" ");//Time
    display.print("RAM: ");display.println(system_get_free_heap_size());//Free RAM
    display.println(getTimeStr10(NTP.getUptime()));//Uptime
  }

  display.display();
}


////////////////////
//WEB Server
////////////////////////
#include "ESP8266WebServer.h" //No ESPAsyncWebServer to save memory
//#include <ArduinoJson.h>

ESP8266WebServer server(80);

String sjson;

String jsr(String k, String v) {return "\"" + k + "\":\"" + v + "\"";}//Json register string
String jsr(String k, float v)  {return "\"" + k + "\":\"" + String(v) +"\"" ;}//Json register number as string
String jsr(String k, float v, int d)  {return "\"" + k + "\":\"" + String(v,d) + "\"" ;}//Json register number d decimals as string

#include "index.h" //web index html

void WEB_Ini(){

  sjson.reserve(3200);//Aprox. HIST_SIZE * 11
  
  server.on("/", HTTP_GET, []{
    Log("HTTP_GET /");
    server.send_P(200, "text/html", index_html);
  });
  
  server.on("/data", HTTP_GET, []{
    Log("HTTP_GET /data");

    relay_ST = digitalRead(rPin);

    sjson = "{";
    sjson += jsr( "TimeStr", getDTStr(now()) );
    sjson += "," + jsr( "ds_temp", ds_temp);
    sjson += "," + jsr( "dht_temp", dht_temp);
    sjson += "," + jsr( "dht_hum", dht_hum);
    sjson += "," + jsr( "V0", v[0]);
    sjson += "," + jsr( "A1", v[1]);
    sjson += "," + jsr( "A2", v[2]);
    sjson += "," + jsr( "A3", v[3]);
    sjson += "," + jsr( "A4", v[4]);
    sjson += "," + jsr( "A5", v[5]);
    sjson += "," + jsr( "V6", v[6]);
    sjson += "," + jsr( "relay", relay_ST );
    sjson += "," + jsr( "modbus_st", modbus_enabled ? result:-1 );//Modbus Status 0 == OK  , -1 == Disabled, x == Error Code
    sjson += "," + jsr( "blynk_st", blynk_enabled ? blynk_OK:-1 );
    sjson += "," + jsr( "datetime", NTP.getTimeDateString() );
    sjson += "," + jsr( "info", "Up " + String(NTP.getUptimeString()) +" LMed: "+String(delta_t_med[0]) + " LMax: "+String(delta_t_max[0]) + " Hist: "+String(hist_n) + " TS: "+ String(hist_last_sent) + " WIFI_D: "+String(WIFI_DisCounter) + " NTP: "+ String(NTP_c) + " RAM : "+String(system_get_free_heap_size()) ); 
       
    String aux="["; for (int i=1; i<D_SIZE; i++)  aux += String(delta_t_max[i])+ (i == D_SIZE -1 ? "":",");aux +="]";
    sjson += "," + jsr( "info2", " D_T: "+ aux + " Built: "+ __DATE__ + " " + __TIME__ );
    sjson += "}";
     
    server.send(200, "application/json", sjson.c_str());
  });

  server.on("/relay", HTTP_GET, []{
    Log("HTTP_GET /relay");
    relay_ST = !digitalRead(rPin);
    digitalWrite(rPin, relay_ST);
    Log("relay %s ", relay_ST ? "ON":"OFF");
    server.send(200, "text/plain", String(relay_ST));
  });

  server.on("/modbus_en", HTTP_GET, []{
    Log("HTTP_GET /modbus_en");
    modbus_enabled = !modbus_enabled;
    Log("Modbus %s ", modbus_enabled ? "ON":"OFF");
    server.send(200, "text/plain", String(modbus_enabled));
  });

  server.on("/blynk_en", HTTP_GET, []{
    Log("HTTP_GET /blynk_en");
    blynk_enabled = !blynk_enabled;
    Blynk_Ini();
    Log("Blynk %s ", blynk_enabled ? "ON":"OFF");
    server.send(200, "text/plain", String(blynk_enabled));
  });

  server.on("/EP_data", HTTP_GET, []{
    Log("HTTP_GET /EP_data");

    sjson = "{";
    sjson += jsr( "TimeStr", EP.TimeStr );
    sjson += "," + jsr( "PV_volt", EP.PV_volt);
    sjson += "," + jsr( "PV_cur", EP.PV_cur);
    sjson += "," + jsr( "PV_pw", EP.PV_pw);
    sjson += "," + jsr( "BAT_volt", EP.BAT_volt);
    sjson += "," + jsr( "BAT_cur", EP.BAT_cur);
    sjson += "," + jsr( "BAT_pw", EP.BAT_pw);
    sjson += "," + jsr( "BAT_SOC", EP.BAT_SOC);
    sjson += "," + jsr( "LD_volt", EP.LD_volt);
    sjson += "," + jsr( "LD_cur", EP.LD_cur);
    sjson += "," + jsr( "LD_pw", EP.LD_pw);
    sjson += "," + jsr( "T_bat", EP.T_bat);
    sjson += "," + jsr( "T_inside", EP.T_inside);
    sjson += "," + jsr( "LD_ST", EP.LD_ST );
    ////////////////
    sjson += "," + jsr( "batstatus", batstatus_str(EP.batstatus) );
    sjson += "," + jsr( "sysstatus", sysstatus_str(EP.sysstatus) );
    sjson += "," + jsr( "ldstatus", EP.ldstatus );
    sjson += "," + jsr( "pv_max_volt_today", EP.pv_max_volt_today);
    sjson += "," + jsr( "pv_min_volt_today", EP.pv_min_volt_today);
    sjson += "," + jsr( "bat_max_volt_today", EP.bat_max_volt_today);
    sjson += "," + jsr( "bat_min_volt_today", EP.bat_min_volt_today);
    sjson += "," + jsr( "conEnT", EP.conEnT);sjson += "," + jsr( "conEnM", EP.conEnM);sjson += "," + jsr( "conEnY", EP.conEnY);sjson += "," + jsr( "conEnTot", EP.conEnTot);
    sjson += "," + jsr( "genEnT", EP.genEnT);sjson += "," + jsr( "genEnM", EP.genEnM);sjson += "," + jsr( "genEnY", EP.genEnY);sjson += "," + jsr( "genEnTot", EP.genEnTot);
    //////////////////////
    sjson += "}"; 
     
    server.send(200, "application/json", sjson.c_str());
  });  

  server.on("/EP_LD", HTTP_GET, []{
    Log("HTTP_GET /EP_LD");
    EP_LD_SET(!EP.LD_ST);
    Log("EP DC LOAD %s ", EP.LD_ST ? "ON":"OFF");
    server.send(200, "text/plain", String(EP.LD_ST));
  });
  
  server.on("/reboot", HTTP_GET, []{
    Log("HTTP_GET /reboot");
    server.send(200, "text/plain", "OK");
    Log("REBOOT");
    //ESP.restart();//Soft
    ESP.reset();//Hard
  });

  server.on("/pdata", HTTP_GET, []{ //Processed Data
    Log("HTTP_GET /pdata");

    sjson = "{";
    sjson += jsr( "TimeStr", PD.TimeStr );
    sjson += "," + jsr( "temp", PD.Temp );
    sjson += "," + jsr( "hum", PD.Hum );
    sjson += "," + jsr( "V", PD.V );
    
    sjson += "," + jsr( "P_PV", PD.P_PV );
    sjson += "," + jsr( "P_DLD", PD.P_DLD );
    sjson += "," + jsr( "P_DGEN", PD.P_DGEN );
    sjson += "," + jsr( "P_BANK", PD.P_BANK );
    sjson += "," + jsr( "P_H", PD.P_H );
    sjson += "," + jsr( "P_B1", PD.P_B1 );
    sjson += "," + jsr( "P_B2", PD.P_B2 );
    sjson += "," + jsr( "P_B3", PD.P_B3 );
    sjson += "," + jsr( "P_B4", PD.P_B4 );
    sjson += "," + jsr( "P_B5", PD.P_B5 );

    sjson += "," + jsr( "E_PV", PD.E_PV,3 );
    sjson += "," + jsr( "E_DLD", PD.E_DLD,3 );
    sjson += "," + jsr( "E_DGEN", PD.E_DGEN,3 );
    sjson += "," + jsr( "E_BANK", PD.E_BANK,3 );
    sjson += "," + jsr( "E_H", PD.E_H,3 );
    sjson += "," + jsr( "E_B1", PD.E_B1,3 );
    sjson += "," + jsr( "E_B2", PD.E_B2,3 );
    sjson += "," + jsr( "E_B3", PD.E_B3,3 );
    sjson += "," + jsr( "E_B4", PD.E_B4,3 );
    sjson += "," + jsr( "E_B5", PD.E_B5,3 );
    sjson += "}";    
    
    server.send(200, "application/json", sjson.c_str());
  });

  server.on("/pdata_reset", HTTP_GET, []{
    Log("HTTP_GET /pdata_reset");
    proData_Ini();
    server.send(200, "text/plain", "OK");
  });

  server.on("/hist_data", HTTP_GET, []{
    t_start(1);
    int ix = server.hasArg("ix") ? (server.arg("ix")).toInt() : -1; 
    
    Log("HTTP_GET /hist_data ? ix = %u", ix);
   
    sjson="[";
    String aux;
    for (int i = 0; i<hist_n ; i++)  {

      Hist_Decode(i);
      
      switch (ix) { //Chart order
        case 0: aux = HD.Temp; break;
        case 1: aux = HD.Hum; break;
        case 2: aux = HD.P_PV; break;
        case 3: aux = HD.P_DLD; break;
        case 4: aux = HD.P_H; break; 
        case 5: aux = HD.P_BANK; break;
        case 6: aux = HD.P_B1; break;
        case 7: aux = HD.P_B2; break;
        case 8: aux = HD.P_B3; break;
        case 9: aux = HD.P_B4; break;
        case 10: aux = HD.P_B5; break;
        case 11: aux = HD.V; break;
        default: aux = '"'+ NTP.getTimeStr(HD.T) +'"'; //Send Time
      }
     
      sjson+= aux; 
      if (i < hist_n-1) sjson+=",";
    }
    sjson+="]";
    
    server.send(200, "application/json", sjson.c_str());   
    t_end(1);
  });

  server.begin();
}

////////////////////
// BLYNK
/////////////////////////
#define BLYNK_PRINT Serial

const char auth[]= "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";//Blynk Authorization Code

unsigned long blynk_last_try;

void Blynk_Ini() {
   t_start(5);  
   if (blynk_enabled && WIFI_OK) {
       Blynk.config(auth); //Blynk.begin(auth, WIFI_SSID, WIFI_PWD);
       //Blynk.config(auth, "owndomain", 18080 );//Local Blynk Server 
       Blynk.connect(1000);//3 secs
       blynk_last_try = millis();
   } else {
     Blynk.disconnect();
   }
   blynk_OK = Blynk.connected();
   t_end(5); 
}

void Blynk_Send() { //Avoid more than 10 values/sec
  static int s;
  if (!blynk_enabled || !blynk_OK ) return;
  t_start(6); 
  ESP.wdtFeed();
  if (s==0) {
    Blynk.virtualWrite(V0, PD.P_PV);
    Blynk.virtualWrite(V1, PD.P_DLD);
    Blynk.virtualWrite(V2, PD.P_H);
    Blynk.virtualWrite(V3, PD.P_BANK);
    Blynk.virtualWrite(V4, PD.V);
  } else if (s==1) {
    Blynk.virtualWrite(V5, PD.Temp);
    Blynk.virtualWrite(V6, PD.Hum);
    Blynk.virtualWrite(V7, EP.BAT_SOC);
    Blynk.virtualWrite(V50, relay_ST);
    Blynk.virtualWrite(V51, EP.LD_ST);
  }
  s++;if (s>1) s=0;
  t_end(6);
}

BLYNK_WRITE(V50) {//Button RELAY OFF/ON
  relay_ST = !digitalRead(rPin);
  digitalWrite(rPin, relay_ST);
  Blynk_Send();
}

BLYNK_WRITE(V51) {//Button DC LOAD OFF/ON
  EP_LD_SET(!EP.LD_ST);
  Blynk_Send();
}

////////////////////
// THINGSPEAK
/////////////////////////
#include <ESP8266HTTPClient.h>
const char * api_url = "http://api.thingspeak.com/update?api_key=XXXXXXXXXXXXXXXX"; //Thingspeak API Key to Write
String url;

void thingspeak_Hist_Send() {
   if (hist_n <= hist_last_sent) return;
   int i = hist_last_sent; 
   Hist_Decode(i);
   url = api_url + String("&timezone=Europe/Brussels&created_at=")+ HD.TimeStr;//With TimeStamp . Tested 2005-12-12%2001%3A02%3A03
   url += "&field1=" + String(HD.Temp) + "&field2=" + String(HD.Hum) + "&field3=" + String(HD.P_PV) + "&field4=" + String(HD.P_DLD) + "&field5=" + String(HD.P_H) \
    + "&field6=" + String(HD.P_BANK) + "&field7=" + String(HD.V);
  _thingspeak_Send();
  if (thingspeak_OK) hist_last_sent++;
  url="";
}
  
void _thingspeak_Send() {
  thingspeak_OK = false;
  if (!thingspeak_enabled || !WIFI_OK) return;
  t_start(7);
  Log("Thingspeak GET: %s", url.c_str());
  ESP.wdtFeed();
  HTTPClient http;
  http.setTimeout(2000);
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    thingspeak_OK = true;
    String payload = http.getString();
    Log("Thingspeak httpCode: %u payload: %s", httpCode, payload.c_str());
  } else {
    Log("Thingspeak error: %s",http.errorToString(httpCode).c_str());
  }
  http.end();
  t_end(7);
}

//////////////////////////
//SETUP
///////////////

void setup() {
  
  pinMode(rPin, OUTPUT);//relay

  Serial.begin(115200);

  Display_Ini();

  WIFI_Ini();//WIFI & NTP Ini. Final delay 2000 ms

  RemoteDebug_Ini();//Telnet Logs
  
  OTA_Ini();//Upload through WIFI
  
  Modbus_Ini();
  timer.setInterval(MB_TIME, getModbus_fast);
  timer.setInterval(MB_TIME*6, getModbus_slow);

  ADS_Ini();//Analog Inputs init.
  timer.setInterval(200, getADS);//Read every x ms

  DSTemp_Ini();//DS temp. sensor init. 
  timer.setInterval(5000, getDSTemp);//Read every x ms

  DHT_Ini();//DHT Sensor init.
  timer.setInterval(5000, getDHT);//Read DHT Temp. and Hum. every x ms

  proData_Ini();//Processed Data
  timer.setInterval(1000, proData);//Calculate Power and Energy Balance
  
  timer.setInterval(500, Display_D);//Update values in Display every x ms
  timer.setInterval(3000, Display_N);//Switch info screen every x ms

  Hist_Ini();//Historical data init.
  timer.setInterval(HIST_PER*60*1000, Hist_Add);//Update values every 5 min

  timer.setInterval(30000, thingspeak_Hist_Send);//Try to upload local History to Thingspeak every 30 secs 

  timer.setInterval(2000, Blynk_Send);//Blynk Upload Data
  
  WEB_Ini();//Web Server init.

  delay(1000);

}

///////////////////////////////////////////
// LOOP
///////////////////////////////////////////

void loop() {
  t_start(0);

  ESP.wdtDisable();
  ESP.wdtFeed();
     
  yield();//continues ESP Processes

  WIFI_OK = (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0,0,0,0));
  blynk_OK = Blynk.connected();
  if (WIFI_OK) {

    ArduinoOTA.handle();// Don't stop this process with Delays or Whiles 
    server.handleClient();
    if (!NTP_i) NTP_Ini();//Initialize NTP 
    if (blynk_OK) Blynk.run(); else if (millis() - blynk_last_try > 60000) Blynk_Ini();
    
  } else {
    if (WIFI_OK_last) { WIFI_DisCounter++; Log("WIFI disconnection (%u)", WIFI_DisCounter); }
    //WiFi.reconnect();delay(1000);
  }
  WIFI_OK_last = WIFI_OK;
  
  RemoteDebug_Handle();
  timer.run();//MAIN processes

  t_end(0);
}
