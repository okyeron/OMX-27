// OMX-27 MIDI KEYBOARD / SEQUENCER

#include "Adafruit_Keypad.h"
#include <Adafruit_NeoPixel.h>
#include <MIDI.h>

#include "ClearUI.h"
#include "sequencer.h"

#define LED_PIN    14
#define LED_COUNT 27

unsigned long blinkInterval = 500;
elapsedMillis msec = 0;

bool blinkState = false;
bool noteSelect = false;
bool noteSelection = false;
int selectedNote = 0;
int selectedStep = 0;

int mode = 0;
int newmode = 0;
const char* modes[] = {"MIDI","SEQ"};

MIDI_CREATE_DEFAULT_INSTANCE();

int noteon_velocity = 100;
int step_delay;
unsigned long tempoStartTime, tempoEndTime, lastStepTime;

// POTS/ANALOG INPUTS				// CCS mapped to Organelle Defaults
int pots[] = {21,22,23,24,7};		// the MIDI CC (continuous controller) for each analog input
int analogPins[] = {23,22,21,20,16};	// teensy pins for analog inputs
int previous[] = {-1,-1,-1,-1,-1};	// store previously sent values, to detect changes
int analogValues[] = {0,0,0,0,0};		// default values
float EMA_a[] = {0.6,0.6,0.6,0.6,0.6};
int EMA_S[] = {0,0,0,0,0};

// KEYSWITCH ROWS/COLS
const byte ROWS = 5; //five rows
const byte COLS = 6; //six columns
// Map the keys
char keys[ROWS][COLS] = {
  {0, 1, 2, 3, 4, 5},
  {6, 7, 8, 9, 10,26},
  {11,12,13,14,15,24},
  {16,17,18,19,20,25},
  {22,23,21}
  };
byte rowPins[ROWS] = {6, 4, 3, 5, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 8, 10, 9, 15, 17}; //connect to the column pinouts of the keypad

// KEYBOARD NOTE LAYOUT
int notes[] = {0,
     61,63,   66,68,70,   73,75,   78,80,82,
59,60,62,64,65,67,69,71,72,74,76,77,79,81,83,84};

int steps[] = {0,
     1,2,   3,4,5,   6,7,   8,9,10,
11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26};

// NOTES


// ENCODER
Encoder myEncoder(12, 11); 	// encoder pins
Button encButton(0);		// encoder button pin
long newPosition = 0;
long oldPosition = -999;

// CV 
word pitchCV;
uint8_t RES;
uint16_t AMAX;
word V_scale;

// COLOR PRESETS
const auto RED = 0xFF0000;
const auto ORANGE = 0xFF8C00;
const auto YELLOW = 0xFFFF00;
const auto GREEN = 0x00FF00;
const auto BLUE = 0x0000FF;
const auto INDIGO = 0x4B0082;
const auto VIOLET = 0xEE82EE;
const auto HALFGREEN = 0x008000;
const auto HALFRED = 0x800000;
const auto DKORANGE = 0x663300;
const auto LBLUE = 0xADD8E6;
const auto DKBLUE = 0x000080;
const auto CYAN = 0x00FFFF;
const auto LTCYAN = 0xE0FFFF;
const auto DKCYAN = 0x008080;
const auto MAGENTA = 0xFF00FF;
const auto DKMAGENTA = 0x330033;
const auto PURPLE = 0x3B0F85;
const auto AMBER = 0x999900;
const auto BEIGE = 0xFFCC33;
const auto HALFWHITE = 0x808080;
const auto LOWWHITE = 0x202020;
const auto LEDOFF = 0x000000;

const uint32_t seqColors[] = {ORANGE,YELLOW,HALFGREEN,MAGENTA,VIOLET,DKCYAN,DKBLUE,PURPLE};

//initialize an instance of class NewKeypad
Adafruit_Keypad customKeypad = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

// Declare NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// ####### SETUP #######

