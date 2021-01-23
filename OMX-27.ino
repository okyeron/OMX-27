// OMX-27 MIDI KEYBOARD / SEQUENCER
// 
// Steven Noreyko, January 2021

#include <Adafruit_Keypad.h>
#include <Adafruit_NeoPixel.h>
#include <ResponsiveAnalogRead.h>
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

#include "sequencer.h"
#include "ClearUI.h"

#define LED_PIN    14
#define LED_COUNT 27

const int potCount = 5;
ResponsiveAnalogRead *analog[potCount];

// storage of pot values; current is in the main loop; last value is for midi output
int volatile currentValue[potCount];
int lastMidiValue[potCount];
int potMin = 0;
int potMax = 8190;
int temp;

elapsedMillis msec = 0;
elapsedMillis pots_msec = 0;
elapsedMillis checktime1 = 0;
elapsedMicros clksTimer = 0;
unsigned long clksDelay;

elapsedMillis step_interval[8] = {0,0,0,0,0,0,0,0};
unsigned long lastStepTime[8] = {0,0,0,0,0,0,0,0};
float step_delay;


bool dirtyPixels = false;
bool dirtyDisplay = false;
bool blinkState = false;
bool noteSelect = false;
bool noteSelection = false;
bool funcTwoSelect = false;
int selectedNote = 0;
int selectedStep = 0;
bool midiAUX = false;
bool enc_edit = false;

float clockbpm = 120;
float newtempo = clockbpm;
int noteon_velocity = 100;
unsigned long tempoStartTime, tempoEndTime;

unsigned long blinkInterval = clockbpm * 2;


int mode = 0;
int newmode = 0;
const char* modes[] = {"MIDI","SEQ-1","SEQ-2"};


// POTS/ANALOG INPUTS				// CCS mapped to Organelle Defaults
int pots[] = {21,22,23,24,7};		// the MIDI CC (continuous controller) for each analog input
int analogPins[] = {23,22,21,20,16};	// teensy pins for analog inputs
int previous[] = {-1,-1,-1,-1,-1};	// store previously sent values, to detect changes
int analogValues[] = {0,0,0,0,0};		// default values
int potCC = pots[0];
int potVal = analogValues[0];

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

int octave = 0; // default C4 is 0 - range is -4 to +5
int newoctave = octave;


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


// FIGURE OUT WHAT TO DO WITH CLOCK FOR NOW
void sendClock(){
	usbMIDI.sendRealTime(usbMIDI.Clock);
//	MIDI.sendClock();
}
void startClock(){
	usbMIDI.sendRealTime(usbMIDI.Start);
//	MIDI.sendStart();
}
void stopClock(){
	usbMIDI.sendRealTime(usbMIDI.Stop);
//	MIDI.sendStop();
}
void resetClocks(){
	// ############### CLOCK/TIMING ###############

	// BPM tempo to step-delay calculation
	step_delay = 60000 / clockbpm / 4; // 16th notes
	// BPM to clock pulses
	clksDelay = (60000000 / clockbpm) / 24;
}

void readPotentimeters(){
	for(int k=0; k<potCount; k++) {
		temp = analogRead(analogPins[k]);
		analog[k]->update(temp);
		
	    // read from the smoother, constrain (to account for tolerances), and map it
    	temp = analog[k]->getValue();
    	temp = constrain(temp, potMin, potMax);
		temp = map(temp, potMin, potMax, 0, 16383);
		// map and update the value
    	analogValues[k] = temp >> 7;
   	
    	if(analog[k]->hasChanged()) {
      		// do stuff		
			usbMIDI.sendControlChange(pots[k], analogValues[k], midiChannel);
			MIDI.sendControlChange(pots[k], analogValues[k], midiChannel);
			potCC = pots[k];
			potVal = analogValues[k];
			dirtyDisplay = true;
    	}
	}
}

// ####### SETUP #######

