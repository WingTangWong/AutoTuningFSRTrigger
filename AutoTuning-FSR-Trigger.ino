/*

  FSR Self Calibrating Trigger
  
  Designed for Arduino, but ideally will be run on an attiny85.
  
  Inputs:
    ADC
    
  Output:
    Digital IO High/Low
  
*/

// IO pins
int analogIN   = A3; // Pin for the ADC input
int digitalOUT = 13; // Pin for the digital signal out
long noiseLEVEL = 15;
long threshold = 25;
// timeouts/etc.
long TIMEOUT = 500; // In milliseconds. 2000 = 2 seconds.
long ambient=0;
long reading=0;
long newambient=0;
long delta=0;
long range=0;
long sensorValue=0;
bool debug = true;
long sinceLastSettle=millis();
long settleTIMEOUT = 250; // Only force resettle every 250 ms


void settle() {
  // We want to wait for readings to settle as well as determine
  // a valid range for normal "noise"
  while ( true ) {
    // Take a triple reading and average it.
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

void latchup() {
  long newReading=0;
  long startTime = millis();
  // Going to set this high until system re-settles.
  digitalWrite( digitalOUT, HIGH );
  while( true ) {
    newReading = analogRead( analogIN );
    if ( newReading <= ambient ) {
      // Okay, we've returned to normal!
      break;
    };
    
    if ( ( millis() - startTime ) > TIMEOUT ) {
      break;
    };
   
  };
  digitalWrite( digitalOUT, LOW);    
};

void setup() {
  // put your setup code here, to run once:
    // initialize serial communications at 9600 bps:
    if ( debug ) {
      Serial.begin(115200); 
    };
    
  pinMode( digitalOUT, OUTPUT);
  digitalWrite( digitalOUT, LOW);
  pinMode( analogIN, INPUT);

  // Force the input values to settle.
  settle();
}

void loop() {
  // put your main code here, to run repeatedly: 
  sensorValue = analogRead(analogIN);

  if ( sensorValue > ( ambient + threshold ) ) {
    if ( debug ) {
        Serial.print("HIT!!!: ");
        Serial.println(sensorValue);
    };
    latchup();

  };
  
  // Should we resettle?
  if ( ( millis() - sinceLastSettle ) > settleTIMEOUT ) {
      settle();
  };
}
