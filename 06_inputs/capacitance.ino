long result;   //variable for the result of the tx_rx measurement.
int analog_pin = D1;
int tx_pin = D2; //D6 is tx, D7 is rx

void setup() {
    pinMode(tx_pin, OUTPUT);      //Pin 4 provides the voltage step
    Serial.begin(9600);
}

void loop() {

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();


    if (cmd == "read") {
      int sum[] = {0, 0, 0, 0, 0};
      for(int i=0; i < 5; i++){
          sum[i]= tx_rx();
      }
       Serial.print("average: ");
      Serial.println((sum[1]+sum[2]+sum[3]+sum[4]+sum[0])/5);  
    }
  }
}


long tx_rx(){         // Function to execute rx_tx algorithm and return a value
                      // that depends on coupling of two electrodes.
                      // Value returned is a long integer.
  int read_high;
  int read_low;
  int diff;
  long int sum;
  int N_samples = 100;    // Number of samples to take.  Larger number slows it down, but reduces scatter.

  sum = 0;

  for (int i = 0; i < N_samples; i++){
   digitalWrite(tx_pin,HIGH);              // Step the voltage high on conductor 1.
   read_high = analogRead(analog_pin);     // Measure response of conductor 2.
   delayMicroseconds(1000);                 // Delay to reach steady state.
   digitalWrite(tx_pin,LOW);               // Step the voltage to zero on conductor 1.
   read_low = analogRead(analog_pin);      // Measure response of conductor 2.
   diff = read_high - read_low;            // desired answer is the difference between high and low.
   sum += diff;                            // Sums up N_samples of these measurements.
 }
  Serial.println(sum);
  return sum;
}     

