// OMX-27 MIDI KEYBOARD / SEQUENCER

#include "Adafruit_Keypad.h"
#include <Adafruit_NeoPixel.h>
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

#include "sequencer.h"
#include "ClearUI.h"

#define LED_PIN    14
#define LED_COUNT 27

unsigned long blinkInterval = 500;

elapsedMillis msec = 0;
elapsedMillis pots_msec = 0;
elapsedMillis checktime1 = 0;
elapsedMicros clksTimer = 0;
long clksDelay;

elapsedMillis step_interval[8] = {0,0,0,0,0,0,0,0};
unsigned long lastStepTime[8] = {0,0,0,0,0,0,0,0};
uint16_t step_delay;

bool dirtyPixels = false;
bool dirtyDisplay = false;

bool blinkState = false;
bool noteSelect = false;
bool noteSelection = false;
int selectedNote = 0;
int selectedStep = 0;
bool midiAUX = false;
bool enc_edit = false;

float clockbpm = 120;
float newtempo = clockbpm;
int noteon_velocity = 100;
unsigned long tempoStartTime, tempoEndTime;


int mode = 0;
int newmode = 0;
const char* modes[] = {"MIDI","SEQ-1","SEQ-2"};


// POTS/ANALOG INPUTS				// CCS mapped to Organelle Defaults
int pots[] = {21,22,23,24,7};		// the MIDI CC (continuous controller) for each analog input
int analogPins[] = {23,22,21,20,16};	// teensy pins for analog inputs
int previous[] = {-1,-1,-1,-1,-1};	// store previously sent values, to detect changes
int analogValues[] = {0,0,0,0,0};		// default values
float EMA_a[] = {0.6,0.6,0.6,0.6,0.6};
int EMA_S[] = {0,0,0,0,0};
int potCC = pots[1];
int potVal = analogValues[1];


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

// ####### SETUP #######

