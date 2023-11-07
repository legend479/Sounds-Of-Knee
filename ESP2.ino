
// #define IN_pin 14
// void setup() {
//   pinMode(IN_pin,INPUT);
//   Serial.begin(9600);
// }

// void loop() {
//   Serial.print("Output :");
//   Serial.println(analogRead(IN_pin));
//   delay(1000);
// }

// // Include necessary libraries
// #include <Arduino.h>

// // Define the analog pin where the flex sensor is connected
// const int flexSensorPin = 12;

// void setup() {
//   // Initialize serial communication for debugging
//   Serial.begin(115200);

//   // Set the flex sensor pin as an input
//   pinMode(flexSensorPin, INPUT);
// }

// void loop() {
//   // Read the analog voltage from the flex sensor
//   int flexSensorValue = analogRead(flexSensorPin);

//   // Print the raw sensor value to the serial monitor
//   Serial.print("Flex Sensor Value: ");
//   Serial.println(flexSensorValue);

//   // You can add code here to process the sensor value as needed
  
//   // Add a delay to prevent rapid readings
//   delay(1000); // 1 second delay
// }


#include <HTTPClient.h>
#include <Wire.h>
#include "max32664.h"
// #include <stdlib.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ThingSpeak.h>

#define mqttport 1883

char ssid[] = "iPhone_atp";
char password[] = "1 2 3 4 5 6 7";
const char* server = "mqtt3.thingspeak.com";
char mqttUserName[] = "FCoPExU9MAUQISkGCBEDCz0";
char mqttClientID[] = "FCoPExU9MAUQISkGCBEDCz0";
char mqttpass[] = "OwEpA4RuSjFFfZNzg6fGoBXa";
int writeChannelID = 2151521;
char writeAPIKey[] = "XVEHO8NYR0DRP5KQ";


WiFiClient client;
PubSubClient mqttClient(server, mqttport, client);

#define CSE_IP "192.168.207.133"
#define CSE_PORT 5089
#define OM2M_ORGIN "admin:admin"
#define OM2M_MN "/~/in-cse/in-name/"
#define OM2M_AE "AE-TEST"
#define OM2M_DATA_CONT "User-Patterns"

#define RESET_PIN 4
#define MFIO_PIN 5
// SCL 22
// SDA 21
#define RAWDATA_BUFFLEN 250

max32664 MAX32664(RESET_PIN, MFIO_PIN, RAWDATA_BUFFLEN);

HTTPClient http;

// int i = 0;

void mfioInterruptHndlr(){
  //Serial.println("i");
}

void enableInterruptPin(){

  //pinMode(mfioPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MAX32664.mfioPin), mfioInterruptHndlr, FALLING);

}

void loadAlgomodeParameters(){

  algomodeInitialiser algoParameters;
  /*  Replace the predefined values with the calibration values taken with a reference spo2 device in a controlled environt.
      Please have a look here for more information, https://pdfserv.maximintegrated.com/en/an/an6921-measuring-blood-pressure-MAX32664D.pdf
      https://github.com/Protocentral/protocentral-pulse-express/blob/master/docs/SpO2-Measurement-Maxim-MAX32664-Sensor-Hub.pdf
  */

  algoParameters.calibValSys[0] = 120;
  algoParameters.calibValSys[1] = 122;
  algoParameters.calibValSys[2] = 125;

  algoParameters.calibValDia[0] = 80;
  algoParameters.calibValDia[1] = 81;
  algoParameters.calibValDia[2] = 82;

  algoParameters.spo2CalibCoefA = 1.5958422;
  algoParameters.spo2CalibCoefB = -34.659664;
  algoParameters.spo2CalibCoefC = 112.68987;

  MAX32664.loadAlgorithmParameters(&algoParameters);
}


#define flexSensorpin 32
#define sound_pin 33
#define piezoelectric_sensor 34
#define emg_pin 35

