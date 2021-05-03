#include <DallasTemperature.h>
#include "DHTesp.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>


//#define RELEASE 1
#define DEBUG 1

#ifdef DEBUG
  #define     WIFI              1
  #define     TRACE             1
  #define     FEATURE_LED       1
#else // RELEASE
  #define     WIFI              1
  #define     TRACE             0
  #define     FEATURE_LED       0
#endif // RELEASE/DEBUG

#define LOGLN(...) \
  Serial.println(__VA_ARGS__)
#define DEBUG_LOG(...) \
  if(DEBUG) Serial.print(__VA_ARGS__)
#define DEBUG_LOGLN(...) \
  if(DEBUG) Serial.println(__VA_ARGS__)
#define DEBUG_LOG_PRINTF(...) \
  if(DEBUG) Serial.printf(__VA_ARGS__)

#define SETUP_INIT()            \
  if(DEBUG) Serial.begin(9600); \
  DEBUG_LOG_PRINTF("program: %s @ [%s]", __FILE__, __DATE__);

#define     HOST              "HOST"
#define     PORT              "PORT"
#define     BUF_SIZE          5*10+4
#define     ONE_WIRE_BUS      4
#define     HOME_PROTOCOL_VER 0.1
#define     LED_ON            LOW
#define     LED_OFF           HIGH
#if FEATURE_LED
  #define     TURN_LED_ON       digitalWrite(LED_BUILTIN, LED_ON);
  #define     TURN_LED_OFF      digitalWrite(LED_BUILTIN, LED_OFF);
#else  // FEATURE_LED
  #define     TURN_LED_ON       do {} while(0);
  #define     TURN_LED_OFF      digitalWrite(LED_BUILTIN, LED_OFF);
#endif // FEATURE_LED

const int   DHTPin     = 14;            // D5
const char* ssid       = "SSID";
const char* password   = "PASSWORD";

DHTesp dht;
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];
static char buf[BUF_SIZE];
String macAddr;
WiFiClient client;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;


void setup_wifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    TURN_LED_ON
    delay(500);
    DEBUG_LOG(".");
    TURN_LED_OFF
    delay(100);
  }
  TURN_LED_OFF
}

void print_wifi()
{
  macAddr = WiFi.macAddress();
  DEBUG_LOGLN("");
  DEBUG_LOGLN("WiFi connected");
  DEBUG_LOGLN("IP address: ");
  DEBUG_LOGLN(WiFi.localIP());
  DEBUG_LOGLN("MAC address: ");
  DEBUG_LOGLN(macAddr);
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) DEBUG_LOG("0");
      DEBUG_LOG(deviceAddress[i], HEX);
  }
}

float get_onewire()
{
  float tempC = -1;
  if(sensors.getAddress(tempDeviceAddress, 0))
  {
    DEBUG_LOG("Found device with address: ");
    printAddress(tempDeviceAddress);
    DEBUG_LOGLN();

    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(0);
    if(tempC != DEVICE_DISCONNECTED_C) 
    {
      DEBUG_LOG("Temperature for 1-wire is: ");
      DEBUG_LOGLN(tempC);
    }
    else
    {
      DEBUG_LOGLN("Error: Could not read temperature data");
    }
  }
  return tempC;
}

void print_onewire()
{
  if(sensors.getAddress(tempDeviceAddress, 0))
  {
    DEBUG_LOG("Found device with address: ");
    printAddress(tempDeviceAddress);
    DEBUG_LOGLN();
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    if(tempC != DEVICE_DISCONNECTED_C) 
    {
      DEBUG_LOG("Temperature for 1-wire is: ");
      DEBUG_LOGLN(tempC);
    }
    else
    {
      DEBUG_LOGLN("Error: Could not read temperature data");
    }
  }
  else
  {
    DEBUG_LOGLN("no addr found");
  }
}

void setup_onewire() {
  sensors.begin();
}

void setup_dht() {
  dht.setup(DHTPin, DHTesp::DHT11);
  DEBUG_LOGLN("Status\tHumidity (%)\tTempDHT (C) \t Temp1WIRE (C) \t TempDHT (F) \t TEMP1WIRE (F)");
  TURN_LED_ON
  delay(dht.getMinimumSamplingPeriod());

  TURN_LED_OFF
  DEBUG_LOGLN("DHT Ready");
}

char* get_temp_and_humid() {
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  float temperature2 = get_onewire();
  snprintf(buf, BUF_SIZE, "%3.1f,%f,%f,%f,%f,%f",
    HOME_PROTOCOL_VER, humidity,
    temperature, temperature2,
    dht.toFahrenheit(temperature), dht.toFahrenheit(temperature2));
  DEBUG_LOGLN(buf);

  return buf;
}

void setup()
{
  SETUP_INIT()
  pinMode(LED_BUILTIN, OUTPUT);

#if WIFI
  WiFi.mode(WIFI_STA);
#endif // WIFI
  TURN_LED_ON

#if WIFI
  setup_wifi();
  print_wifi();
#endif // WIFI
  setup_dht();
  TURN_LED_OFF
}

void loop()
{
  DEBUG_LOGLN("loop begin0");
  TURN_LED_ON

#if WIFI
  HTTPClient http;

  String connect_str = "http://" HOST ":" PORT "/" + macAddr;
  if (!http.begin(client, connect_str)) {
    DEBUG_LOGLN("connection failed");
    delay(5000);
    return;
  }

  http.addHeader("Content-Type", "text/plain");
  DEBUG_LOGLN("[HTTP] POST");
  int httpCode = http.POST(get_temp_and_humid());

  if (httpCode > 0) {
    DEBUG_LOG_PRINTF("[HTTP] POST... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      const String& payload = http.getString();
      DEBUG_LOGLN("received payload:\n<<");
      DEBUG_LOGLN(payload);
      DEBUG_LOGLN(">>");
    }
  } else {
    DEBUG_LOG_PRINTF("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
#else // WIFI
  DEBUG_LOGLN(get_temp_and_humid());
#endif // WIFI

  print_onewire();
  TURN_LED_OFF

#if WIFI
  delay(300000);  //Send a request every 5 minutes
#else // WIFI
  delay(3000);
#endif // WIFI
}