void setup() {
	Serial.begin(115200);
	checktime1 = 0;
	clksTimer = 0;

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

//void enc_selector(uint16_t z, uint16_t pos) {
//	if (z == 1){
//		display.clearDisplay();
//		display.setCursor(16, 2);
//		display.print("ENC: ");
//		display.print(pos);
//		display.display();
//	}
//}


// MIDI LEDS

void midi_leds() {
	if (midiAUX){
		strip.setPixelColor(0, HALFRED);
	} else {
		strip.setPixelColor(0, LEDOFF);
	}
	dirtyPixels = true;
}

// SEQUENCER LEDS
void show_current_step(int patternNum) {
	
	if (msec >= blinkInterval){
		blinkState = !blinkState;
		msec = 0;
	}

	if (playing && blinkState){
		strip.setPixelColor(0, HALFWHITE);
	} else if (noteSelect){
		strip.setPixelColor(0, CYAN);
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
				} else if (j == patternNum+3){  	// pattern select
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

				// 
				//stepNote[patternNum][seqPos[patternNum]] = notes[thisKey]
				if(i % 4 == 0){ // mark groups of 4
					if(i == seqPos[patternNum]){
						strip.setPixelColor(i+11, HALFRED); // step chase
					} else if (stepPlay[patternNum][i] == 1){
						strip.setPixelColor(i+11, seqColors[patternNum]); // step on
					} else {
						strip.setPixelColor(i+11, LOWWHITE); 
					}
				} else if (i == seqPos[patternNum]){
					strip.setPixelColor(i+11, HALFRED); // step chase
				} else if (stepPlay[patternNum][i] == 1){
					strip.setPixelColor(i+11, seqColors[patternNum]); // step on
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
	//  usbMIDI.sendNoteOn(sequencer[current_step].note, VELOCITY, CHANNEL);
}

void step_off(int patternNum){
	//	Serial.print(seqPos);
	//	Serial.println(" step off");
	//  usbMIDI.sendNoteOff(sequencer[current_step].note, VELOCITY, CHANNEL);
}

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
	display.setCursor(74, 16);
	display.print("BPM:");
	display.setCursor(100, 16);
	display.print((int)clockbpm);

}
void dispNotes(){
	display.setTextSize(1);
	display.setCursor(0, 16);
	display.print("Note:");
	display.print(lastNote[playingPattern]);

}

void readPots(){
	// --- READ POTS to MIDI ---
	if (pots_msec >= 30) {
		pots_msec = 0;
		for(int i=0; i<5; i++) {
			analogValues[i] = analogRead(analogPins[i]) / 8;
			EMA_S[i] = (EMA_a[i]*analogValues[i]) + ((1-EMA_a[i])*EMA_S[i]); // Filtered result
		}
		for(int i=0; i<5; i++) {
			if (EMA_S[i] != previous[i]){
				usbMIDI.sendControlChange(pots[i], analogValues[i], midiChannel);
				MIDI.sendControlChange(pots[i], analogValues[i], midiChannel);
				previous[i] = EMA_S[i]; //analogValues[i];
				potCC = pots[i];
				potVal = analogValues[i];
			}
		}
		dirtyDisplay = true;
//		Serial.print("one:");
//		Serial.println(sinceTest1);

	}
}
// ####### MAIN LOOP #######

void loop() {
	customKeypad.tick();
	checktime1 = 0;

	// DISPLAY SETUP
	display.clearDisplay();
	display.setTextSize(1);
	display.setCursor(96, 0);
	display.print(modes[newmode]);				
				
	
	// ENCODER
	auto u = myEncoder.update();
	if (u.active()) {
    	auto amt = u.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)
//    	Serial.println(u.dir() < 0 ? "ccw " : "cw ");
//    	Serial.println(amt);
    	

		// Change Mode
    	if (enc_edit) {
	    	newmode = constrain(newmode + amt, 0, 2);
	    	
		} else {
			newtempo = constrain(clockbpm + amt, 40, 300);
			if (newtempo != clockbpm){
			
				// SET TEMPO HERE
				clockbpm = newtempo;
				dirtyDisplay = true;
			}
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
				dirtyDisplay = true;
			}
			enc_edit = !enc_edit;

			break;
		case Button::DownLong: //Serial.println("Button downlong"); 
			break;
		case Button::Up: //Serial.println("Button up"); 
			break;
		case Button::UpLong: //Serial.println("Button uplong"); 
			break;
	}
				

	// BPM tempo to step-delay calculation
	step_delay = 60000 / clockbpm / 4; // 16th notes
	// BPM to clock pulses
	clksDelay = (60000000 / clockbpm) / 24;
	if (clksTimer > clksDelay ) {
		// SEND CLOCK
	  clksTimer = 0;
	}


	
	// ############### POTS ###############
	switch(mode) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
	} // END POTS SWITCH


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
//					MIDI.sendControlChange(25, 100, midiChannel);
					if (midiAUX) {
						// STOP CLOCK
					} else {
						// START CLOCK
					}
					midiAUX = !midiAUX;
					
				} else if (e.bit.EVENT == KEY_JUST_RELEASED && thisKey == 0) { 
					// Hard coded Organelle stuff
					usbMIDI.sendControlChange(25, 0, midiChannel);
//					MIDI.sendControlChange(25, 0, midiChannel);
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
					if (noteSelect && thisKey > 10){
						if (noteSelection) {
							// blink ??
							selectedNote = thisKey;
							stepNote[playingPattern][selectedStep] = notes[thisKey];
								
						} else {
							selectedStep = keyPos; //set selection to this step
							noteSelection = true;
						}
					} else {					
						// Black Keys
						if (thisKey > 2 && thisKey < 11) { // pattern select
							playingPattern = thisKey-3;
							
						} else if (thisKey == 1) { 
							noteSelect = !noteSelect; // toggle noteSelect 

						} else if (thisKey == 2) { 
							seqReset(); // reset all sequences to step 1
							
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
			readPots();
			if (dirtyDisplay){
				dispPots();
				dispTempo();
				dispNotes();
			}
			break;
		case 1: 		// SEQUENCER 1
			readPots();
			if (dirtyDisplay){
				dispPattLen();
				dispTempo();		
			}
			if(playing == true) {
				if(step_interval[playingPattern] > step_delay){
					lastStepTime[playingPattern] = step_interval[playingPattern];
					step_ahead(playingPattern);
					step_on(playingPattern);
					playNote(playingPattern);
					step_interval[playingPattern] =0;
				} else if(step_interval[playingPattern] > step_delay / 2){
					step_off(playingPattern);
				}
				show_current_step(playingPattern);
			} else {
				show_current_step(playingPattern);
			}
			if (playing) {
				for (int z=0; z<16; z++){
				}
			}
			break;
		case 2: 		// SEQUENCER 2
			readPots();
			if (dirtyDisplay){
				dispPattLen();
				dispTempo();		
			}
			if(playing == true) {
				for (int j=0; j<8; j++){
//					Serial.print("pattern:");
//					Serial.println(j);

					if(step_interval[j] > step_delay){
						lastStepTime[j] = step_interval[j];
						step_on(j);
						playNote(j);
						step_ahead(j);
						step_interval[j] = 0;
					} else if(step_interval[playingPattern] > step_delay / 2){
						step_off(j);
					}
				}				
				show_current_step(playingPattern);
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
	
} // END MAIN LOOP

void seqReset(){
	for (int k=0; k<8; k++){
		seqPos[k] = 0;
	}
}
void noteOn(int notenum, int velocity, int patternNum){
	usbMIDI.sendNoteOn(notes[notenum], velocity, midiChannel);
	lastNote[patternNum] = notes[notenum];
//	MIDI.sendNoteOn(notes[notenum], velocity, midiChannel);
	pitchCV = map (notes[notenum], 35, 90, 0, 4096);
	digitalWrite(13, HIGH);
	analogWrite(A14, pitchCV);
	strip.setPixelColor(notenum, HALFWHITE);         //  Set pixel's color (in RAM)
	dirtyPixels = true;	
	dirtyDisplay = true;
}
void noteOff(int notenum){
	usbMIDI.sendNoteOff(notes[notenum], 0, midiChannel);
//	MIDI.sendNoteOff(notes[notenum], 0, midiChannel);
	digitalWrite(13, LOW);
	analogWrite(A14, 0);
	strip.setPixelColor(notenum, LEDOFF); 
	dirtyPixels = true;
	dirtyDisplay = true;
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
	dirtyPixels = true;
}
