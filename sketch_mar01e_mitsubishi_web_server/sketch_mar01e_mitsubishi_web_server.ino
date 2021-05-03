#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include <ir_Mitsubishi.h>

/*
 * API:
 *
 *  <IP>:<PORT>/do?
 *    temp=(16-31|60.4-88.2)
 *    [&temp_units=(f|F)]
 *    &power=(on|off)
 *    &mode=(1="Heat",2="Dry",3="Cool",4="Auto")
 *    &fan=(1-8)
 *    &swingh=(1-12)
 *    &swingv=(1-12)
 *
 * > iwr -uri "<IP>:<PORT>/do?temp=68&temp_units=f&mode=1&fan=7&swingh=7&swingv=12" -Method POST
 */


#define RELEASE
//#define DEBUG

#ifdef DEBUG
#define     WIFI              0
#define     TRACE             1
#else // RELEASE
#define     WIFI              1
#define     TRACE             0
#endif // RELEASE/DEBUG

#define LOGLN(...) \
  Serial.println(__VA_ARGS__)
#define DEBUG_LOG(...) \
  if(DEBUG) Serial.print(__VA_ARGS__)
#define DEBUG_LOGLN(...) \
  if(DEBUG) Serial.println(__VA_ARGS__)
#define DEBUG_LOG_PRINTF(...) \
  if(DEBUG) Serial.printf(__VA_ARGS__)

#define SETUP_INIT()              \
  if(DEBUG) Serial.begin(115200); \
  DEBUG_LOG_PRINTF("program: %s @ [%s]", __FILE__, __DATE__);


#define     FEATURE_LED       0

#define     PORT              PORT
#define     LED_ON            LOW
#define     LED_OFF           HIGH
#ifdef FEATURE_LED
#define     TURN_LED_ON       digitalWrite(LED_BUILTIN, LED_ON);
#define     TURN_LED_OFF      digitalWrite(LED_BUILTIN, LED_OFF);
#else  // FEATURE_LED
#define     TURN_LED_ON       __noop
#define     TURN_LED_OFF      __noop
#endif // FEATURE_LED

ESP8266WebServer server(PORT);
const uint16_t kIrLed = 4;           // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);               // Set the GPIO to be used to sending the message.
IRMitsubishiAC irMitsubishi(kIrLed); // Set the GPIO to be used to sending the message.

const char* ssid       = "SSID";
const char* password   = "PASSWORD";

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
  DEBUG_LOG("IP address: ");
  DEBUG_LOGLN(WiFi.localIP());  //Print the local IP
}

float convertFtoCRoundedToHalf(float tempF) {
  float tempC = (tempF-32)*.5556;
  return floor((tempC*2) + 0.5)/2;
}

void getAndSetTemperature() {
  float temp;

  // Assumes Celsius
  if (server.hasArg("temp")) {
    DEBUG_LOG("temp:");
    DEBUG_LOGLN(server.arg("temp"));
    temp = server.arg("temp").toFloat();
  }

  // Override if specified in Fahrenheit
  if (server.hasArg("temp_units")) {
    DEBUG_LOGLN("temp_units");
    DEBUG_LOGLN(server.arg("temp_units"));
    if((server.arg("temp_units") == "F" || server.arg("temp_units") == "f") &&
        server.hasArg("temp")) {
      DEBUG_LOGLN("temp conversion");
      DEBUG_LOGLN(convertFtoCRoundedToHalf(server.arg("temp").toFloat()));
      temp = convertFtoCRoundedToHalf(server.arg("temp").toFloat());
    }
  }

  if (temp >= 16 && temp <= 31) {
    irMitsubishi.setTemp(temp);
  } else {
    DEBUG_LOGLN("Warning: temperature ignored - outside of supported range");
  }
}

void handleDo() {
  // TODO: set state to common values
  /*
  /// Reset the state of the remote to a known good state/sequence.
  void IRMitsubishiAC::stateReset(void) {
  // The state of the IR remote in IR code form.
  // Known good state obtained from:
  //   https://github.com/r45635/HVAC-IR-Control/blob/master/HVAC_ESP8266/HVAC_ESP8266.ino#L108
  static const uint8_t kReset[kMitsubishiACStateLength] = {
      0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x08, 0x06, 0x30, 0x45, 0x67};
  setRaw(kReset);
  }*/
  TURN_LED_ON
  irMitsubishi.stateReset();

  getAndSetTemperature();

  if (server.hasArg("power")) {
    DEBUG_LOG("power:");
    DEBUG_LOGLN(server.arg("power"));
    if (server.arg("power") == "on") {
      irMitsubishi.on();
    } else if (server.arg("power") == "off") {
      irMitsubishi.off();
    }
  }

  if (server.hasArg("mode")) {
    DEBUG_LOG("mode:");
    DEBUG_LOGLN(server.arg("mode"));
    int mode = server.arg("mode").toInt();
    if (mode >= 1 && mode <= 4) {
      irMitsubishi.setMode(mode);
    } else {
      DEBUG_LOGLN("Warning: mode ignored - outside of supported range");
    }
  }

  if (server.hasArg("swingv")) {
    DEBUG_LOG("swingv:");
    int swingv = server.arg("swingv").toInt();
    DEBUG_LOGLN(swingv);
    if (swingv >= 1 && swingv <= 12) { // TODO: Confirm this
      irMitsubishi.setVane(swingv);
    } else {
      DEBUG_LOGLN("Warning: swingv ignored - outside of supported range");
    }
  }

  if (server.hasArg("swingh")) {
    DEBUG_LOG("swingh:");
    int swingh = server.arg("swingh").toInt();
    DEBUG_LOGLN(swingh);
    if (swingh >= 1 && swingh <= 12) { // TODO: Confirm this
      irMitsubishi.setWideVane(swingh);
    } else {
      DEBUG_LOGLN("Warning: swingh ignored - outside of supported range");
    }
  }

  if (server.hasArg("fan")) {
    DEBUG_LOG("fan:");
    int fanSpeed = server.arg("fan").toInt();
    DEBUG_LOGLN(fanSpeed);
    if (fanSpeed >= 1 && fanSpeed <= 8) { // TODO: Confirm this
      irMitsubishi.setFan(server.arg("fan").toInt());
    } else {
      DEBUG_LOGLN("Warning: fanSpeed ignored - outside of supported range");
    }
  }

  server.send(200, "text/plain", "thanks");
  DEBUG_LOGLN("Done processing Mitsubishi AC command.");
  irMitsubishi.send();
  DEBUG_LOGLN("Mitsubishi AC command sent.");
  TURN_LED_OFF
}

void setup_server() {
  server.on("/do", HTTP_POST, handleDo);
  server.begin();
  DEBUG_LOGLN("Server listening");
}

void setup() {
  pinMode(kIrLed, OUTPUT);
  SETUP_INIT()
  irsend.begin();
  irMitsubishi.calibrate();
  setup_wifi();
  setup_server();
}

void loop() {
  server.handleClient();
}