void setup() {
	Serial.begin(115200);
	checktime1 = 0;
	clksTimer = 0;
	resetClocks();
	
	// set analog read resolution to teensy's 13 usable bits
	analogReadResolution(13);
	
	// initialize ResponsiveAnalogRead
	for (int i = 0; i < potCount; i++){
		analog[i] = new ResponsiveAnalogRead(0, true, .001);
		analog[i]->setAnalogResolution(1 << 13);

		// ResponsiveAnalogRead is designed for 10-bit ADCs
		// meanining its threshold defaults to 4. Let's bump that for 
		// our 13-bit adc by setting it to 4 << (13-10)
		analog[i]->setActivityThreshold(32);

		currentValue[i] = 0;
		lastMidiValue[i] = 0;
	}

	// hardware midi
	MIDI.begin();
		
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
//	readPotentimeters();
	
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
	//dispTempo();
	display.display();			

	//Serial.println(" loading... ");
}
// ####### END SETUP #######




// ####### MIDI LEDS

void midi_leds() {
	if (midiAUX){
		strip.setPixelColor(0, HALFRED);
	} else {
		strip.setPixelColor(0, LEDOFF);
	}
	dirtyPixels = true;
}

// ####### SEQUENCER LEDS

void show_current_step(int patternNum) {
	blinkInterval = step_delay*2;
	
	if (msec >= blinkInterval){
		blinkState = !blinkState;
		msec = 0;
	}
															// AUX KEY
	if (playing && blinkState){
		strip.setPixelColor(0, HALFWHITE);
	} else if (noteSelect){
		strip.setPixelColor(0, CYAN);
	} else if (funcTwoSelect){
		strip.setPixelColor(0, DKBLUE);
	} else {
		strip.setPixelColor(0, LEDOFF);
	}

	if (noteSelect && noteSelection) {
		for(int j = 1; j < NUM_STEPS+11; j++){
			if (j < patternLength[patternNum]+11){
				if (j == selectedNote){
					strip.setPixelColor(j, HALFWHITE);
				} else if (j == selectedStep+11){
					strip.setPixelColor(j, ORANGE);
				} else{
					strip.setPixelColor(j, LEDOFF);
				}
			} else {
				strip.setPixelColor(j, LEDOFF);
			}
		}
		
	} else {
		for(int j = 1; j < NUM_STEPS+11; j++){		
			if (j < patternLength[patternNum]+11){
				if (j == 1) {								// NOTE SELECT
					if (noteSelect){
						if (noteSelect && blinkState){
							strip.setPixelColor(j, DKCYAN);
						} else {
							strip.setPixelColor(j, LEDOFF);
						}
					} else {
						strip.setPixelColor(j, DKCYAN);
					}
					
				} else if (j == 2) {						// FUNC TWO
					if (funcTwoSelect){
						if (funcTwoSelect && blinkState){
							strip.setPixelColor(j, DKBLUE);
						} else {
							strip.setPixelColor(j, LEDOFF);
						}
					} else {
						strip.setPixelColor(j, DKBLUE);
					}

					
				} else if (j == patternNum+3){  			// PATTERN SELECT
					strip.setPixelColor(patternNum+3, seqColors[patternNum]);

				} else {
					strip.setPixelColor(j, LEDOFF);
				}
			} else {
				strip.setPixelColor(j, LEDOFF);
			}
		}

		for(int i = 0; i < NUM_STEPS; i++){
			if (i < patternLength[patternNum]){
				if(i % 4 == 0){ // mark groups of 4
					if(i == seqPos[patternNum]){
						if (playing){
							strip.setPixelColor(i+11, HALFRED); // step chase
						} else if (stepPlay[patternNum][i] == 1){
							strip.setPixelColor(i+11, seqColors[patternNum]); // step on color
						} else {
							strip.setPixelColor(i+11, LOWWHITE); 
						}
//						strip.setPixelColor(i+11, HALFRED); // step chase
					} else if (stepPlay[patternNum][i] == 1){
						strip.setPixelColor(i+11, seqColors[patternNum]); // step on color
					} else {
						strip.setPixelColor(i+11, LOWWHITE); 
					}
				} else if (i == seqPos[patternNum]){
					if (playing){
						strip.setPixelColor(i+11, HALFRED); // step chase
					} else if (stepPlay[patternNum][i] == 1){
						strip.setPixelColor(i+11, seqColors[patternNum]); // step on color
					} else {
						strip.setPixelColor(i+11, LEDOFF); 
					}
//					strip.setPixelColor(i+11, HALFRED); // step chase

				} else if (stepPlay[patternNum][i] == 1){
					strip.setPixelColor(i+11, seqColors[patternNum]); // step on color

				} else {
					strip.setPixelColor(i+11, LEDOFF);
				}
			}
		}
	}
	dirtyPixels = true;
//	strip.show();
}