void setup() {
	Serial.begin(9600);
	step_delay = 100;
	lastStepTime = millis();
	
	//CV gate pin
	pinMode(13, OUTPUT); // GONNA MOVE THIS TO A10
		
  	// set DAC Resolution CV/GATE
    RES = 12;
    analogWriteResolution(RES); // set resolution for DAC
    AMAX = pow(2,RES);
    V_scale = 64; // pow(2,(RES-7)); 4095 max
    analogWrite(A14, 0);

  	// Display
	initializeDisplay();
	
	// Startup screen		
	display.clearDisplay();
	testdrawrect();
	delay(300);
	display.clearDisplay();
	display.setCursor(16,4);
	defaultText(2);
	display.setTextColor(SSD1306_WHITE);
	display.println("OMX-27");
	display.display();

	// keypad
	customKeypad.begin();

	// hardware midi
	MIDI.begin();
  
	// Handle incoming MIDI events
	//MIDI.setHandleClock(handleExtClock);
	//MIDI.setHandleStart(handleExtStart);
	//MIDI.setHandleContinue(handleExtContinue);
	//MIDI.setHandleStop(handleExtStop);
	//MIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
	//usbMIDI.setHandleNoteOn(HandleNoteOn); 
	//MIDI.setHandleControlChange(HandleControlChange);
	//usbMIDI.setHandleControlChange(HandleControlChange);
	//MIDI.setHandleNoteOff(HandleNoteOff);
	//usbMIDI.setHandleNoteOff(HandleNoteOff);

	// READ POTS
	for(int k=0; k<5; k++) {
		analogValues[k] = analogRead(analogPins[k]) / 8;
		previous[k] = analogValues[k];
		EMA_S[k] = previous[k];
	}

	//LEDs
	strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
	strip.show();            // Turn OFF all pixels ASAP
	strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

	for(int i=0; i<LED_COUNT; i++) { // For each pixel...
		strip.setPixelColor(i, HALFWHITE);
		strip.show();   // Send the updated pixel colors to the hardware.
		delay(20); // Pause before next pass through loop
	}
	rainbow(10); // rainbow startup pattern
  
	delay(100);
	strip.fill(0, 0, LED_COUNT);
	strip.show();            // Turn OFF all pixels ASAP

	delay(100);

	// Clear display and show default mode
	display.clearDisplay();
	display.setTextSize(1);
	display.setCursor(96, 0);
	display.print(modes[newmode]);
	display.display();			

	//Serial.println(" loading... ");
}

void enc_selector(uint16_t z, uint16_t pos) {
	if (z == 1){
		display.clearDisplay();
		display.setCursor(16, 2);
		display.print("ENC: ");
		display.print(pos);
		display.display();
	}
}

void blinkLED(){

}

// SEQUENCER LEDS
void show_current_step() {

	if (msec >= blinkInterval){
		blinkState = !blinkState;
		msec = 0;
	}

	if (playing && blinkState){
		strip.setPixelColor(0, HALFWHITE);
	} else {
		strip.setPixelColor(0, LEDOFF);
	}

	if (noteSelect && noteSelection) {
		for(int j = 1; j < NUM_STEPS+11; j++){
			if (j == selectedNote){
				strip.setPixelColor(j, HALFWHITE);
			} else if (j == selectedStep+11){
				strip.setPixelColor(j, ORANGE);
			} else{
				strip.setPixelColor(j, LEDOFF);
			}
		}
		
	} else {
		for(int j = 1; j < NUM_STEPS+11; j++){		
			if (j == 1) {
				if (noteSelect){
					if (noteSelect && blinkState){
						strip.setPixelColor(j, DKCYAN);
					} else {
						strip.setPixelColor(j, LEDOFF);
					}
				} else {
					strip.setPixelColor(j, DKCYAN);
				}
			} else if (j == 2) {
				strip.setPixelColor(j, DKBLUE);
			} else if (j == playingPattern+3){  	// pattern select
				strip.setPixelColor(playingPattern+3, seqColors[playingPattern]);
			} else {
				strip.setPixelColor(j, LEDOFF);
			}
		}

		for(int i = 0; i < NUM_STEPS; i++){
			// 
			//stepNote[playingPattern][seqPos] = notes[thisKey]
			if(i % 4 == 0){ // mark groups of 4
				if(i == seqPos){
					strip.setPixelColor(i+11, HALFRED); // step chase
				} else if (stepPlay[playingPattern][i] == 1){
					strip.setPixelColor(i+11, seqColors[playingPattern]); // step on
				} else {
					strip.setPixelColor(i+11, LOWWHITE); 
				}
			} else if (i == seqPos){
				strip.setPixelColor(i+11, HALFRED); // step chase
			} else if (stepPlay[playingPattern][i] == 1){
				strip.setPixelColor(i+11, seqColors[playingPattern]); // step on
			} else {
				strip.setPixelColor(i+11, LEDOFF);
			}
		}
	}
	strip.show();
}

