#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

//LOGIC 
// temp increases AND altitude increases → thermal = true
// temp decreases AND altitude decreases → thermal = false
// altitude same → do nothing


// For temprature sensor
int ThermistorPin = A0;
int Vo;
float A = 3.354e-03;
float B = 2.5698e-4;
float R1 = 10000;
float T_old = 0 ;
bool debug_temp = false;

unsigned long lastThermoTime = 0;
const unsigned long thermoInterval = 5000; // 5 seconds

Adafruit_BMP280 bmp;

float altitude_old = 0;
bool in_thermal = false;

float altitudeThreshold = 0.15;; // meters, avoids tiny sensor noise

bool bmpFound = false;
bool debug_bmp = false;

unsigned long lastBMPTime = 0;
const unsigned long bmpInterval = 5000;


// for the servo
class ServoWrapper {
  public:
    Servo motor;
    int pin;

    ServoWrapper(int pin_input){
      pin = pin_input;
    }

    void attach(){
      motor.attach(pin);
    }

    void write(int angle){
      motor.write(angle);
    }
};

ServoWrapper stabServo(D5);
ServoWrapper rudderServo(D6);

// For Magnato code
int hallSensor = A1;
int magno;
unsigned long passTimes[3] = {0,0,0};
bool magnetDetected = false;
bool debug_magno = false;

unsigned long lastMagnoTime = 0;
const unsigned long magnoInterval = 100;    // 100 ms

void setup() {
  Serial.begin(9600);

  stabServo.attach();
  stabServo.write(27.5);   // starting position, for stab, should be 7.5

  rudderServo.attach();
  rudderServo.write(0);   // starting position
  
  Wire.begin(D9, D8); // SDI = D9, SCK = D8
  bmp.begin(0x77);

}
void measureThermo() {

  float T, R2;

  Vo = analogRead(ThermistorPin);
  R2 = R1 * ((4095.0 / (float)Vo) - 1.0);
  T = (1.0 / (A + B*log(R2/R1) ));  // Calculate temperature using datasheet formula.
  T = T - 273.15;               //Convert from Kelvin to Celcius.

   if(debug_temp){
      Serial.print("T_old: ");
      Serial.print(T_old);
      Serial.print("Current temp: ");
      Serial.println(T);


    T_old = T;
    return; 
  }
  float altitude = bmp.readAltitude(1013.25);

  // First time of the rus, set T_old to the current temprature
  if (T_old == 0 || altitude_old == 0){
    T_old = T;
    altitude_old = altitude;
    return;
  }

  float tempChange = T - T_old;
  float altitudeChange = altitude - altitude_old;

  if (altitudeChange > altitudeThreshold && tempChange < -5){  
    in_thermal = true;
    Serial.println("thermal detected");
    stabServo.write(0); //move 15 from 20
    rudderServo.write(30); 
  }

  else if (altitudeChange < -altitudeThreshold && tempChange > 5){  
    in_thermal = false;
    Serial.println("escaped thermal");
    stabServo.write(20); 
    rudderServo.write(0);
  }

  if(debug_temp){
    Serial.print("T_old: ");
    Serial.print(T_old);
    Serial.print(" Current temp: ");
    Serial.print(T);

    Serial.print(" altitude_old: ");
    Serial.print(altitude_old);
    Serial.print(" Current altitude: ");
    Serial.print(altitude);

    Serial.print(" in_thermal: ");
    Serial.println(in_thermal);
  }
  T_old = T;
  altitude_old = altitude;
}

void measureMagno() {

  int threshold = 1500;

  int sensorValue = analogRead(hallSensor);

  if(debug_magno){
    Serial.print("sensor value: ");
    Serial.println(sensorValue);
  }
    

  // detect magnet passing
  if(sensorValue < threshold && magnetDetected == false) {

    magnetDetected = true;
    // always update so that the most recent measurement is 2 and all the others cycle back
    passTimes[0] = passTimes[1];
    passTimes[1] = passTimes[2];
    passTimes[2] = millis();

    if(passTimes[0] != 0 && passTimes[1] != 0 && passTimes[2] != 0) {
      //find speeds btwn 1 & 2 and 2&3
      Serial.print(passTimes[0]);
      Serial.print(" ");
      Serial.print(passTimes[1] );
      Serial.print(" ");
      Serial.println(passTimes[2]);

      float dt1 = (passTimes[1] - passTimes[0]) / 1000.0;
      float dt2 = (passTimes[2] - passTimes[1]) / 1000.0;

      float speed1 = 1.0 / dt1;
      float speed2 = 1.0 / dt2;
      //avg the two speeds
      float avgSpeed = (speed1 + speed2) / 2.0;

      Serial.print("Rotations per second: ");
      Serial.println(avgSpeed);
    }
  }

  // reset trigger when magnet leaves
  if(sensorValue >= threshold) {
    magnetDetected = false;
  }
}

void debugBMP280(){

  Serial.print("BMP altitude: ");
  Serial.println(bmp.readAltitude(1013.25));

  Serial.println();
}

// This section is to read cmd from the terminal 
void CheckSerialCommadns(){
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    // Serial.println(cmd);

    if (cmd == "debug temp on") {
        debug_temp = true;
        Serial.println("Temperature debugging ON");
      }
      else if (cmd == "debug temp off") {
        debug_temp = false;
        Serial.println("Temperature debugging OFF");
      }
      else if (cmd == "debug_magno_on") {
        debug_magno = true;
        Serial.println("Magnet debugging ON");
      }

      else if (cmd == "debug_magno_off") {
        debug_magno = false;
        Serial.println("Magnet debugging OFF");
      }

      else if (cmd == "debug bmp") {
        debugBMP280();
        Serial.println("debug_bmp - scan I2C and print BMP280 readings");
      }

      else if (cmd == "help") {
        Serial.println("Available commands:");
        Serial.println("debug_temp_on  - enable temperature debugging");
        Serial.println("debug_temp_off - disable temperature debugging");
        Serial.println("debug_magno_on  - enable magnet sensor debugging");
        Serial.println("debug_magno_off - disable magnet sensor debugging");
        Serial.println("debug_bmp_on  - enable BMP280 debugging");
        Serial.println("debug_bmp_off - disable BMP280 debugging");
      }

    else if (cmd == "ms"){
      Serial.println("Moving servo");
      // int x = random(1, 181);
      stabServo.write(30);
      rudderServo.write(60);
    }

    else if (cmd == "thermal"){
      Serial.println("Simulating thermal detection");
      stabServo.write(0);//move 20
      rudderServo.write(30);//add 30
    }

    else if (cmd == "no thermal"){
      Serial.println("Simulating escaping thermal");
      stabServo.write(20);
      rudderServo.write(0);
    }
  }


}


void loop() {

  CheckSerialCommadns();
  
  if (millis() - lastThermoTime >= thermoInterval) {
    lastThermoTime = millis();
    measureThermo();
  }


  // if (millis() - lastMagnoTime >= magnoInterval) {
  //   lastMagnoTime = millis();
  //   measureMagno();
  // }
}