void step_ahead(int patternNum) {
  // step ahead one place
    seqPos[patternNum]++;
    if (seqPos[patternNum] >= patternLength[patternNum])
      seqPos[patternNum] = 0;
}

void step_on(int patternNum){
	//	Serial.print(g);
	//	Serial.println(" step on");

}

void step_off(int patternNum, int position){
	//	Serial.print(seqPos[patternNum]);
	//	Serial.println(" step off");
      usbMIDI.sendNoteOff(lastNote[patternNum][position], 0, midiChannel);
      MIDI.sendNoteOff(lastNote[patternNum][position], 0, midiChannel);
      lastNote[patternNum][position] = 0;
      analogWrite(A14, 0);
      digitalWrite(13, LOW);
}

// #### DISPLAY ###

void dispPattLen(){
	display.setTextSize(1);
	display.setCursor(0, 0);
	display.print("SEQ: ");
	display.setCursor(32, 0);
	display.print(playingPattern+1);

	display.setCursor(0, 12);
	display.print("LEN: ");
	display.setCursor(32, 12);
	display.print(pattLen[playingPattern]);

}

void dispPatt(){
	display.setTextSize(1);
	display.setCursor(96, 0);
	display.print("P:");
	display.print(playingPattern+1);

	display.setCursor(96, 12);
	display.print("L:");
	display.print(pattLen[playingPattern]);
}

void dispPots(){
	display.setTextSize(1);
	display.setCursor(0, 0);
	display.print("CC");
	display.print(potCC);
	display.print(": ");
	display.setCursor(30, 0);
	display.print(potVal);
}

void dispTempo(){
	display.setTextSize(1);
	display.setCursor(74, 12);
	display.print("BPM:");
	display.setCursor(100, 12);
	display.print((int)clockbpm);
}

void dispOctave(){
	display.setTextSize(1);
	display.setCursor(0, 24);
	display.print("Octave:");
	display.print((int)octave+4);
}

void dispNotes(){
	display.setTextSize(1);
	display.setCursor(0, 12);
	display.print("NOTE:");
	display.print(lastNote[playingPattern][seqPos[playingPattern]]);
//	display.setCursor(0, 24);
//	display.print("BPM:");
//	display.print((int)clockbpm);
}

void dispNoteSelect(){
	display.setTextSize(1);
	display.setCursor(0, 0);
	display.print("NOTE-SELECT ");

	display.setCursor(0, 12);
	display.print("STEP: ");
	display.print(selectedStep+1);
	display.setCursor(0, 24);
	display.print("NOTE:");
	display.print(stepNote[playingPattern][selectedStep]);
}
void dispMode(){
	display.setTextSize(1);
	display.setCursor(96, 0);
	if (newmode != mode && enc_edit) {
		display.print(modes[newmode]);
	} else if (enc_edit) {
		display.print(modes[mode]);
	}
}

// ####### MAIN LOOP #######

