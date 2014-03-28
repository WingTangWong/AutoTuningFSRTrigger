/*

  Auto Tuning FSR Trigger
  Author: Wing Tang Wong
  GitHub: https://github.com/WingTangWong/AutoTuning-FSR-Trigger
 
  Wrote the code testing on an Arduino Mega/Uno set, but the intended destination is to 
  run it from an ATTINY85. 
  
  Inputs: ADC
  Output: Digital IO High/Low

*/


/*
  Okay, let's set this up so that depending on which board you have the Arduino IDE setup to, we'll have different pins setup.
*/

// Default pin values we will use
int analogIN   = A3; // Pin for the ADC input
int digitalOUT = 0; // Pin for the digital signal out  Use '13' if you are using UNO or MEGA boards to make use of the LED.



// IO pins
long noiseLEVEL = 15;
long threshold = 25;
long TIMEOUT = 500; // In milliseconds. 2000 = 2 seconds.
long ambient=0;
long reading=0;
long newambient=0;
long delta=0;
long range=0;
long sensorValue=0;
long sinceLastSettle=millis();
long settleTIMEOUT = 250; // Only force resettle every 250 ms

// Debugging info
bool debug = false;




/*

  settle()

  Basically, update ambient value until it is settled to some value where the variation of the value does not
  exceed a range of jitter

*/


void settle() {

  while ( true ) {

    reading = analogRead(analogIN) + analogRead(analogIN) + analogRead(analogIN);
    reading = int(reading / 3.000); // Make sure we're dealing with whole numbers.

    newambient = int( (ambient + reading) / 2.00 ); // Let's average that into the ambient value.

    delta = newambient - ambient;
    ambient = newambient;

    if ( delta < 0 ) {
      delta = delta * -1;
    };

  if ( debug ) {
    Serial.print("sensor = " );                       
    Serial.print(reading);
    Serial.print("\t ambient = ");
    Serial.print(ambient);
    Serial.print("\t range = ");
    Serial.print(range);
    Serial.print("\t delta = ");
    Serial.println(delta);
  };
  
    if ( delta <= noiseLEVEL ) {
       if ( debug ) { Serial.println("Settled!"); };
        break;
    };
  };
  sinceLastSettle=millis();
};


/*
  
  latchup()

  When a triggering event occurs, let's hold the signal high until either the pressure
  drops off or a timeout occurs.

  The timeout clause is for cases where someone has put something on the plate, changing
  the ambient pressure value to a higher one. 

*/

void latchup() {
  int newReading=0;
  long startTime = millis();

  // Going to set this high until system re-settles.
  digitalWrite( digitalOUT, HIGH );

  while( true ) {

    // Take a sample reading
    newReading = analogRead( analogIN );

    // Has the pressure returned to normal?
    if ( newReading <= ( ambient + noiseLEVEL ) ) {
      // Okay, we've returned to normal!
      break;
    };
   
    // Did we timeout? If so, let's exit. 
    if ( ( millis() - startTime ) > TIMEOUT ) {
      break;
    };
   
  };

  // Drop the signal before we leave
  digitalWrite( digitalOUT, LOW);    
};





/*

  Setup function. Just setup the pin input/output values and perform initial settle operation.

*/

void setup() {

    // If we are doing debug, let's setup the serial line
    if ( debug ) {
      Serial.begin(115200); 
    };
    
    pinMode( digitalOUT, OUTPUT);
    digitalWrite( digitalOUT, LOW);
    pinMode( analogIN, INPUT);

    // Force the input values to settle.
    settle();
}




/*

  Main program loop.

*/

void loop() {

  // Take a reading.
  sensorValue = analogRead(analogIN);

  // Determine if the reading counts as a hit.
  if ( sensorValue > ( ambient + threshold ) ) {
    if ( debug ) {
        Serial.print("HIT!!!: ");
        Serial.println(sensorValue);
    };
    latchup();
  };
 
  
  // Determine if we should re-settle the pressure value.
  if ( ( millis() - sinceLastSettle ) > settleTIMEOUT ) {
      settle();
  };
}
