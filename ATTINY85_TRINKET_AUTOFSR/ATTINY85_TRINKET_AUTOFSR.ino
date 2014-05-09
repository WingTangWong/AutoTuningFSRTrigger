/*

  Auto Tuning FSR Trigger for ATtiny85 and AdaFruit Trinket

  Author: Wing Tang Wong
  GitHub: https://github.com/WingTangWong/AutoTuningFSRTrigger/ATTINY85_TRINKET_FSR

  Description:

    This Arduino Sketch is meant to be installed on an ATtiny85 or AdaFruit Trinket, which 
    uses an ATtiny85 as the MCU. This is the heavily pruned and reduced code.

    The sketch makes use of all of the IO pins in the following manner:

    =========   =============    ==================================================================================
    Chip PIN  - Trinket Pin   -  Usage
    =========   =============    ==================================================================================
    1         - RST           -  Short to reset the MCU. Wire to a switch that shorts to GND.
    2         - PB3 / 3 / 3   -  Digital pin 3, Analog pin 3. Used for FSR input.
    3         - PB4 / 4 / 2   -  Digital pin 4, Analog pin 2. Used for FSR input.
    4         - GND           -  GND pin. 
    5         - PB0 / OUT     -  Digital Pin 0, Used for end stop output. HIGH by default, goes LOW when triggered.
    6         - PB1 / LED     -  Digital Pin 1, used as LED indicator. On the Trinket, this is wired to an LED.
    7         - PB2 / 2 / 1   -  Digital pin 2, Analog Pin 1. Used for FSR input.
    8         - VCC           -  VCC input. 5V or 3.3V depending on your Trinket. 
    =========   =============    ==================================================================================

    The design makes use of all three generally accessible ADC pins for three independent FSR inputs. ADC0 /PB5 is 
    on the RSET pin and will not be used as an IO pin.

  Installing the code:

    To install this code on your ATTINY85 MCU or onto the AdaFruit Trinket, you will want to download the Arduino 1.0.5(or later)
    IDE along with AdaFruit's definitions/etc. 

    Alternatively, you can follow the Adafruit instructions on how to customize your existing Arduino IDE to work with the AdaFruit Trinket.

    Note, the Trinket operates at 8Mhz or 16Mhz. Make sure you set the Board in Arduino IDE appropriately.

    If you are programming to a bare ATTIN85 chip, you will want to edit the boards.txt definition to include an "1Mhz Trinket"
    like so:

      trinket1.name=Adafruit Trinket 1MHz
      trinket1.bootloader.low_fuses=0xF1
      trinket1.bootloader.high_fuses=0xD5
      trinket1.bootloader.extended_fuses=0xFE
      trinket1.upload.maximum_size=5310
      trinket1.build.mcu=attiny85
      trinket1.build.f_cpu=1000000L
      trinket1.build.core=arduino:arduino
      trinket1.build.variant=tiny8

    This will allow an ATtiny85 to run off of the internal 1Mhz OSC and with timings operating properly. You can burn the code
    onto a bare ATTINY85 using the 8Mhz definition, but the chip will run super slow. Using the 16Mhz definition on a bare ATTINY85
    can potentially brick your MCU. 


  Chip Hookup:

    Once you have flashed your chip, you will need to hookup the chip to your 3D printer's z-min-end-stop. To do that, you should
    refer to the wiring diagram included as an image in this repo.

    Basically:

    * VCC should be hooked up to your printer board's VCC (Can use the VCC pin from the 3 pin endstop connection)
    * GND should be hooked up to your printer board's GND (Can use the GND pin from the 3 pin endstop connection)
    * OUT should be hooked up to the signal pin on your printer control board's z-min-end-stop. Be sure to not hook it up to the VCC/GND pins.
    * LED can be left unconnected as the Trinket has an oboard LED. If using a bare ATTINY85, you can wire up an LED and low value resistor
      from the pin to GND:  (LED PIN)----|>|------/\/\/\/\-----(GND)
      * LED can be any low voltage LED
      * Resistor can be a typical 1k to 4.7k ohm resistor. 
    * RSET can be left unconnected on the Trinket. On the bare ATTINY85, it can also be left unconnected, or pulled up via a 10k-50k ohm resistor
      hooked up to VCC.
    * ADC1, ADC2, ADC3 should be hooked up as follows:
      * Pin pulled down to GND via a 10K+ ohm resistor. 
      * Pin also connected to one leg of the FSR.
      * Other leg of FSR connected to VCC.
      * You can optionally place an LED in-line with the FSR and VCC so it illuminates when the FSR is pressed. Though this will not indicate
        a trigger state.

  Testing Circuit:
    * Power on your printer board and your new module.
    * Perform an M119 on your printer to get the endstop states. Presuming none of the endstops are triggered, you should see
      that none of the enstops are triggered.
    * Press down on an FSR sensor. The output LED should light up. 
    * Perform another M119. You should now see the z-min-end-stop triggered. 
    * If this works, you are good to give probing a try.
    * Install the FSR under the bed plate if you have not already.
    * Make sure everything is hooked up. If you had left the Trinket on, you will see the LED still lit. Hit the reset button on the Trinket.
      The light will turn off.
    * Press down on the print bed. The light should go on. Letting go will see the light go off again. You're good to attempt a test bed levelling.

  Testing Probing:
    * Assuming your FSR are installed
    * Home your printer
    * With one hand on the printer control board reset switch, initiate a G29 to auto-level probe the bed.
    * When the tip of the hot end touches the bed, if it doesn't tink and back off immediately, ie. it is pushing against the bed, hit the reset.
      Then, check to see what might be wrong.
    * If the head tinks against the bed, but keeps raising up toward home, hit reset. Something made the sensor/module stuck in ON/triggered
      mode.
    * If the head just proceeds through the auto-level process, yay!

*/