void loop() {
	customKeypad.tick();
	checktime1 = 0;
	
//	resetClocks();
	if (clksTimer > clksDelay ) {
		// SEND CLOCK
	  clksTimer = 0;
	}
//Serial.println(step_delay);

	// DISPLAY SETUP
	display.clearDisplay();
			
				
	// ############### POTS ###############
	//
	readPotentimeters();
	

	// ############### ENCODER ###############
	// 
	auto u = myEncoder.update();
	if (u.active()) {
    	auto amt = u.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)
//    	Serial.println(u.dir() < 0 ? "ccw " : "cw ");
//    	Serial.println(amt);
    	
		// Change Mode
    	if (enc_edit) {
			// set mode
	    	newmode = constrain(newmode + amt, 0, 2);
	    	dispMode();
	    	dirtyDisplay = true;

		} else if (!noteSelect){
		
			// set octave 
			newoctave = constrain(octave + amt, -5, 4);
			if (newoctave != octave){
				octave = newoctave;
				dirtyDisplay = true;
			}
//			newtempo = constrain(clockbpm + amt, 40, 300);
//			if (newtempo != clockbpm){
//				// SET TEMPO HERE
//				clockbpm = newtempo;
				resetClocks();
//				dirtyDisplay = true;
//			}
		} else {
			switch(mode) { // process encoder input depending on mode
				case 0: // MIDI
					break;
				case 1: // SEQ
					if (noteSelect && !enc_edit){ // sequence edit more
						// 
						pattLen[playingPattern] = constrain(patternLength[playingPattern] + amt, 1, 16);
						patternLength[playingPattern] = pattLen[playingPattern];
						dirtyDisplay = true;
					}		
					break;
				case 2: // SEQ
					if (noteSelect && !enc_edit){ // sequence edit more
						// 
						pattLen[playingPattern] = constrain(patternLength[playingPattern] + amt, 1, 16);
						patternLength[playingPattern] = pattLen[playingPattern];
						dirtyDisplay = true;
					}		
					break;
			}		

		}
	}
	
	auto s = encButton.update();
	switch (s) {
		case Button::Down: //Serial.println("Button down"); 
			if (newmode != mode && enc_edit) {
				mode = newmode;
				playing = 0;
				setAllLEDS(0,0,0);
			}
			enc_edit = !enc_edit;			
			dispMode();
			dirtyDisplay = true;
			break;
		case Button::DownLong: //Serial.println("Button downlong"); 
			break;
		case Button::Up: //Serial.println("Button up"); 
			break;
		case Button::UpLong: //Serial.println("Button uplong"); 
			break;
	}
				

	// ############### KEY HANDLING ###############
	
	while(customKeypad.available()){
		keypadEvent e = customKeypad.read();
		int thisKey = e.bit.KEY;

		switch(mode) {
		
			case 0: // MIDI CONTROLLER
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey != 0) {
					//Serial.println(" pressed");
					noteOn(thisKey, noteon_velocity, playingPattern);
					
				} else if(e.bit.EVENT == KEY_JUST_RELEASED && thisKey != 0) {
					//Serial.println(" released");
					noteOff(thisKey);
				}
				
				// AUX KEY
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey == 0) {
					// Hard coded Organelle stuff
					usbMIDI.sendControlChange(25, 100, midiChannel);
					MIDI.sendControlChange(25, 100, midiChannel);
					if (midiAUX) {
						// STOP CLOCK
					} else {
						// START CLOCK
					}
					midiAUX = !midiAUX;
					
				} else if (e.bit.EVENT == KEY_JUST_RELEASED && thisKey == 0) { 
					// Hard coded Organelle stuff
					usbMIDI.sendControlChange(25, 0, midiChannel);
					MIDI.sendControlChange(25, 0, midiChannel);
//					midiAUX = false;
				}					
				break;

			case 1: // SEQUENCER 1
				// fall through
			case 2: // SEQUENCER 2
				// Sequencer row keys

				// PRESS EVENTS
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey != 0) {
					
					int keyPos = thisKey - 11;
					
					// are we noteSelect ?
					if (noteSelect){
						if (noteSelection) {
							selectedNote = thisKey;
							stepNote[playingPattern][selectedStep] = notes[thisKey];
							dirtyDisplay = true;
							
						} else if ( thisKey > 10 ) {
							selectedStep = keyPos; //set selection to this step
							noteSelection = true;
							dirtyDisplay = true;
							
						} else if (thisKey > 2 && thisKey < 11) { // Pattern select keys
							playingPattern = thisKey-3;
							dirtyDisplay = true;
							
						} else if (thisKey == 1) { 
							noteSelect = !noteSelect; // toggle noteSelect on/off
							dirtyDisplay = true;
						}
						
					// are we funcTwoSelect ?
					}else if (funcTwoSelect && thisKey > 10){
					

					} else {					
						// Black Keys
						if (thisKey > 2 && thisKey < 11) { // Pattern select keys
							playingPattern = thisKey-3;
							dirtyDisplay = true;
							
						} else if (thisKey == 1) { 
							noteSelect = !noteSelect; // toggle noteSelect on/off
							dirtyDisplay = true;

						} else if (thisKey == 2) { 
							funcTwoSelect = !funcTwoSelect; // toggle funcTwoSelect on/off

							seqReset(); // reset all sequences to step 1
							dirtyDisplay = true;
							
						} else if (thisKey > 10) { // SEQUENCE 1-16 KEYS
							if (stepPlay[playingPattern][keyPos] == 1){ // toggle note on
								stepPlay[playingPattern][keyPos] = 0;
							} else if (stepPlay[playingPattern][keyPos] == 0){ // toggle note off
								stepPlay[playingPattern][keyPos] = 1;
								//stepNote[playingPattern][keyPos] = notes[thisKey];
							}
						}
					}
//					Serial.print("playingPattern: ");
//					Serial.println(playingPattern);
				}
				

				// RELEASE EVENTS
				if (e.bit.EVENT == KEY_JUST_RELEASED && thisKey != 0 && noteSelection && selectedNote > 0) {
					noteSelection = false;
//					noteSelect = false;
					selectedStep = 0;
					selectedNote = 0;
				}

				// AUX key
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey == 0) {
					if (noteSelect){
						// toggle out of note select
						noteSelect = !noteSelect;
						if (noteSelection){
							noteSelection = false;
						}
						dirtyDisplay = true;
					} else if (funcTwoSelect){
						funcTwoSelect = !funcTwoSelect;
						dirtyDisplay = true;
					} else {
//						strip.setPixelColor(0, HALFWHITE);
						if (playing){
							playing = 0;
							allNotesOff();
//							strip.setPixelColor(0, LEDOFF);
						} else {
							playing = 1;
						}
					}
				} else if (e.bit.EVENT == KEY_JUST_RELEASED && thisKey == 0) {
				
				}