void step_ahead() {
  // step ahead one place
    seqPos++;
    if (seqPos >= NUM_STEPS)
      seqPos = 0;
}

void step_on(){
	//	Serial.print(seqPos);
	//	Serial.println(" step on");
	//  usbMIDI.sendNoteOn(sequencer[current_step].note, VELOCITY, CHANNEL);
}

void step_off(){
	//	Serial.print(seqPos);
	//	Serial.println(" step off");
	//  usbMIDI.sendNoteOff(sequencer[current_step].note, VELOCITY, CHANNEL);
}


// ####### MAIN LOOP #######

void loop() {
	customKeypad.tick();

	// ENCODER
	auto u = myEncoder.update();
	if (u.active()) {
    	//Serial.print(u.dir() < 0 ? "ccw " : "cw ");
    	auto amt = u.accel(0); // where 5 is the acceleration factor if you want it, 0 if you don't)
    	newmode = constrain(mode + amt, 0, 1);

		display.clearDisplay();
		display.setTextSize(1);
		display.setCursor(96, 0);
		display.print(modes[newmode]);
		display.display();			
	}
	auto s = encButton.update();
	switch (s) {
		case Button::Down: Serial.println("down"); 
			if (newmode != mode) {
				mode = newmode;
				playing = 1;
				setAllLEDS(0,0,0);
				}
			break;
		case Button::DownLong: Serial.println("downlong"); 
			break;
		case Button::Up: Serial.println("up"); 
			break;
		case Button::UpLong: Serial.println("uplong"); 
			break;
	}
		
	
	switch(mode) {
		case 0:
			// ############### POTS ###############
			// --- READ POTS to MIDI ---
			if (msec >= 20) {
				msec = 0;
				for(int i=0; i<5; i++) {
					analogValues[i] = analogRead(analogPins[i]) / 8;
					EMA_S[i] = (EMA_a[i]*analogValues[i]) + ((1-EMA_a[i])*EMA_S[i]); // Filtered result
				}
				for(int i=0; i<5; i++) {
					if (EMA_S[i] != previous[i]){
						usbMIDI.sendControlChange(pots[i], analogValues[i], midiChannel);
						MIDI.sendControlChange(pots[i], analogValues[i], midiChannel);
						previous[i] = EMA_S[i]; //analogValues[i];
						display.clearDisplay();
						display.setTextSize(1);
						display.setCursor(96, 0);
//						display.print("MODE: ");
						display.print(modes[mode]);				
						display.setTextSize(2);
						display.setCursor(0, 0);
						display.print("CC");
						display.print(pots[i]);
						display.print(": ");
						display.setCursor(0, 16);
						display.print(analogValues[i]);
						display.display();
					}
				}
			}
			break;
		case 1:
			// SEQUENCER
			if(playing == true) {
				if(millis() > lastStepTime + step_delay){
					lastStepTime = millis();
					step_ahead();
					step_on();
					playNote();
				} else if(millis() > lastStepTime + (step_delay / 2)){
					step_off();
				}
				show_current_step();
			} else {
				show_current_step();
			}
			if (playing) {
				for (int z=0; z<16; z++){
				}
			}
			break;
	} // END POTS SWITCH

	// ############### KEYS ###############
	
	while(customKeypad.available()){
		keypadEvent e = customKeypad.read();
		int thisKey = e.bit.KEY;

		display.clearDisplay();
		display.setTextSize(1);
		display.setCursor(96, 0);
//		display.print("MODE: ");
		display.print(modes[mode]);				

		switch(mode) {
			case 0: // midi controller
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey != 0) {
					//Serial.println(" pressed");
					noteOn(thisKey, noteon_velocity);
					display.setTextSize(2);
					display.setCursor(2, 12);
					display.print("Note:");
					display.print(notes[thisKey]);
				} else if(e.bit.EVENT == KEY_JUST_RELEASED && thisKey != 0) {
				  //Serial.println(" released");
				  noteOff(thisKey);
				}
				// what to do with AUX key?
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey == 0) {
				  usbMIDI.sendControlChange(25, 100, midiChannel);
				  MIDI.sendControlChange(25, 100, midiChannel);
				  strip.setPixelColor(thisKey, HALFRED);
				} else if (e.bit.EVENT == KEY_JUST_RELEASED && thisKey == 0) { 
				  usbMIDI.sendControlChange(25, 0, midiChannel);
				  MIDI.sendControlChange(25, 0, midiChannel);
				  strip.setPixelColor(thisKey, HALFWHITE);
				}
				strip.show();						
				break;
				
			case 1: // sequencer
				// Sequencer row keys

				// PRESSES
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey != 0) {
					int keyPos = thisKey - 11;
					
					// are we noteSelect ?
					if (noteSelect){
						if (noteSelection) {
							// blink ??
							stepNote[playingPattern][keyPos] = notes[thisKey];
							selectedStep = keyPos;
							selectedNote = thisKey;
							
						} else {
							selectedStep = keyPos; //set playhead to this step
							noteSelection = true;
						}
					} else {					
						// Black Keys
						if (thisKey > 2 && thisKey < 11) {
							playingPattern = thisKey-3;
						} else if (thisKey == 1) {
							noteSelect = !noteSelect;
						} else if (thisKey > 10) {
							if (stepPlay[playingPattern][keyPos] == 1){
								stepPlay[playingPattern][keyPos] = 0;
								//strip.setPixelColor(thisKey, LEDOFF); 
							} else if (stepPlay[playingPattern][keyPos] == 0){
								stepPlay[playingPattern][keyPos] = 1;
								stepNote[playingPattern][keyPos] = notes[thisKey];
								//strip.setPixelColor(thisKey, ORANGE); 
							}
						}
					}
				}
				

				// RELEASES
				if (e.bit.EVENT == KEY_JUST_RELEASED && thisKey != 0 && noteSelection && selectedNote > 0) {
					noteSelection = false;
					noteSelect = false;
					selectedStep = 0;
					selectedNote = 0;
				}

				// AUX key
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey == 0) {
					strip.setPixelColor(0, HALFWHITE);
					if (playing){
						playing = 0;
						allNotesOff();
						strip.setPixelColor(0, LEDOFF);
					} else {
						playing = 1;
					}
				} else if (e.bit.EVENT == KEY_JUST_RELEASED && thisKey == 0) {
				
				}
				// 
				strip.show();
				break;
		} // END SWITCH
		display.display();
	} // END KEYS WHILE
	
	

	while (usbMIDI.read()) {
	// ignore incoming messages
	}
	while (MIDI.read()) {
	// ignore incoming messages
	}
}

void noteOn(int notenum, int velocity){
	usbMIDI.sendNoteOn(notes[notenum], velocity, midiChannel);
	MIDI.sendNoteOn(notes[notenum], velocity, midiChannel);
	strip.setPixelColor(notenum, HALFWHITE);         //  Set pixel's color (in RAM)

	pitchCV = map (notes[notenum], 35, 90, 0, 4096);
	digitalWrite(13, HIGH);
	analogWrite(A14, pitchCV);
}
void noteOff(int notenum){
	usbMIDI.sendNoteOff(notes[notenum], 0, midiChannel);
	MIDI.sendNoteOff(notes[notenum], 0, midiChannel);
	strip.setPixelColor(notenum, LEDOFF); 
	digitalWrite(13, LOW);
	analogWrite(A14, 0);
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 1*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

void testdrawrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }

  delay(500);
}

void setAllLEDS(int R, int G, int B) {
	for(int i=0; i<LED_COUNT; i++) { // For each pixel...
		strip.setPixelColor(i, strip.Color(R, G, B));
	}
	strip.show();   // Send the updated pixel colors to the hardware.
}
