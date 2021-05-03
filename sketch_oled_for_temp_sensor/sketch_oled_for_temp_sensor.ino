#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <time.h>
#include <U8x8lib.h>

#define ONE_WIRE_BUS 2 // D4
#define TEMP_LENGTH  6
#define DEBUG        1
#define DEBUG_LOG(...) \
  if(DEBUG) Serial.print(__VA_ARGS__)
#define DEBUG_LOGLN(...) \
  if(DEBUG) Serial.println(__VA_ARGS__)
#define DEBUG_LOG_PRINTF(...) \
  if(DEBUG) Serial.printf(__VA_ARGS__)

#define SETUP_INIT()            \
  if(DEBUG) Serial.begin(115200); \
  DEBUG_LOG_PRINTF("program: %s @ [%s]", __FILE__, __DATE__);


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

int numberOfDevices;
DeviceAddress tempDeviceAddress;

U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

void setup_temp_sensor() {
  sensors.begin();
  DEBUG_LOG("Locating devices...");
  numberOfDevices = sensors.getDeviceCount();
  DEBUG_LOG("Found ");
  DEBUG_LOG(numberOfDevices, DEC);
  DEBUG_LOGLN(" devices.");
}

void setup(void) {
  SETUP_INIT()
  u8x8.begin();
  u8x8.setFlipMode(1);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  setup_temp_sensor();
}

void loop(void) {
  sensors.requestTemperatures();

  for(int i = 0; i < numberOfDevices; i++){
    if(sensors.getAddress(tempDeviceAddress, i)){
      DEBUG_LOG("Temperature for device: ");
      DEBUG_LOGLN(i, DEC);

      float tempC = sensors.getTempC(tempDeviceAddress);
      DEBUG_LOG("Temp C: ");
      DEBUG_LOG(tempC);
      DEBUG_LOG(" Temp F: ");
      DEBUG_LOGLN(DallasTemperature::toFahrenheit(tempC));
      char temp_c[TEMP_LENGTH];
      char temp_f[TEMP_LENGTH];
      DEBUG_LOGLN("------");

      sprintf(temp_c, "%.2f", tempC);
      sprintf(temp_f, "%.2f", DallasTemperature::toFahrenheit(tempC));

      DEBUG_LOG("Temp_C converted: [");
      DEBUG_LOG(temp_c);
      DEBUG_LOG("]\n");
      DEBUG_LOG("Temp_F converted: [");
      DEBUG_LOG(temp_f);
      DEBUG_LOG("]\n");
      DEBUG_LOGLN("------");

      int start_margin = 6;
      u8x8.draw2x2String(0,0,"C");
      u8x8.draw2x2String(2,0,":");

      for (int i = 0; i < TEMP_LENGTH; i++) {
        u8x8.draw2x2String(2*i+start_margin, 0, &temp_c[i]);
      }

      u8x8.draw2x2String(0,2,"F");
      u8x8.draw2x2String(2,2,":");

      for (int i = 0; i < TEMP_LENGTH; i++) {
        u8x8.draw2x2String(2*i+start_margin, 2, &temp_f[i]);
      }
    }
  }

  delay(200);
}