//				strip.show();
				break;
		} // END MODE SWITCH

	} // END KEYS WHILE

	// ############### MODES ###############

	switch(mode){
		case 0:			// MIDI KEYBOARD
			midi_leds();

			if (dirtyDisplay){
				dispPots();
				//dispTempo();
				dispNotes();
				dispOctave();
			}
			break;
		case 1: 		// SEQUENCER 1

			if (dirtyDisplay){
				if (!enc_edit){
					dispPatt();
					dispTempo();
				}
				
				if (noteSelect) {
					dispNoteSelect();
					
				} else {
					//dispPattLen();
					//dispTempo();		
				}
			}
			if(playing == true) {
				// step timing
				if(step_interval[playingPattern] >= step_delay){
					
					// Do stuff
					
					// turn previous note off
					int lastPos = (seqPos[playingPattern]+15) % 16;
//					Serial.println(lastPos);
					if (lastNote[playingPattern][lastPos] > 0){
						step_off(playingPattern, lastPos);
					}
					
					lastStepTime[playingPattern] = step_interval[playingPattern];
					
					step_on(playingPattern);
					playNote(playingPattern);
					show_current_step(playingPattern);
					step_ahead(playingPattern);
					step_interval[playingPattern] = 0;
				}
			} else {
				show_current_step(playingPattern);
			}
			if (playing) {
				for (int z=0; z<16; z++){
				}
			}
			break;
		case 2: 		// SEQUENCER 2
//			readPotentimeters();
			if (dirtyDisplay){
				if (noteSelect) {
					dispNoteSelect();
				} else {
					dispPattLen();
					dispTempo();		
				}
			}
			if(playing == true) {
				for (int j=0; j<8; j++){
//					Serial.print("pattern:");
//					Serial.println(j);
					
					if(step_interval[j] >= step_delay){
						int lastPos = (seqPos[j]+15) % 16;
						if (lastNote[j][lastPos] > 0){
							step_off(j, lastPos);
						}
						lastStepTime[j] = step_interval[j];
						step_on(j);
						playNote(j);
						show_current_step(playingPattern);
						step_ahead(j);
						step_interval[j] = 0;
//					} else if(step_interval[playingPattern] >= step_delay / 2){
//						step_off(j, lastPos);
//						show_current_step(playingPattern);
					}
				}				
			} else {
				show_current_step(playingPattern);
			}
		break;
	}

