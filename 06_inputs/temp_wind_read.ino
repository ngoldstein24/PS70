#include <ESP32Servo.h>

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

// for the servo
Servo servoMotor;
int servoPin = D6;

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
  
  servoMotor.attach(servoPin);
  servoMotor.write(100);   // starting position
}

void measureThermo() {

  float T, R2;

  Vo = analogRead(ThermistorPin);
  R2 = R1 * 1/(4095.0 / (float)Vo - 1.0);
  T = (1.0 / (A + B*log(R2/R1) ));  // Calculate temperature using datasheet formula.
  T = T - 273.15;               //Convert from Kelvin to Celcius.

  // First time of the rus, set T_old to the current temprature
  if (T_old == 0){
    T_old = T;
    return;
  }

  if (T - T_old > 2){  
    Serial.println("thermal detected");
    servoMotor.write(150);
  }

  if (T_old - T > 2){  
    Serial.println("escaped thermal");
    servoMotor.write(100);

  }

  if(debug_temp){
    Serial.print("T_old: ");
    Serial.print(T_old);
    Serial.print("Current temp: ");
    Serial.println(T);
  }

  T_old = T;
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


// This section is to read cmd from the terminal 
void CheckSerialCommadns(){
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    // Serial.println(cmd);

    if (cmd == "debug_temp_on") {
        debug_temp = true;
        Serial.println("Temperature debugging ON");
      }
      else if (cmd == "debug_temp_off") {
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

      else if (cmd == "help") {
        Serial.println("Available commands:");
        Serial.println("debug_temp_on  - enable temperature debugging");
        Serial.println("debug_temp_off - disable temperature debugging");
        Serial.println("debug_magno_on  - enable magnet sensor debugging");
        Serial.println("debug_magno_off - disable magnet sensor debugging");
      }

    else if (cmd == "ms"){
      Serial.println("Moving servo");
      int x = random(1, 256);
      servoMotor.write(x);
    }
  }


}

void loop() {

  CheckSerialCommadns();
  
  if (millis() - lastThermoTime >= thermoInterval) {
    lastThermoTime = millis();
    measureThermo();
  }

  if (millis() - lastMagnoTime >= magnoInterval) {
    lastMagnoTime = millis();
    measureMagno();
  }
}