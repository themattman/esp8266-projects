#define DEBUG        1
#define DEBUG_LOG(...) \
  if(DEBUG) Serial.print(__VA_ARGS__)
#define DEBUG_LOGLN(...) \
  if(DEBUG) Serial.println(__VA_ARGS__)

#define SETUP_INIT()            \
  if(DEBUG) Serial.begin(9600); \
  DEBUG_LOG("program: "); \
  DEBUG_LOGLN(__FILE__); \
  DEBUG_LOGLN(__DATE__); \

// PM2.5 AQI Sensor
// OUT: A0
// VCC: Vin

int dustPin = 0;
float dustVal = 0; 
int ledPower = 4;
int delayTime = 280;
int delayTime2 = 40;
float offTime = 9680;

void setup(){
  SETUP_INIT()
  pinMode(dustPin, INPUT);
}
 
void loop(){
  delayMicroseconds(delayTime);
  dustVal = analogRead(dustPin); 
  delayMicroseconds(delayTime2);
  delayMicroseconds(offTime);
 
  delay(1000);
  DEBUG_LOGLN(dustVal);
}