//	Serial.print("one:");
//	Serial.println(checktime1);

	// DISPLAY at end of loop

	if (dirtyDisplay){
		display.display();
		dirtyDisplay = false;
	}
	
//	Serial.print("two:");
//	Serial.println(checktime1);

	// are pixels dirty
	if (dirtyPixels){
		strip.show();	
		dirtyPixels = false;
	}

	while (usbMIDI.read()) {
		// ignore incoming messages
	}
	while (MIDI.read()) {
		// ignore incoming messages
	}
	
} // #### END MAIN LOOP

void seqReset(){
	for (int k=0; k<8; k++){
		seqPos[k] = 0;
	}
}

// #### MIDI Mode note on/off
void noteOn(int notenum, int velocity, int patternNum){
	int adjnote = notes[notenum] + (octave * 12); // adjust key for octave range
	if (adjnote>=0 && adjnote <128){
		usbMIDI.sendNoteOn(adjnote, velocity, midiChannel);
		lastNote[patternNum][seqPos[patternNum]] = adjnote;
		MIDI.sendNoteOn(adjnote, velocity, midiChannel);	
		// CV
		pitchCV = map (adjnote, 35, 90, 0, 4096);
		digitalWrite(13, HIGH);
		analogWrite(A14, pitchCV);
	}

	strip.setPixelColor(notenum, HALFWHITE);         //  Set pixel's color (in RAM)
	dirtyPixels = true;	
	dirtyDisplay = true;
}
void noteOff(int notenum){
	int adjnote = notes[notenum] + (octave * 12); // adjust key for octave range
	if (adjnote>=0 && adjnote <128){
		usbMIDI.sendNoteOff(adjnote, 0, midiChannel);
		MIDI.sendNoteOff(adjnote, 0, midiChannel);
		// CV
		digitalWrite(13, LOW);
		analogWrite(A14, 0);
	}
	
	strip.setPixelColor(notenum, LEDOFF); 
	dirtyPixels = true;
	dirtyDisplay = true;
}

// #### LED STUFF
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
void setAllLEDS(int R, int G, int B) {
	for(int i=0; i<LED_COUNT; i++) { // For each pixel...
		strip.setPixelColor(i, strip.Color(R, G, B));
	}
	dirtyPixels = true;
}

// #### OLED STUFF
void testdrawrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }

  delay(500);
}
