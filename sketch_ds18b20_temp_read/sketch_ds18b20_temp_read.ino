#include <OneWire.h>
#include <DallasTemperature.h>


//#define RELEASE 1
#define DEBUG 1

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

#define ONE_WIRE_BUS 4 // D2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int numberOfDevices;
DeviceAddress tempDeviceAddress;


void setup(void)
{
  SETUP_INIT()

  // Start up the library
  sensors.begin();
  DEBUG_LOG("Locating devices...");
  numberOfDevices = sensors.getDeviceCount();
  DEBUG_LOG("Found ");
  DEBUG_LOG(numberOfDevices, DEC);
  DEBUG_LOGLN(" devices.");

  // Loop through each device, print out address
  for(int i = 0; i < numberOfDevices; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
      DEBUG_LOG("Found device ");
      DEBUG_LOG(i, DEC);
      DEBUG_LOG(" with address: ");
      printAddress(tempDeviceAddress);
      DEBUG_LOGLN();
    } else {
      DEBUG_LOG("Found ghost device at ");
      DEBUG_LOG(i, DEC);
      DEBUG_LOG(" but could not detect address. Check power and cabling");
    }
  }
}

void loop(void)
{
  sensors.requestTemperatures(); // Send the command to get temperatures

  // Loop through each device, print out temperature data
  for(int i = 0; i < numberOfDevices; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
      // Output the device ID
      DEBUG_LOG("Temperature for device: ");
      DEBUG_LOGLN(i, DEC);
      // Print the data
      float tempC = sensors.getTempC(tempDeviceAddress);
      DEBUG_LOG("Temp C: ");
      DEBUG_LOG(tempC);
      DEBUG_LOG(" Temp F: ");
      DEBUG_LOGLN(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
    }
  }

  delay(5000);
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) DEBUG_LOG("0");
      DEBUG_LOG(deviceAddress[i], HEX);
  }
}
