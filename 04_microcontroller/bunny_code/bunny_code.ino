const int B1A = D0;  // define pin D0 for B-1A (PWM Speed)
const int B1B = D1;  // define pin D1 for B-1B (direction)

// setting PWM properties
const int freq = 5000;
const int resolution = 8;

bool waiting = true;

void setup() {
  pinMode(B1A, OUTPUT); // specify these pins as outputs
  pinMode(B1B, OUTPUT);

  ledcAttach(B1A, freq, resolution);
  ledcWrite(B1A, 0);   // start with the motors off 

  //I ALWAYS WANT B1A TO BE HIGH. DIRECTION WILL NEVER CHANGE (using snail cams)
  digitalWrite(B1A, HIGH);
  
  Serial.begin(9600); 
  delay(2000);  
}
void loop(){
  //Prompt user to present the bunny with something (i.e. type something)
  Serial.println("Present something to the bunny!");
  while(waiting){
    if(Serial.available()){
      String input = Serial.readStringUntil('\n');
      excitedBunny(input);
      waiting = false;
    }
  }
  delay(2000);
  Serial.println();
  waiting = true;
}

//function that reads text serial inputs to control motor 
//input: text in serial monitor
//outputs: speed and text response to user

void excitedBunny(String text){
  if(text.equals("carrots")){
    Serial.println();
    Serial.println("Yummy!");
    ledcWrite (B1A, 255); //full speed because bunny so excited!
    delay(3000); //let it run for 3 seconds
    ledcWrite(B1A, 0); //turn off 
  }
  else if(text.equals("lettuce")){
    Serial.println();
    Serial.println("eh okay!");
    ledcWrite (B1A, 200); //slower speed
    delay(1500); //let it run for 3 seconds
    ledcWrite(B1A, 0); //turn off 
  }
  else{
    Serial.println();
    Serial.println("The bunny is not excited by this >:(");
    Serial.println();
  }
}

