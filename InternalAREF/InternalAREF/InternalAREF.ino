// The _BV() is a macro that shifts 1 to left by the specified numnber.
// ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);



long readVcc() {
  long result;   // Read 1.1V reference against AVcc
  // ADMUX = _BV(MUX3) | _BV(MUX2); // REFS[2:0] bits in ADMUX to select VCC...  ADC channels on MUX[3:0] in ADMUX
  ADMUX = (0<<REFS2) | (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);
  // (0<<REFS2) | (0<<REFS1) | (1<<REFS0) = 1.1 internal bandgap
  // 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

long readADC( int pin = 3 ) {
  long result;   // Read 1.1V reference against AVcc
  ADMUX = _BV(MUX3) | _BV(MUX2);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
};

long readTEMP( ) {
  long result;   // Read 1.1V reference against AVcc
  // The on-chip temperature sensor is selected by writing the code “1111” to the MUX[3:0] bits in ADMUX register
  // when the ADC4 channel is used as an ADC input.

  ADMUX = _BV(MUX3) | _BV(MUX2);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
};




void setup() {
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  pinMode(3,INPUT);
  pinMode(4,INPUT);
}

void loop() {
  for( int i=0; i< (int)( readVcc() / 1000 ); i++) {
    digitalWrite(1, HIGH);
    delay(500);
    digitalWrite(1, LOW);
    delay(500);
  };
  delay(2000);
}
