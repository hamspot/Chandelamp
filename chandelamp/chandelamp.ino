/* 
  Ham Spot "Chandelamp" Touch Lamp - Chandelier style desk lamp
  Copyright 2012 Ham Spot Inc
  Code released under GNU General Public License version 2
  
  Theory of operation:
    1. At boot, 74LS154 chip select lines are setup to pulse width modulate at the desired lamp brightness duty cycle
    2. The circuit is allowed to settle then a calibration measurement is taken from the capacitive touch plate.
    3. The circuit begins running in a tight loop.
    4. The capacitive touch plate is scanned for relative capacitance changes from the calibration point.
    5. If the capacitance changes for a period longer than debounceDelay and modeChangeDelay touchMode increments by 1
    6. lamp_mode is called to light up all of the lights below the current touchMode value; one lamp at a time to save energy.
    7. If the lamp mode exceeds the number of lamps the pulse width duty cycle is lowered increasing brightness.
    8. If the lamp mode exceeds full brightness a shutdown animation is called and the lamp goes dark.
    9. Loop back to 3 thousands of times of times per second to eliminate any flickering.
*/

/* DEFAULTS */
int brightness = 128;       // 50% Brightness = 128, 75% brightness = 64, 100% brightness = 0
long debounceDelay = 50;    // time it takes to register a touch; may cause flicker when touching
long modeChangeDelay = 300; // time between mode changes 


// constants won't change. They're used here to 
// set pin numbers:
const int ledPWM = 1;
const int ledA = 2;
const int ledB = 3;
const int ledC = 4;
const int touchSense = 0;

// sensor key
#define TOUCHPORT 1<<PINB0
 
// Variables will change:
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
long lastDebounceTime = 0;   // the last time the output pin was toggled
long modeChangeTime = 0;     // the last time the mode was changed

// calbration occurs at startup only
int calibrate = 0;
int capval = 0;
 
// lamp mode of operation incremented by touch
int touchMode = 0;
 
void setup() {
  // initialize the digital pin as an output.
  pinMode(ledPWM, OUTPUT);
  pinMode(touchSense, INPUT);
  pinMode(ledA, OUTPUT); // binary 0001
  pinMode(ledB, OUTPUT); // binary 0010
  pinMode(ledC, OUTPUT); // binary 0100
  
  // set the default brightness to 50%
  analogWrite(ledPWM, brightness);

  delay(100); // let the circuit settle a bit
  calibrate = getcap(TOUCHPORT);
}
 
void loop() {
  // read the state of the switch into a local variable:
  int reading = readplate();
 
  // check to see if you just pressed the button 
  // (i.e. the input went from LOW to HIGH),  and you've waited 
  // long enough since the last press to ignore any noise:  
 
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  } 
 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    buttonState = reading;
  }
 
  // set the LED using the state of the button:
  if( (millis() - modeChangeTime) > modeChangeDelay && buttonState == 1 ){
    touchMode++;
    modeChangeTime = millis();
  }
  
  // light up the appropriate lamps
  lamp_mode(); 
 
  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
}

// light test 
void lamp_mode(){
  // if we are in mode 9 and brightness is less than hi
  if( touchMode == 9 && brightness != 0 ){
    lamp_level_hi();
  }
  
  // if we are in mode 8 and brightness less than medium
  if( touchMode == 8 && brightness != 64 ){
    lamp_level_med();
  }
  
  // if we are in mode 0 and brightness is more than low
  if( touchMode == 0 && brightness != 128 ){
    lamp_level_low();
  }
  
  // setup lights for the current mode
  switch( touchMode ){
    case 9: // hi bright
    case 8: // med bright
    case 7: // low bright
      lamp_on(7);
    case 6:
      lamp_on(6);
    case 5:
      lamp_on(5);
    case 4:
      lamp_on(4);
    case 3:
      lamp_on(3);
    case 2:
      lamp_on(2);
    case 1:
      lamp_on(1);
      break;
    case 10:
      lamp_waterfall();
    default:
      touchMode = 0;
      lamp_off();
  }
}

// enable low brightness
void lamp_level_low(){
      brightness = 128;
      analogWrite(ledPWM, brightness);
}

// enable medium brightness
void lamp_level_med(){
      brightness = 64;
      analogWrite(ledPWM, brightness);
}

// enable high brightness
void lamp_level_hi(){
      brightness = 0;
      analogWrite(ledPWM, brightness);
}

// turn all off lights
void lamp_off(){
      digitalWrite(ledA, LOW);
      digitalWrite(ledB, LOW);
      digitalWrite(ledC, LOW);
}

// main lamp control to drive 74LS154; bin to dec encoder, line driver 
void lamp_on(int lamp_id){
  // turn all lamps off before we try to turn something on
  lamp_off();
  
  // turn on just the lamp specified
  switch(lamp_id){
    case 1:
      digitalWrite(ledA, HIGH);
      digitalWrite(ledB, LOW);
      digitalWrite(ledC, LOW);
      break;
    case 2:
      digitalWrite(ledA, LOW);
      digitalWrite(ledB, HIGH);
      digitalWrite(ledC, LOW);
      break;
    case 3:
      digitalWrite(ledA, HIGH);
      digitalWrite(ledB, HIGH);
      digitalWrite(ledC, LOW);
      break;
    case 4:
      digitalWrite(ledA, LOW);
      digitalWrite(ledB, LOW);
      digitalWrite(ledC, HIGH);
      break;
    case 5:
      digitalWrite(ledA, HIGH);
      digitalWrite(ledB, LOW);
      digitalWrite(ledC, HIGH);
      break;
    case 6:
      digitalWrite(ledA, LOW);
      digitalWrite(ledB, HIGH);
      digitalWrite(ledC, HIGH);
      break;
    case 7:
      digitalWrite(ledA, HIGH);
      digitalWrite(ledB, HIGH);
      digitalWrite(ledC, HIGH);
      break;
  }
  
  // leave lamp on for 100 microseconds
  delayMicroseconds(100);
}

// waterfall animation (guilded lilly shutdown)
void lamp_waterfall(){
    int lamp = 7;
    long fallcycle = millis();
    
    // cascade downward one level at a time
    for( lamp = 7; lamp >= 0; lamp-- ){
      // start cycle anew
      fallcycle = millis();
      
      // human brains are slow
      while( (fallcycle + 100) > millis() ){
        switch( lamp ){
          case 7:
            lamp_on(7);
          case 6:
            lamp_on(6);
          case 5:
            lamp_on(5);
          case 4:
            lamp_on(4);
          case 3:
            lamp_on(3);
          case 2:
            lamp_on(2);
          case 1:
            lamp_on(1);
          case 0:
            lamp_off();
        }
      }
    }
    
    // loops end and the lights go out
    lamp_off();
    touchMode = 0;
    lamp_level_low();
    delay(500);
}

boolean readplate()
{
  // read touch plate
  capval = getcap(TOUCHPORT);
  
  // if the value is greater than the calibration point
  if( capval > calibrate ){
    return HIGH;
  }
  else {
    return LOW;
  }
}
 
// relative capacity of the touchplate
char getcap(char pin)
{
  char i = 0;
  DDRB &= ~pin;          // input
  PORTB |= pin;          // pullup on
  for(i = 0; i <16; i++)
    if( (PINB & pin) ) break;
  PORTB &= ~pin;         // low level
  DDRB |= pin;           // discharge
  return i;
}