// Pin assignments
int OUT = 0;          // OUTPUT pin
int LED = 1;          // LED pin
int A[3]  = {1,2,3};  // Analog pin numbers
int AD[3] = {2,4,3};  // Digital pin numbers correlating to the physical analog pins

// Various variables used for state and tracking calculations
unsigned long PRECALIBRATION=0;
unsigned long SEED = 40;
unsigned long VALUE[3]={0,0,0};
unsigned long AVERAGE[3]={0,0,0};
unsigned long NOISE_LEVEL[3]={0,0,0};
unsigned long TRIGGER_LEVEL[3]={0,0,0};
unsigned long TALLY[3]={0,0,0};
unsigned long TOTAL[3]={0,0,0};
unsigned long STATE[3]={0,0,0};
unsigned long MARGIN = 50;
long scratch;

// the setup routine runs once when you press reset:
void setup() {                
  pinMode( OUT , OUTPUT );
  digitalWrite( OUT , HIGH );
  pinMode( LED , OUTPUT );
  digitalWrite( LED , LOW );
  for(scratch=0; scratch<3 ; scratch++) {
      pinMode( AD[scratch] , INPUT );
      digitalWrite( AD[scratch], LOW );
  };
  do_calibration();
}

// the loop routine runs over and over again forever:
void loop() {
  do_calibration();
  do_read_sensors();
  do_output();
};


// Take the read values and average them
// Also, get the noise level.
void do_read_average(int idx)
{
        VALUE[idx]=analogRead( A[idx] );
        delay(5);
        VALUE[idx]=analogRead( A[idx] );
        delay(5);
        TOTAL[idx] = TOTAL[idx] + VALUE[idx];
        TALLY[idx]++;
        AVERAGE[idx] = ( TOTAL[idx] / TALLY[idx] );
        if ( VALUE[idx] > AVERAGE[idx] ) {
          NOISE_LEVEL[idx] = (unsigned long)(( NOISE_LEVEL[idx] + ( VALUE[idx] - AVERAGE[idx] )) / 2.00);
        } else {
          NOISE_LEVEL[idx] = (unsigned long)(( NOISE_LEVEL[idx] + ( AVERAGE[idx] - VALUE[idx] )) / 2.00);
        };
        if ( NOISE_LEVEL[idx] < 20 ) {
          NOISE_LEVEL[idx] = 20;
        };
};

// If we determined we really need to calibrate, do it here.
// Read in values and set trigger levels.
void do_real_calibration()
{
  int idx;
  int run;

  for(idx=0; idx<3 ; idx++ ) {
    if ( TALLY[idx] < SEED ) {
      for(run=0; run<SEED ; run++ ) {
        do_read_average(idx);
      };
    } else {
      do_read_average(idx);
    };
    TRIGGER_LEVEL[idx]=NOISE_LEVEL[idx] + MARGIN;
  };
  PRECALIBRATION++;
};


// Function to determine if we need to perform a calibration.
void do_calibration()
{
  if ( PRECALIBRATION < 1 ) {
    do_real_calibration();
  };
  
};


// Function to read values from the FSR sensors
void do_read_sensors()
{
  int idx;
  for( idx=0; idx<3 ; idx++ ) {
    VALUE[idx] = analogRead( A[idx] );
    delay(5);
    VALUE[idx] = analogRead( A[idx] );
    delay(5);
    VALUE[idx] = analogRead( A[idx] );
    if ( VALUE[idx] > ( AVERAGE[idx] + TRIGGER_LEVEL[idx]) ) {
      STATE[idx] = 1;
    };
    if ( VALUE[idx] < ( AVERAGE[idx] + NOISE_LEVEL[idx]) ) {
      STATE[idx] = 0;
    };
  };
};

// Update the LED and OUTPUT pins.
void do_output()
{
  int idx;
  int TRIGGERED=0;
  for(idx=0; idx<3; idx++ ){
    TRIGGERED=TRIGGERED + STATE[idx];
  };

  // The HIGH/LOW corresponds to a Normally Connected signal output.
  if ( TRIGGERED > 0 ) {
    digitalWrite( OUT, LOW );
    digitalWrite( LED, HIGH);
  } else {
    digitalWrite( OUT, HIGH
    );
    digitalWrite( LED, LOW );    
  };
};