void setup(){

  Serial.begin(115200);
  pinMode(flexSensorpin,INPUT);
  pinMode(sound_pin,INPUT);
  pinMode(piezoelectric_sensor,INPUT);
  pinMode(emg_pin,INPUT);  
  WiFi.begin(ssid ,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(server, mqttport);

  Wire.begin();

  loadAlgomodeParameters();

  int result = MAX32664.hubBegin();
  if (result == CMD_SUCCESS){
    Serial.println("Sensorhub begin!");
  }else{
    //stay here.
    while(1){
      Serial.println("Could not communicate with the sensor! please make proper connections");
      delay(5000);
    }
  }

  bool ret = MAX32664.startBPTcalibration();
  while(!ret){

    delay(10000);
    Serial.println("failed calib, please retsart");
    //ret = MAX32664.startBPTcalibration();
  }

  delay(1000);

  //Serial.println("start in estimation mode");
  ret = MAX32664.configAlgoInEstimationMode();
  while(!ret){

    //Serial.println("failed est mode");
    ret = MAX32664.configAlgoInEstimationMode();
    delay(10000);
  }

  //MAX32664.enableInterruptPin();
  Serial.println("Getting the device ready..");
  delay(1000);
}

//MQTT

void mqttPublish(long writeChannelID, char* pubWriteAPIKey,String dataString){
  // int index = 0;
  // String dataString = "";

  // while(fieldArray[index] == 1){
  // dataString = "field1="+String(hr)+"&field2="+String(systole)+"&field3="+String(diastole)+"&field4="+String(spo2);
  // index++;
  // Serial.println(dataString);
  String topicString = "channels/" + String( writeChannelID ) + "/publish";
  // Serial.println(topicString);
  mqttClient.publish(topicString.c_str(), dataString.c_str());
  // Serial.println(writeChannelID);
}
void publish_to_om2m(String data,String label)
{
    //   String label = "Label";
    // String data;
    String server = "http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN + String() + OM2M_AE + "/" + String() + OM2M_DATA_CONT + "/";
    Serial.println(server);
    
    if (label=="Label-1")
      server += "Flex_Sensor"
    else if(label=="Label-2")
      server += "Health_Sensor"
    else if(label=="Label-3")
      server += "Peizo_Sensor"
    else if(label=="Label-4")
      server += "Microphone"
    else if(label=="Label-5")
      server += "EMG_Sensor"

    http.begin(server);    

    http.addHeader("X-M2M-Origin", OM2M_ORGIN);
    http.addHeader("Content-Type", "application/json;ty=4");
    // http.addHeader("Content-Length", "100");

    // data = "[" + String(systole) + ", " + String(diastole) + ", " + String(hr) + ", " + String(spo2) + "]"; 
    String req_data = String() + "{\"m2m:cin\": {"

      +
      "\"con\": \"" + data + "\","

      +
      "\"lbl\": \"" + label + "\","

      // + "\"rn\": \"" + "Entry "+String(i++) + "\","

      // +
      // "\"cnf\": \"text\""

      +
      "}}";
    int code = http.POST(req_data);
    http.end();
    // Serial.println(code);
}
void loop(){

//   while (!mqttClient.connected()){
//     if(mqttClient.connect(mqttClientID, mqttUserName, mqttpass)){
//       Serial.print("MQTT to ");
//       Serial.print( server);
//       Serial.print(" at port ");
//       Serial.print( mqttport );
//       Serial.println(" successful.");
//     }
//     else {
//       Serial.print("MQTT connection failed, rc = ");
//       Serial.print( mqttClient.state() );
//       Serial.println(" Will try again in a second.");
//       delay(1000);
//     }
//   }

//   mqttClient.loop();

  uint8_t num_samples = MAX32664.readSamples();

  if(num_samples){

    Serial.print("sys = ");
    float systole = MAX32664.max32664Output.sys;
    // Serial.print(MAX32664.max32664Output.sys);
    Serial.print(systole);

    Serial.print(", dia = ");
    float diastole = MAX32664.max32664Output.dia;
    // Serial.print(MAX32664.max32664Output.dia);
    Serial.print(diastole);

    Serial.print(", hr = ");
    float hr = MAX32664.max32664Output.hr;
    // Serial.print(MAX32664.max32664Output.hr);
    Serial.print(hr);

    Serial.print(" spo2 = ");
    float spo2 = MAX32664.max32664Output.spo2;
    // Serial.println(MAX32664.max32664Output.spo2);
    Serial.println(spo2);

//     //MQTT
//     mqttPublish(writeChannelID, writeAPIKey);

//     // OM2M

//     // String label = "Label";
//     String data;
//     // String server = "http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN;

//     // http.begin(server + String() + OM2M_AE + "/" + OM2M_DATA_CONT + "/");

//     // http.addHeader("X-M2M-Origin", OM2M_ORGIN);
//     // http.addHeader("Content-Type", "application/json;ty=4");
//     // http.addHeader("Content-Length", "100");

    data = "[" + String(systole) + ", " + String(diastole) + ", " + String(hr) + ", " + String(spo2) + "]"; 
//    // String req_data = String() + "{\"m2m:cin\": {"

//     //   +
//     //   "\"con\": \"" + data + "\","

//     //   +
//     //   "\"lbl\": \"" + label + "\","

//     //   // + "\"rn\": \"" + "Entry "+String(i++) + "\","

//     //   // +
//     //   // "\"cnf\": \"text\""

//     //   +
//     //   "}}";
//     // int code = http.POST(req_data);
//     // http.end();
//     // Serial.println(code);
  }
float VCC = 4.98; // Measured voltage of Ardunio 5V line
float R_DIV = 47500.0; // Measured resistance of 3.3k resistor

// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
float STRAIGHT_RESISTANCE = 37300.0; // resistance when straight
float BEND_RESISTANCE = 90000.0; // resistance at 90 deg
int flexADC = analogRead(flexSensorpin);
float flexV = flexADC * VCC / 1023.0;
float flexR = R_DIV * (VCC / flexV - 1.0);
// Serial.println("Resistance: " + String(flexR) + " ohms");

// Use the calculated resistance to estimate the sensor's
// bend angle:
float angle = map(flexR, STRAIGHT_RESISTANCE, BEND_RESISTANCE, 0, 90.0);
Serial.println("Bend: " + String(angle) + " degrees");
Serial.println();

int flex_sensor_value=analogRead(flexSensorpin);
Serial.print("Flex Sensor value: ");
Serial.println(angle);
Serial.println(flex_sensor_value);


int sound_value=analogRead(sound_pin);
Serial.print("Electronic Microphone Value: ");
Serial.println(sound_value);

int piezo_value=analogRead(piezoelectric_sensor);
Serial.print("Piezoelectricc Sensor value");
Serial.println(piezo_value);

int emg_value=analogRead(emg_pin);
Serial.print("emg value: ");
Serial.println(emg_value);

String emg_to_publish=String(emg_value);
String piezo_publish=String(piezo_value);
String sound_publish=String(sound_value);
String flex_to_publish=String(flex_sensor_value);

publish_to_om2m(data,"Label-2");
publish_to_om2m(emg_to_publish,"Label-5");
publish_to_om2m(piezo_publish,"Label-3");
publish_to_om2m(sound_publish,"Label-4");
publish_to_om2m(flex_to_publish,"Label-1");
// String dataString = "field1="+String(hr)+"&field2="+String(systole)+"&field3="+String(diastole)+"&field4="+String(spo2);
// void mqttPublish(long writeChannelID, char* pubWriteAPIKey,String dataString)
  delay(100);
}
