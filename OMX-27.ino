// OMX-27 MIDI KEYBOARD / SEQUENCER
// v 1.1.1
// 
// Steven Noreyko, May 2021
//
//
//	Big thanks to: 
//	John Park and Gerald Stevens for initial testing and feature ideas
//	mzero for immense amounts of code coaching/assistance
//	drjohn for support
//  Additional code contributions: Matt Boone, wavefiler

// HW_VERSIONS
#define DEV			1
#define MIDIONLY	0


#include <Adafruit_Keypad.h>
#include <Adafruit_NeoPixel.h>
#include <ResponsiveAnalogRead.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <EEPROM.h>

#include "consts.h"
#include "config.h"
#include "colors.h"
#include "MM.h"
#include "ClearUI.h"
#include "sequencer.h"
#include "noteoffs.h"


U8G2_FOR_ADAFRUIT_GFX u8g2_display;

const int potCount = 5;
ResponsiveAnalogRead *analog[potCount];

// storage of pot values; current is in the main loop; last value is for midi output
int volatile currentValue[potCount];
int lastMidiValue[potCount];
int potMin = 0;
int potMax = 8190;
int temp;

// Timers and such
elapsedMillis msec = 0;
elapsedMillis pots_msec = 0;
elapsedMillis dialogTimeout = 0;
elapsedMicros clksTimer = 0;
unsigned long clksDelay;
elapsedMillis keyPressTime[27] = {0};
//bool step_dirty[] = {false,false,false,false,false,false,false,false};

volatile unsigned long ppqInterval; 

using Micros = unsigned long;
Micros lastProcessTime;
Micros nextStepTime;
Micros lastStepTime;
volatile unsigned long step_micros; 
volatile unsigned long noteon_micros;
volatile unsigned long noteoff_micros;

// ANALOGS
int analogValues[] = {0,0,0,0,0};		// default values
int potValues[] = {0,0,0,0,0};			
int potCC = pots[0];
int potVal = analogValues[0];
int potNum = 0;
bool plockDirty[] = {false,false,false,false,false};
int prevPlock[] = {0,0,0,0,0};

// MODES
OMXMode omxMode = DEFAULT_MODE;
OMXMode newmode = DEFAULT_MODE;
int nsmode = 4;
int nsmode2 = 4;
int nspage = 0;
int ppmode = 3;
int patmode = 0;
int mimode = 0;
int sqmode = 0;


// VARIABLES / FLAGS
float step_delay;
bool dirtyPixels = false;
bool dirtyDisplay = false;
bool blinkState = false;
bool noteSelect = false;
bool noteSelection = false;
bool patternParams = false;
int noteSelectPage = 0;
int selectedNote = 0;
int selectedStep = 0;
bool stepSelect = false;
bool stepRecord = false;
bool stepDirty = false;
bool dialogFlags[] = {false, false, false, false, false, false};
unsigned dialogDuration = 1000;

bool copiedFlag = false;
bool pastedFlag = false;
bool clearedFlag = false;

bool midiAUX = false;
bool enc_edit = false;
int noteon_velocity = 100;
int octave = 0; // default C4 is 0 - range is -4 to +5
int newoctave = octave;
int transpose = 0;
int rotationAmt = 0;
int hline = 8;
// CV 
int pitchCV;
uint8_t RES;
uint16_t AMAX;
int V_scale;

// clock
float clockbpm = 120;
float newtempo = clockbpm;
unsigned long tempoStartTime, tempoEndTime;

unsigned long blinkInterval = clockbpm * 2;
unsigned long longPressInterval = 1500;

uint8_t swing = 0;
uint8_t maxswing = 5;

bool keyState[27] = {false};
int midiKeyState[27] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};


// ENCODER
Encoder myEncoder(12, 11); 	// encoder pins on hardware
Button encButton(0);		// encoder button pin on hardware
//long newPosition = 0;
//long oldPosition = -999;


//initialize an instance of class Keypad
Adafruit_Keypad customKeypad = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

// Declare NeoPixel strip object
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


// ####### CLOCK/TIMING #######

void advanceClock(Micros advance) {
	static Micros timeToNextClock = 0;
	while (advance >= timeToNextClock) {
		advance -= timeToNextClock;		

		MM::sendClock();
		timeToNextClock = ppqInterval * (PPQ / 24);
	}
	timeToNextClock -= advance;
}
void advanceSteps(Micros advance) {
	static Micros timeToNextStep = 0;
//	static Micros stepnow = micros();	
	while (advance >= timeToNextStep) {
		advance -= timeToNextStep;		
		timeToNextStep = ppqInterval;

		// turn off any expiring notes
		pendingNoteOffs.play(micros());
				
		// turn on any pending notes
		pendingNoteOns.play(micros());
	}
	timeToNextStep -= advance;
}


void resetClocks(){
	// BPM tempo to step_delay calculation
	ppqInterval = 60000000/(PPQ * clockbpm); 		// ppq interval is in microseconds
	step_micros = ppqInterval * (PPQ/4); 			// 16th note step in microseconds (quarter of quarter note)
	
	// 16th note step length in milliseconds
	step_delay = step_micros * 0.001; 	// ppqInterval * 0.006; // 60000 / clockbpm / 4; 
}



// ####### POTENTIMETERS #######

void sendPots(int val, int channel){
	MM::sendControlChange(pots[val], analogValues[val], channel);
	potCC = pots[val];
	potVal = analogValues[val];
	potValues[val] = potVal;
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
			switch(omxMode) { 
				case MODE_OM:
						// fall through - same as MIDI
				case MODE_MIDI: // MIDI
					sendPots(k, midiChannel);
					dirtyDisplay = true;
					break;    	

				case MODE_S2: // SEQ2
						// fall through - same as SEQ1
				case MODE_S1: // SEQ1
					if (noteSelect && noteSelection){ // note selection - do P-Locks
						potNum = k;
						potCC = pots[k];
						potVal = analogValues[k];
								// stepNoteP[8] {notenum,vel,len,p1,p2,p3,p4,p5}
						if (k < 4){ // only store p-lock value for first 4 knobs
							stepNoteP[playingPattern][selectedStep].params[k] = analogValues[k];
						}
						sendPots(k, PatternChannel(playingPattern));
						dirtyDisplay = true;
						
					} else if (!noteSelect){
						sendPots(k, PatternChannel(playingPattern));
					}
					break;  

				default:
					break;  	
    		}
    	}
	}
}



// ####### SETUP #######

void setup() {
	Serial.begin(115200);

	dialogTimeout = 0;
	clksTimer = 0;
		
	lastProcessTime = micros();
	resetClocks();

	nextStepTime = micros();
	lastStepTime = micros();
	
	// SET ANALOG READ resolution to teensy's 13 usable bits
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

	// HW MIDI
	MM::begin();
		
	//CV gate pin
	pinMode(CVGATE_PIN, OUTPUT); 
		
  	// set DAC Resolution CV/GATE
    RES = 12;
    analogWriteResolution(RES); // set resolution for DAC
    	AMAX = pow(2,RES);
    	V_scale = 64; // pow(2,(RES-7)); 4095 max
    analogWrite(CVPITCH_PIN, 0);

	// Load from EEPROM
	bool bLoaded = loadFromEEPROM();
	if ( !bLoaded )
	{
		// Failed to load due to initialized EEPROM or version mismatch
		// defaults
		omxMode = DEFAULT_MODE;
		playingPattern = 0;
		midiChannel = 1;
		pots[0] = CC1;
		pots[1] = CC2;
		pots[2] = CC3;
		pots[3] = CC4;
		pots[4] = CC5;
		initPatterns();
	}

  	// Init Display
	initializeDisplay();
	u8g2_display.begin(display);
	
	// Startup screen		
	display.clearDisplay();
	testdrawrect();
	delay(200);
	display.clearDisplay();
	u8g2_display.setForegroundColor(WHITE);
	u8g2_display.setBackgroundColor(BLACK);
	drawLoading();

	// Keypad
	customKeypad.begin();

	//LEDs
	strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
	strip.show();            // Turn OFF all pixels ASAP
	strip.setBrightness(LED_BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)
	for(int i=0; i<LED_COUNT; i++) { // For each pixel...
		strip.setPixelColor(i, HALFWHITE);
		strip.show();   // Send the updated pixel colors to the hardware.
		delay(5); // Pause before next pass through loop
	}
	rainbow(5); // rainbow startup pattern
	delay(500);
	
	// clear LEDs
	strip.fill(0, 0, LED_COUNT);
	strip.show();

	delay(100);

	// Clear display
	display.display();			

	dirtyDisplay = true;
}

// ####### END SETUP #######



// ####### MIDI LEDS #######

void midi_leds() {
	if (midiAUX){
		strip.setPixelColor(0, MEDRED);
	} else {
		strip.setPixelColor(0, LEDOFF);
	}
	dirtyPixels = true;
}

// ####### SEQUENCER LEDS #######

void show_current_step(int patternNum) {
	blinkInterval = step_delay*2;
	
	if (msec >= blinkInterval){
		blinkState = !blinkState;
		msec = 0;
	}

	// AUX KEY

	if (playing && blinkState){
		strip.setPixelColor(0, WHITE);
	} else if (noteSelect && blinkState){
		strip.setPixelColor(0, NOTESEL);
	} else if (patternParams && blinkState){
		strip.setPixelColor(0, seqColors[patternNum]);
	} else if (stepRecord && blinkState){
		strip.setPixelColor(0, seqColors[patternNum]);
	} else {
		switch(omxMode){
			case MODE_S1:
				strip.setPixelColor(0, SEQ1C);
				break;
			case MODE_S2:
				strip.setPixelColor(0, SEQ2C);
				break;
			default:
				strip.setPixelColor(0, LEDOFF);
				break;
		}
	}

	if (patternSettings[patternNum].mute) {
		stepColor = muteColors[patternNum];
	} else {
		stepColor = seqColors[patternNum];
	}

	if (noteSelect && noteSelection) {
		for(int j = 1; j < NUM_STEPS+11; j++){
			if (j < PatternLength(patternNum)+11){
				if (j == selectedNote){
					strip.setPixelColor(j, HALFWHITE);
				} else if (j == selectedStep+11){
					strip.setPixelColor(j, SEQSTEP);
				} else{
					strip.setPixelColor(j, LEDOFF);
				}
			} else {
				strip.setPixelColor(j, LEDOFF);
			}
		}
		
	} else if (stepRecord) {
		for(int j = 1; j < NUM_STEPS+11; j++){
			if (j < PatternLength(patternNum)+11){
				if (j == seqPos[playingPattern]+11){ 
					strip.setPixelColor(j, SEQCHASE);
//				} else if (j == selectedNote){
//					strip.setPixelColor(j, HALFWHITE);
				} else if (j != selectedNote){
					strip.setPixelColor(j, LEDOFF);
				}
			} else  {
				strip.setPixelColor(j, LEDOFF);
			}
		}
		
	} else {
		for(int j = 1; j < NUM_STEPS+11; j++){		
			if (j < PatternLength(patternNum)+11){
				if (j == 1) {								

					// NOTE SELECT
					if (keyState[j] && blinkState){
						strip.setPixelColor(j, LEDOFF);
					} else {
						strip.setPixelColor(j, FUNKONE);
					}
				} else if (j == 2) {

					// PATTERN PARAMS
					if (keyState[j] && blinkState){
						strip.setPixelColor(j, LEDOFF);
					} else {
						strip.setPixelColor(j, FUNKTWO);
					}
					
				} else if (j == patternNum+3){  			// PATTERN SELECT
					strip.setPixelColor(j, stepColor); 
					if (patternParams && blinkState){
						strip.setPixelColor(j, LEDOFF);						
					}
				} else {
					strip.setPixelColor(j, LEDOFF);
				}
			} else {
				strip.setPixelColor(j, LEDOFF);
			}
		}

		for(int i = 0; i < NUM_STEPS; i++){
			if (i < PatternLength(patternNum)){
				if(i % 4 == 0){ // mark groups of 4
					if(i == seqPos[patternNum]){
						if (playing){
							strip.setPixelColor(i+11, SEQCHASE); // step chase
						} else if (stepNoteP[patternNum][i].stepType == STEPTYPE_PLAY){
							strip.setPixelColor(i+11, stepColor); // step on color
						} else {
							strip.setPixelColor(i+11, SEQMARKER); 
						}

					} else if (stepNoteP[patternNum][i].stepType == STEPTYPE_PLAY){
						strip.setPixelColor(i+11, stepColor); // step on color
					} else {
						strip.setPixelColor(i+11, SEQMARKER); 
					}
				} else if (i == seqPos[patternNum]){
					if (playing){
						strip.setPixelColor(i+11, SEQCHASE); // step chase
					} else if (stepNoteP[patternNum][i].stepType == STEPTYPE_PLAY){
						strip.setPixelColor(i+11, stepColor); // step on color
					} else {
						strip.setPixelColor(i+11, LEDOFF);  // DO WE NEED TO MARK PLAYHEAD WHEN STOPPED?
					}

				} else if (stepNoteP[patternNum][i].stepType == STEPTYPE_PLAY){

					strip.setPixelColor(i+11, stepColor); // step on color

				} else {
					strip.setPixelColor(i+11, LEDOFF); 
				}
			}
		}
	}
	dirtyPixels = true;
//	strip.show();
}
// ####### END LEDS




// ####### DISPLAY FUNCTIONS #######

void dispGridBoxes(){
	display.fillRect(0, 0, gridw, 10, WHITE);
	display.drawFastVLine(gridw/4, 0, gridh, INVERSE);
	display.drawFastVLine(gridw/2, 0, gridh, INVERSE);
	display.drawFastVLine(gridw*0.75, 0, gridh, INVERSE);
}
void invertColor(bool flip){
	if (flip) {
		u8g2_display.setForegroundColor(BLACK);
		u8g2_display.setBackgroundColor(WHITE);
	} else {
		u8g2_display.setForegroundColor(WHITE);
		u8g2_display.setBackgroundColor(BLACK);
	}
}
void dispValBox(int v, int16_t n, bool inv){			// n is box 0-3
	invertColor(inv);
	u8g2centerNumber(v, n*32, hline*2+5, 32, 22);
}

void dispSymbBox(const char* v, int16_t n, bool inv){			// n is box 0-3
	invertColor(inv);
	u8g2centerText(v, n*32, hline*2+6, 32, 22);
}

void dispGenericMode(int submode, int selected){
	const char* legends[4] = {"","","",""};
	int legendVals[4] = {0,0,0,0};
	switch(submode){
		case SUBMODE_MIDI:
			legends[0] = "OCT";
			legends[1] = "CH";
			legends[2] = "CC";
			legends[3] = "NOTE";
			legendVals[0] = (int)octave+4;
			legendVals[1] = midiChannel;
			legendVals[2] = potVal;
			legendVals[3] = midiLastNote;
			break;
		case SUBMODE_SEQ:
			legends[0] = "PTN";
			legends[1] = "LEN";
			legends[2] = "TRSP";
			legends[3] = "BPM";
			legendVals[0] = playingPattern+1;
			legendVals[1] = PatternLength(playingPattern);
			legendVals[2] = (int)transpose;
			legendVals[3] = (int)clockbpm;
			break;
		default:
			break;
	}
	
	
	u8g2_display.setFontMode(1);  
	u8g2_display.setFont(FONT_LABELS);
	u8g2_display.setCursor(0, 0);	
	dispGridBoxes();

	// labels
	u8g2_display.setForegroundColor(BLACK);
	u8g2_display.setBackgroundColor(WHITE);

	for (int j= 0; j<4; j++){
		u8g2centerText(legends[j], (j*32) + 1, hline-2, 32, 10);
	}

	// value text formatting
	u8g2_display.setFontMode(1); 
	u8g2_display.setFont(FONT_VALUES);
	u8g2_display.setForegroundColor(WHITE);
	u8g2_display.setBackgroundColor(BLACK);
	

	switch(selected){
		case 0: 	//
			display.fillRect(0*32+2, 11, 29, 21, WHITE);
			break;
		case 1: 	//
			display.fillRect(1*32+2, 11, 29, 21, WHITE);
			break;
		default:
			break;
	}
	// ValueBoxes
	int highlight = false;
	for (int j = 0; j<4; j++){
		
		if (j == selected) {
			highlight = true;
		}else{
			highlight = false;
		}
		dispValBox(legendVals[j], j, highlight);
	}

		
}


void dispMidiMode(){
	// value text formatting
	u8g2_display.setFontMode(1); 
	u8g2_display.setFont(FONT_VALUES);
	u8g2_display.setForegroundColor(WHITE);
	u8g2_display.setBackgroundColor(BLACK);
	
	// ValueBoxes
	dispValBox(potVal, 0, false);
	dispValBox(midiLastNote, 1, false);

	bool octFlip = false;		
	bool chFlip = false;		
	switch(mimode){
		case 0: 	//
			display.fillRect(2*32, 11, 33, 22, WHITE);
			octFlip = true;
			break;
		case 1: 	//
			display.fillRect(3*32, 11, 33, 22, WHITE);
			chFlip = true;
			break;
		default:
			break;
	}

	dispValBox((int)octave+4, 2, octFlip);
	dispValBox(midiChannel, 3, chFlip);
	dispGridBoxes();

	u8g2_display.setFontMode(1);  
	u8g2_display.setFont(FONT_LABELS);
	u8g2_display.setCursor(0, 0);
	
	// labels
	u8g2_display.setForegroundColor(BLACK);
	u8g2_display.setBackgroundColor(WHITE);

	u8g2_display.setCursor(7, hline);
	u8g2_display.print("CC");
	u8g2_display.print(potCC);

	u8g2centerText("NOTE", 33, hline-2, 32, 10);
	u8g2centerText("OCT", 65, hline-2, 32, 10);
	u8g2centerText("CH", 97, hline-2, 32, 10);
	
}

void dispSeqMode1(){
	u8g2_display.setFontMode(1);  
	u8g2_display.setFont(FONT_LABELS);
	u8g2_display.setCursor(0, 0);	
	dispGridBoxes();
	// labels
	u8g2_display.setForegroundColor(BLACK);
	u8g2_display.setBackgroundColor(WHITE);

	u8g2centerText("PTN", 1, hline-2, 32, 10);
	u8g2centerText("LEN", 33, hline-2, 32, 10);
	u8g2centerText("TRSP", 65, hline-2, 32, 10);
	u8g2centerText("BPM", 97, hline-2, 32, 10);

	// value text formatting
	u8g2_display.setFontMode(1); 
	u8g2_display.setFont(FONT_VALUES);
	u8g2_display.setForegroundColor(WHITE);
	u8g2_display.setBackgroundColor(BLACK);
	
	// ValueBoxes
	bool trspFlip = false;			
	switch(sqmode){
		case 1: 	//
			display.fillRect(2*32, 10, 33, 22, WHITE);
			trspFlip = true;
			break;
		default:
			break;
	}
		
	dispValBox(playingPattern+1, 0, false);
	dispValBox(PatternLength(playingPattern), 1, false);
	dispValBox((int)transpose, 2, trspFlip);
	dispValBox((int)clockbpm, 3, false);
}

void dispStepRec(){
	u8g2_display.setFontMode(1);  
	u8g2_display.setFont(FONT_LABELS);
	u8g2_display.setCursor(0, 0);
	
	dispGridBoxes();

	// labels
	u8g2_display.setForegroundColor(BLACK);
	u8g2_display.setBackgroundColor(WHITE);

	u8g2centerText("PATT", 0, hline-2, 32, 10);
	u8g2centerText("STEP", 33, hline-2, 32, 10);
	u8g2centerText("NOTE", 65, hline-2, 32, 10);
	u8g2centerText("OCT", 97, hline-2, 32, 10);
	
	// value text formatting
	u8g2_display.setFontMode(1); 
	u8g2_display.setFont(FONT_VALUES);
	u8g2_display.setForegroundColor(WHITE);
	u8g2_display.setBackgroundColor(BLACK);
	
	// ValueBoxes
	dispValBox(playingPattern+1, 0, false);
	dispValBox(seqPos[playingPattern]+1, 1, false);
	dispValBox(stepNoteP[playingPattern][selectedStep].note, 2, false);
	dispValBox((int)octave+4, 3, false);

}

void dispNoteSelect(){
	if (!noteSelection){

	}else{
		// labels formatting
		u8g2_display.setFontMode(1);  
		u8g2_display.setFont(FONT_LABELS);
		u8g2_display.setCursor(0, 0);	
		dispGridBoxes();
		display.drawFastHLine(0, 20, 64, WHITE);
		// labels
		u8g2_display.setForegroundColor(BLACK);
		u8g2_display.setBackgroundColor(WHITE);

		u8g2centerText("L-1/2", 0, hline-2, 32, 10);
		u8g2centerText("L-3/4", 32, hline-2, 32, 10);
//		u8g2centerText("NOTE", 65, hline-2, 32, 10);
//		u8g2centerText("VEL", 97, hline-2, 32, 10);

		// value text formatting
		u8g2_display.setFontMode(1); 

		bool ccFlip[] = {false,false,false,false};
		switch(nsmode){
			case 0:  	// CC1
				display.fillRect(0*32, 11, 33, 11, WHITE);
				ccFlip[0] = true;
				break;
			case 1: 	// CC2
				display.fillRect(0*32, 11*2-1, 33, 11, WHITE);
				ccFlip[1] = true;
				break;
			case 2: 	// CC3
				display.fillRect(1*32, 10, 33, 11, WHITE);
				ccFlip[2] = true;
				break;
			case 3: 	// CC4
				display.fillRect(1*32, 11*2-1, 33, 11, WHITE);
				ccFlip[3] = true;
				break;
			default:
				break;
		}

		u8g2_display.setFont(FONT_LABELS);		
		for (int j=0; j<4; j++){
			char tempText[4];

			int stepNoteParam = stepNoteP[playingPattern][selectedStep].params[j];
			if (stepNoteParam > -1){
				itoa( stepNoteParam, tempText, 10 );
			} else {
				snprintf( tempText, sizeof(tempText), "---");
			}
			// this is ugly
			int xoffset = 0;
			int yoffset = 0;
			if (j==1 || j==3){
				yoffset = 11;
			}
			if (j==2 || j==3){
				xoffset = 32; 
			}
			invertColor(ccFlip[j]);
			u8g2centerText(tempText, xoffset, hline*2+yoffset, 32, 11); 	// CC VALUES
		}

		u8g2_display.setFont(FONT_VALUES);
		// ValueBoxes

	}
}

void dispNoteSelect2(){
	// labels formatting
	u8g2_display.setFontMode(1);  
	u8g2_display.setFont(FONT_LABELS);
	u8g2_display.setCursor(0, 0);	
	dispGridBoxes();
	// labels
	u8g2_display.setForegroundColor(BLACK);
	u8g2_display.setBackgroundColor(WHITE);

	u8g2centerText("NOTE", 0, hline-2, 32, 10);
	u8g2centerText("OCT", 32, hline-2, 32, 10);
	u8g2centerText("VEL", 65, hline-2, 32, 10);
	u8g2centerText("LEN", 97, hline-2, 32, 10);

	// value text formatting
	u8g2_display.setFontMode(1); 

	bool lenFlip = false, octFlip = false, noteFlip = false, velFlip = false;		
	switch(nsmode2){
		case 0: 	//
			display.fillRect(0*32, 10, 33, 22, WHITE);
			noteFlip = true;
			break;
		case 1: 	//
			display.fillRect(1*32, 10, 33, 22, WHITE);
			octFlip = true;
			break;
		case 2: 	//
			display.fillRect(2*32, 10, 33, 22, WHITE);
			velFlip = true;
			break;
		case 3: 	//
			display.fillRect(3*32, 10, 33, 22, WHITE);
			lenFlip = true;
			break;
		default:
			break;
	}

	u8g2_display.setFont(FONT_VALUES);
	// ValueBoxes
	dispValBox(stepNoteP[playingPattern][selectedStep].note, 0, noteFlip); 		// NOTE NUM
	dispValBox((int)octave+4, 1, octFlip); 		// OCTAVE
	dispValBox(stepNoteP[playingPattern][selectedStep].vel, 2, velFlip); 	// VELOCITY
	dispValBox(stepNoteP[playingPattern][selectedStep].len + 1, 3, lenFlip); 	// NOTE LENGTH
}

void dispPatternParams(){
	if (patternParams){

		// values formatting
		u8g2_display.setFontMode(1); 
		u8g2_display.setFont(FONT_VALUES);
		
		bool pattFlip = false, lenFlip = false, rotFlip = false, chFlip = false;		
		switch(ppmode){
			case 0:  // LEN
				display.fillRect(1*32, 11, 33, 22, WHITE);
				lenFlip = true;
				break;
			case 1: 	// ROTATE
				display.fillRect(2*32, 11, 33, 22, WHITE);
				rotFlip = true;
				break;
			case 2: 	// CHANNEL
				display.fillRect(3*32, 11, 33, 22, WHITE);
				chFlip = true;
				break;
			default:
				break;
		}

	// ValueBoxes
		dispValBox(playingPattern+1, 0, pattFlip); // PAT
		dispValBox(PatternLength(playingPattern), 1, lenFlip); // LEN
		dispValBox(rotationAmt, 2, rotFlip); // LEN
		dispValBox(PatternChannel(playingPattern), 3, chFlip); // CHANNEL
	
//		u8g2_display.setFont(FONT_SYMB);
//		invertColor(rotFlip);
//		u8g2centerText("\u25C1\u25B7", 2*32, hline*2+6, 32, 22); // "\u00BB\u00AB" // // dice: "\u2685"

		// labels formatting
		u8g2_display.setFontMode(1);  
		u8g2_display.setFont(FONT_LABELS);
		u8g2_display.setCursor(0, 0);	
		dispGridBoxes();
		// labels
		u8g2_display.setForegroundColor(BLACK);
		u8g2_display.setBackgroundColor(WHITE);

	// ValueBoxLabels
		u8g2centerText("PTN", 0, hline-2, 32, 10);
		u8g2centerText("LEN", 32, hline-2, 32, 10);
		u8g2centerText("ROT", 65, hline-2, 32, 10);
		u8g2centerText("CHAN", 97, hline-2, 32, 10);
		
	}
}

void dispInfoDialog(){			
	for (int key=0; key < NUM_DIALOGS; key++){ 	
		if (infoDialog[key].state){ 
			// reset timer
			dialogTimeout = 0;
			display.clearDisplay();
			u8g2_display.setFontMode(1);  
			u8g2_display.setFont(FONT_TENFAT);
			u8g2_display.setForegroundColor(WHITE);
			u8g2_display.setBackgroundColor(BLACK);
			
			u8g2centerText(infoDialog[key].text, 0, 10, 128, 32);
			infoDialog[key].state = false;
		}
	}		
}

void dispMode(){
	// labels formatting
	u8g2_display.setFontMode(1);  
	u8g2_display.setFont(FONT_BIG);
	u8g2_display.setCursor(0, 0);	

	u8g2_display.setForegroundColor(WHITE);
	u8g2_display.setBackgroundColor(BLACK);
	
	const char* displaymode = "";
	if (newmode != omxMode && enc_edit) {
		displaymode = modes[newmode]; // display.print(modes[newmode]);
	} else if (enc_edit) {
		displaymode = modes[omxMode]; // display.print(modes[mode]);
	}
	u8g2centerText(displaymode, 86, 20, 44, 32);
}

void dispPattLen(){
	display.setCursor(1, 19);
	display.setTextSize(1);
	display.print("LEN");	
	display.setCursor(29, 18);
	display.setTextSize(2);
	display.print(PatternLength(playingPattern));
}
void dispPattStrt(){
	display.setCursor(1, 19);
	display.setTextSize(1);
	display.print("SRT");	
	display.setCursor(29, 18);
	display.setTextSize(2);
	display.print(PatternLength(playingPattern));
}

void dispPatt(){
	display.setCursor(0, 0);
	display.setTextSize(1);
	display.print("PTN");		
	display.setCursor(30, 0);
	display.setTextSize(2);
	display.print(playingPattern+1);
}

void dispTempo(){
	display.setCursor(65, 19);
	display.setTextSize(1);
	display.print("BPM");		
	display.setCursor(92, 18);
	display.setTextSize(2);
	display.print((int)clockbpm);
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
}


// ############## MAIN LOOP ##############

void loop() {
	customKeypad.tick();
	clksTimer = 0;
	
	Micros now = micros();
	Micros passed = now - lastProcessTime;
	lastProcessTime = now;
	
	if (passed > 0) {
		if (playing){
			advanceClock(passed);
			advanceSteps(passed);
		}
	}
	doStep();
	
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
//			int modesize = NUM_OMX_MODES;
//			Serial.println(modesize);
	    	newmode = (OMXMode)constrain(newmode + amt, 0, NUM_OMX_MODES - 1);
	    	dispMode();
	    	dirtyDisplay = true;

		} else if (!noteSelect && !patternParams && !stepRecord){  
			switch(omxMode) { 
				case MODE_OM: // Organelle Mother
					if(u.dir() < 0){									// if turn ccw
						MM::sendControlChange(CC_OM2,0,midiChannel);
					} else if (u.dir() > 0){							// if turn cw
						MM::sendControlChange(CC_OM2,127,midiChannel);
					}    
  					dirtyDisplay = true;
					break;
				case MODE_MIDI: // MIDI			
					if (mimode == 1) { // set length
						int newchan = constrain(midiChannel + amt, 1, 16);
						if (newchan != midiChannel){
							midiChannel = newchan;
						}
						
					}else {
						// set octave 
						newoctave = constrain(octave + amt, -5, 4);
						if (newoctave != octave){
							octave = newoctave;
						}
					}
  					dirtyDisplay = true;
					break;
				case MODE_S1: // SEQ 1
					// FALL THROUGH
				case MODE_S2: // SEQ 2
					if (patmode == 1) { // set octave
						// set octave 
						newoctave = constrain(octave + amt, -5, 4);
						if (newoctave != octave){
							octave = newoctave;
						}						
					} else if (sqmode == 1){ 
						int newswing = constrain(swing + amt, 0, maxswing);
						swing = newswing;
						patternSettings[playingPattern].swing = newswing;
//						Serial.println(patternSettings[playingPattern].swing);
						
//						transposeSeq(playingPattern, amt);
//						int newtransp = transpose + amt;
//						transpose = newtransp;
						
					} else if (sqmode == 0){ 
						// otherwise set tempo
						newtempo = constrain(clockbpm + amt, 40, 300);
						if (newtempo != clockbpm){
							// SET TEMPO HERE
							clockbpm = newtempo;
							resetClocks();
						}
					}
  					dirtyDisplay = true;
					break;
				default:
					break;
			}

		} else if (noteSelect || patternParams || stepRecord) {  
			switch(omxMode) { // process encoder input depending on mode
				case MODE_MIDI: // MIDI
					break;
				case MODE_S1: // SEQ 1
						// FALL THROUGH
						
				case MODE_S2: // SEQ 2						
					if (patternParams && !enc_edit){ 		// SEQUENCE EDIT MODE
						//
						if (ppmode == 0) { 					// SET LENGTH
							SetPatternLength( playingPattern, constrain(PatternLength(playingPattern) + amt, 1, 16) );
						}	
						if (ppmode == 1) { 					// SET PATTERN ROTATION	
							int rotator;
							(u.dir() < 0 ? rotator = -1 : rotator = 1);					
//							int rotator = constrain(rotcc, (PatternLength(playingPattern))*-1, PatternLength(playingPattern));
							rotationAmt = rotationAmt + rotator;
							if (rotationAmt < 16 && rotationAmt > -16 ){
								rotatePattern(playingPattern, rotator);
							}
							rotationAmt = constrain(rotationAmt, (PatternLength(playingPattern)-1)*-1, PatternLength(playingPattern)-1);
						}	
						if (ppmode == 2) { 					// SET PATTERN CHANNEL	
							patternSettings[playingPattern].channel = constrain(patternSettings[playingPattern].channel + amt, 0, 15);
						}
						
					} else if (stepRecord && !enc_edit){
							// SET OCTAVE 
							newoctave = constrain(octave + amt, -5, 4);
							if (newoctave != octave){
								octave = newoctave;
							}						

					} else if (noteSelect && noteSelection && !enc_edit){
						// {notenum,vel,len,p1,p2,p3,p4,p5}

						if (nsmode >= 0 && nsmode < 4){
//							Serial.print("nsmode ");
//							Serial.println(nsmode);
							if(u.dir() < 0){				// RESET PLOCK IF TURN CCW
								stepNoteP[playingPattern][selectedStep].params[nsmode] = -1;
							}
						}
						if (nsmode == 4 && nsmode2 == 4) { 	// CHANGE PAGE
							nspage = constrain(nspage + amt, 0, 1);
//							Serial.print("nspage ");
//							Serial.println(nspage);
						}	

						if (nsmode2 == 0) { 				// SET NOTE NUM
							int tempNote = stepNoteP[playingPattern][selectedStep].note;
							stepNoteP[playingPattern][selectedStep].note = constrain(tempNote + amt, 0, 127);
						}	
						if (nsmode2 == 1) { 				// SET OCTAVE 
							newoctave = constrain(octave + amt, -5, 4);
							if (newoctave != octave){
								octave = newoctave;
							}						
						}	
						if (nsmode2 == 3) { 				// SET NOTE LENGTH
							int tempLen = stepNoteP[playingPattern][selectedStep].len;
							stepNoteP[playingPattern][selectedStep].len = constrain(tempLen + amt, 0, 15); // Note Len between 1-16
						}	
						if (nsmode2 == 2) { 				// SET VELOCITY
							int tempVel = stepNoteP[playingPattern][selectedStep].vel;
							stepNoteP[playingPattern][selectedStep].vel = constrain(tempVel + amt, 0, 127);
						}	
					} else {
						newtempo = constrain(clockbpm + amt, 40, 300);
						if (newtempo != clockbpm){
							// SET TEMPO HERE
							clockbpm = newtempo;
							resetClocks();
						}
					}
					dirtyDisplay = true;
					break;

				case MODE_OM: // Organelle Mother
					break;

				default:
					break;
			}
		}
	}
	// END ENCODER
	
	// ############### ENCODER BUTTON ###############
	//
	auto s = encButton.update();
	switch (s) {
		// SHORT PRESS
		case Button::Down: //Serial.println("Button down"); 

			// what page are we on?
			if (newmode != omxMode && enc_edit) {
				omxMode = newmode;
				seqStop();
				setAllLEDS(0,0,0);
				enc_edit = false;
				dispMode();
			} else if (enc_edit){
				enc_edit = false;
			}

			if(omxMode == MODE_MIDI) {
				// switch midi oct/chan selection
				mimode = !mimode;
			}
			if(omxMode == MODE_OM) {
				MM::sendControlChange(CC_OM1,100,midiChannel);									
			}
			if(omxMode == MODE_S1 || omxMode == MODE_S2) {
				if (noteSelect && noteSelection && !patternParams) {
					if (nspage == 1){
						// increment nsmode
						nsmode = (nsmode + 1 ) % 5;
					}else if (nspage == 0){
						nsmode2 = (nsmode2 + 1 ) % 5;
					}
				} else if (patternParams) {
					// increment ppmode
					ppmode = (ppmode + 1 ) % 4;
				} else if (stepRecord) {
					step_ahead(playingPattern);
					selectedStep = seqPos[playingPattern];
					
				} else {
					sqmode = !sqmode;
					//patmode = !patmode;					
				}
			}
			dirtyDisplay = true;
			break;
			
		// LONG PRESS
		case Button::DownLong: //Serial.println("Button downlong"); 
			if (stepRecord) {
				resetPatternDefaults(playingPattern);
				clearedFlag = true;
			} else {
				enc_edit = true;		
				newmode = omxMode;
				dispMode();
			}
			dirtyDisplay = true;
			
			break;
		case Button::Up: //Serial.println("Button up"); 
			if(omxMode == MODE_OM) {
				MM::sendControlChange(CC_OM1,0,midiChannel);											
			}
			break;
		case Button::UpLong: //Serial.println("Button uplong"); 
			break;
		default:
			break;		
	}
	// END ENCODER BUTTON
				

	// ############### KEY HANDLING ###############
	
	while(customKeypad.available()){
		keypadEvent e = customKeypad.read();
		int thisKey = e.bit.KEY;
		int keyPos = thisKey - 11;

		if (e.bit.EVENT == KEY_JUST_PRESSED){
			keyState[thisKey] = true;
		}

		if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey == 0 && enc_edit) {
			// temp - save whenever the 0 key is pressed in encoder edit mode
			saveToEEPROM();
			Serial.println("EEPROM saved");
		}
		
		switch(omxMode) {
			case MODE_OM: // Organelle
				// Fall Through		
				
			case MODE_MIDI: // MIDI CONTROLLER
		
				// ### KEY PRESS EVENTS
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey != 0) {
					//Serial.println(" pressed");
					midiNoteOn(thisKey, noteon_velocity);

				} else if(e.bit.EVENT == KEY_JUST_RELEASED && thisKey != 0) {
					//Serial.println(" released");
					midiNoteOff(thisKey);
				}
				
				// AUX KEY
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey == 0) {

					// Hard coded Organelle stuff
					MM::sendControlChange(CC_AUX, 100, midiChannel);
					if (midiAUX) {
						// STOP CLOCK
//						Serial.println("stop clock");

					} else {
						// START CLOCK
//						Serial.println("start clock");

					}
					midiAUX = !midiAUX;
					
				} else if (e.bit.EVENT == KEY_JUST_RELEASED && thisKey == 0) { 
					// Hard coded Organelle stuff
					MM::sendControlChange(CC_AUX, 0, midiChannel);
//					midiAUX = false;
				}					
				break;

			case MODE_S1: // SEQUENCER 1
				// fall through
				
			case MODE_S2: // SEQUENCER 2
				// Sequencer row keys

				// ### KEY PRESS EVENTS
				
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey != 0) {
					// set key timer to zero
					keyPressTime[thisKey] = 0;
					
					// NOTE SELECT
					if (noteSelect){
						if (noteSelection) {		// SET NOTE
							stepSelect = false;
							selectedNote = thisKey;
							int adjnote = notes[thisKey] + (octave * 12);
							stepNoteP[playingPattern][selectedStep].note = adjnote;
							if (!playing){
								seqNoteOn(thisKey, noteon_velocity, playingPattern);
							}
							// see RELEASE events for more
							dirtyDisplay = true;
														
						} else if (thisKey == 1) { 

						} else if (thisKey == 2) { 

						} else if (thisKey > 2 && thisKey < 11) { // Pattern select keys
							playingPattern = thisKey-3;
							dirtyDisplay = true;

						} else if ( thisKey > 10 ) {
							selectedStep = keyPos; // set noteSelection to this step
							stepSelect = true;
							noteSelection = true;
							dirtyDisplay = true;							
						}
						
					// PATTERN PARAMS 
					} else if (patternParams) {
						if (thisKey == 1) { 


						} else if (thisKey == 2) { 


						} else if (thisKey > 2 && thisKey < 11) { // Pattern select keys
							
							playingPattern = thisKey-3;

							// COPY / PASTE / CLEAR
							if (keyState[1] && !keyState[2]) { 	
								copyPattern(playingPattern);
								infoDialog[COPY].state = true; // copied flag
//								dialogFlags[0] = true; // copied flag
								Serial.print("copy: ");
								Serial.println(playingPattern);
							} else if (!keyState[1] && keyState[2]) {
								pastePattern(playingPattern);
								infoDialog[PASTE].state = true; // pasted flag
//								dialogFlags[1] = true; // pasted flag
								Serial.print("paste: ");
								Serial.println(playingPattern);							
							} else if (keyState[1] && keyState[2]) {
								clearPattern(playingPattern);
								infoDialog[CLEAR].state = true; // cleared flag
//								dialogFlags[2] = true; // cleared flag
							}
						
							dirtyDisplay = true;
						} else if ( thisKey > 10 ) {
							// set pattern length with key
							SetPatternLength( playingPattern, thisKey - 10);
							dirtyDisplay = true;
						}
					
					// STEP RECORD
					} else if (stepRecord) {
						selectedNote = thisKey;
						selectedStep = seqPos[playingPattern];
											
						int adjnote = notes[thisKey] + (octave * 12);
						stepNoteP[playingPattern][selectedStep].note = adjnote;

//							Serial.print("seqPos: ");
//							Serial.println(seqPos[playingPattern]);
//							Serial.print("selectedStep: ");
//							Serial.println(selectedStep);
//							Serial.print("adjnote: ");
//							Serial.println(adjnote);

						if (!playing){
							seqNoteOn(thisKey, noteon_velocity, playingPattern);
						} // see RELEASE events for more
						stepDirty = true;
						dirtyDisplay = true;

					// REGULAR SEQ MODE
					} else {					
						if (thisKey == 1) {	
//							seqResetFlag = true;					// RESET ALL SEQUENCES TO FIRST/LAST STEP 
																	// MOVED DOWN TO AUX KEY
//								Serial.print("set seqResetFlag: ");
//								Serial.println(seqResetFlag);

						} else if (thisKey == 2) { 					// CHANGE PATTERN DIRECTION
//							patternSettings[playingPattern].reverse = !patternSettings[playingPattern].reverse;

						// BLACK KEYS
						} else if (thisKey > 2 && thisKey < 11) { // Pattern select
						
							// CHECK keyState[] FOR LONG PRESS THINGS
							
							// If KEY 1 is down + pattern and not playing = STEP RECORD
							if (keyState[1] && !playing) { 		
								Serial.print("step record on - pattern: ");
								Serial.println(thisKey-3);
								playingPattern = thisKey-3;
								seqPos[playingPattern] = 0;
								stepRecord = true;
								dirtyDisplay = true;

							// If KEY 2 is down + pattern = PATTERN MUTE
							} else if (keyState[2]) { 		
//								Serial.print("mute ");
//								Serial.println(thisKey);
//								Serial.println(patternSettings[thisKey].mute);
								patternSettings[thisKey-3].mute = !patternSettings[thisKey-3].mute;
								
							} else {
								playingPattern = thisKey-3;
								dirtyDisplay = true;
							}
						
						// SEQUENCE 1-16 STEP KEYS
						} else if (thisKey > 10) { 
							// TOGGLE STEP ON/OFF
							if ( stepNoteP[playingPattern][keyPos].stepType == STEPTYPE_PLAY || stepNoteP[playingPattern][keyPos].stepType == STEPTYPE_MUTE ) {
								stepNoteP[playingPattern][keyPos].stepType = ( stepNoteP[playingPattern][keyPos].stepType == STEPTYPE_PLAY ) ? STEPTYPE_MUTE : STEPTYPE_PLAY;
							}
						}
					}
				}
				
				// ### KEY RELEASE EVENTS
				
				if (e.bit.EVENT == KEY_JUST_RELEASED && thisKey != 0) {
					
				}
				
				if (e.bit.EVENT == KEY_JUST_RELEASED && thisKey != 0 && (noteSelection || stepRecord) && selectedNote > 0) {
					if (!playing){
						seqNoteOff(thisKey, playingPattern);
					}
					if (stepRecord && stepDirty) {
						step_ahead(playingPattern);
						stepDirty = false;
					}
				}

				// AUX KEY PRESS EVENTS
				
				if (e.bit.EVENT == KEY_JUST_PRESSED && thisKey == 0) {
					
					if (noteSelect){
						if (noteSelection){
							selectedStep = 0;
							selectedNote = 0;
						} else {
							
						}
						noteSelection = false;
						noteSelect = !noteSelect;
						dirtyDisplay = true;

					} else if (patternParams){
						patternParams = !patternParams;
						dirtyDisplay = true;

					} else if (stepRecord){
						stepRecord = !stepRecord;
						dirtyDisplay = true;

					} else {
						if (keyState[1] || keyState[2]) { 				// CHECK keyState[] FOR LONG PRESS OF FUNC KEYS
							if (keyState[1]) {	
								seqResetFlag = true;					// RESET ALL SEQUENCES TO FIRST/LAST STEP 
								infoDialog[RESET].state = true; // reset flag
//								dialogFlags[3] = true; // reset flag
								

							} else if (keyState[2]) { 					// CHANGE PATTERN DIRECTION
								patternSettings[playingPattern].reverse = !patternSettings[playingPattern].reverse;
								if (patternSettings[playingPattern].reverse) {
									infoDialog[REV].state = true; // rev direction flag
//									dialogFlags[5] = true; // rev direction flag
								} else{
									infoDialog[FWD].state = true; // fwd direction flag
//									dialogFlags[4] = true; // fwd direction flag
								}

							}
							dirtyDisplay = true;
						} else {
							if (playing){
								// stop transport
								playing = 0;
								allNotesOff();
	//							Serial.println("stop transport");
								seqStop();
							} else {
								// start transport
	//							Serial.println("start transport");
								seqStart();							
							}
						}
					}

				// AUX KEY RELEASE EVENTS

				} else if (e.bit.EVENT == KEY_JUST_RELEASED && thisKey == 0) {
				
				}

//				strip.show();
				break;

			default:
				break;
		} 
		// END MODE SWITCH

		if (e.bit.EVENT == KEY_JUST_RELEASED){
			keyState[thisKey] = false;
			keyPressTime[thisKey] = 0;
		}

	} // END KEYS WHILE
	
	
	// ### LONG KEY SWITCH PRESS
	for (int j=0; j<LED_COUNT; j++){
		if (keyState[j]){
			if (keyPressTime[j] >= longPressInterval && keyPressTime[j] < 9999){

				// DO LONG PRESS THINGS
				switch (omxMode){
					case MODE_MIDI:
						break;
					case MODE_S1:
						// fall through
					case MODE_S2:
						if (!keyState[1] && !keyState[2]) { // SKIP LONG PRESS IF FUNC KEYS ARE ALREDY HELD
							if (j > 2 && j < 11){ // skip AUX key, get pattern keys
								patternParams = true;
								dirtyDisplay = true;
						
							} else if (j > 10){
								if (!stepRecord){ 		// IGNORING LONG PRESSES IN STEP RECORD?
									selectedStep = j - 11; // set noteSelection to this step
									noteSelect = true;
									stepSelect = true;
									noteSelection = true;
									dirtyDisplay = true;
									// re-toggle the key you just held
									if ( stepNoteP[playingPattern][selectedStep].stepType == STEPTYPE_PLAY || stepNoteP[playingPattern][selectedStep].stepType == STEPTYPE_MUTE ) {
										stepNoteP[playingPattern][selectedStep].stepType = ( stepNoteP[playingPattern][selectedStep].stepType == STEPTYPE_PLAY ) ? STEPTYPE_MUTE : STEPTYPE_PLAY;
									}
								}
							}
						}
						break;
					default:
						break;
						
				}
				keyPressTime[j] = 9999;	
			}
		}
	}
	


	// ############### MODES DISPLAY  ##############

	switch(omxMode){
		case MODE_OM: 						// ############## ORGANELLE MODE
			// FALL THROUGH

		case MODE_MIDI:							// ############## MIDI KEYBOARD
			//playingPattern = 0; 		// DEFAULT MIDI MODE TO THE FIRST PATTERN SLOT
			midi_leds();				// SHOW LEDS

			if (dirtyDisplay){			// DISPLAY
				if (!enc_edit){
					dispGenericMode(SUBMODE_MIDI, mimode);
//					dispMidiMode();
				}
			}
			break;

		case MODE_S1: 						// ############## SEQUENCER 1
			// FALL THROUGH
		case MODE_S2: 						// ############## SEQUENCER 2
			if (!enc_edit) {
				if (dialogTimeout > dialogDuration && dialogTimeout < dialogDuration + 20) {
					dirtyDisplay = true;
//					Serial.println("dirty");
				}
			}
	
			if (dirtyDisplay){			// DISPLAY
				if (!enc_edit){
					if (!noteSelect and !patternParams and !stepRecord){
						dispSeqMode1();
						dispInfoDialog();
					}				
					if (noteSelect) {
						if (nspage == 0){
							dispNoteSelect2();
						} else if (nspage == 1){
							dispNoteSelect();
						}
					}
					if (patternParams) {
						dispPatternParams();
						dispInfoDialog();
					}
					if (stepRecord) {
						dispStepRec();
						dispInfoDialog();
					}
					
				}
			}
			
			break;

		default:
			break;

	}

//	Serial.print("one:");
//	Serial.println(checktime1);

	// DISPLAY at end of loop

	if (dirtyDisplay){
		display.display();
		dirtyDisplay = false;
	}
	
//	if (checktime1 >3){
//		Serial.print("two:");
//		Serial.println(checktime1);
//	}
	
	// are pixels dirty
	if (dirtyPixels){
		strip.show();	
		dirtyPixels = false;
	}

	while (MM::usbMidiRead()) {
		// ignore incoming messages
	}
	while (MM::midiRead()) {
		// ignore incoming messages
	}
	
} // ######## END MAIN LOOP ########


// ####### SEQENCER FUNCTIONS

void step_ahead(int patternNum) {
	// step each pattern ahead one place
	for (int j=0; j<8; j++){
		// what direction?
		if (patternSettings[j].reverse) {
			seqPos[j]--;
			if (seqPos[j] < 0)
				seqPos[j] = PatternLength(j)-1;
	
		} else {
			seqPos[j]++;
			if (seqPos[j] >= PatternLength(j))
				seqPos[j] = 0;

		}
	}
}

void step_on(int patternNum){
//		Serial.print(patternNum);
//		Serial.println(" step on");
//	playNote(playingPattern);
	
}

void step_off(int patternNum, int position){
	//	Serial.print(seqPos[patternNum]);
	//	Serial.println(" step off");
	lastNote[patternNum][position] = 0;
	
//      analogWrite(CVPITCH_PIN, 0);
//      digitalWrite(CVGATE_PIN, LOW);
}

void doStep() {
	switch(omxMode){
		case MODE_S1:
			if(playing) {
				// ############## STEP TIMING ##############
				if(micros() >= nextStepTime){
					seqReset();
					// DO STUFF
					int lastPos = (seqPos[playingPattern]+15) % 16;
					if (lastNote[playingPattern][lastPos] > 0){
						step_off(playingPattern, lastPos);
					}
					lastStepTime = nextStepTime;
					nextStepTime += step_micros;

					playNote(playingPattern);
//					step_on(playingPattern);

					show_current_step(playingPattern); // show led for step
					step_ahead(playingPattern);
				}
			} else {
				show_current_step(playingPattern);
			}		
			break;

		case MODE_S2:
			if(playing) {
				if(micros() >= nextStepTime){
					seqReset();
					lastStepTime = nextStepTime;
					nextStepTime += step_micros;

					// check all patterns for notes to play
					for (int j=0; j<8; j++){
						// only play if not muted
						if (!patternSettings[j].mute) {
							int lastPos = (seqPos[j]+15) % 16;
							if (lastNote[j][lastPos] > 0){
								step_off(j, lastPos);
							}
							playNote(j);
						}
					}
					show_current_step(playingPattern);
					step_ahead(playingPattern);

				}			
			} else {
				show_current_step(playingPattern);
			}
			break;

		default:
			break;	
	}
}

void cvNoteOn(int notenum){
	if (notenum>=midiLowestNote && notenum <midiHightestNote){
		pitchCV = static_cast<int>(roundf( (notenum - midiLowestNote) * stepsPerSemitone)); // map (adjnote, 36, 91, 0, 4080);
		digitalWrite(CVGATE_PIN, HIGH);
		analogWrite(CVPITCH_PIN, pitchCV);
	}
}
void cvNoteOff(){
	digitalWrite(CVGATE_PIN, LOW);
//	analogWrite(CVPITCH_PIN, 0);
}

// #### MIDI Mode note on/off
void midiNoteOn(int notenum, int velocity) {
	int adjnote = notes[notenum] + (octave * 12); // adjust key for octave range
	if (adjnote>=0 && adjnote <128){
		midiLastNote = adjnote;

		// keep track of adjusted note when pressed so that when key is released we send
		// the correct note off message
		midiKeyState[notenum] = adjnote;

		MM::sendNoteOn(adjnote, velocity, midiChannel);
		// CV
		cvNoteOn(adjnote);
	}

	strip.setPixelColor(notenum, MIDINOTEON);         //  Set pixel's color (in RAM)
	dirtyPixels = true;	
	dirtyDisplay = true;
}

void midiNoteOff(int notenum) {
	// we use the key state captured at the time we pressed the key to send the correct note off message
	int adjnote = midiKeyState[notenum];
	if (adjnote>=0 && adjnote <128){
		MM::sendNoteOff(adjnote, 0, midiChannel);
		// CV off
		cvNoteOff();
	}
	
	strip.setPixelColor(notenum, LEDOFF); 
	dirtyPixels = true;
	dirtyDisplay = true;
}

// #### SEQ Mode note on/off
void seqNoteOn(int notenum, int velocity, int patternNum){
	int adjnote = notes[notenum] + (octave * 12); // adjust key for octave range
	if (adjnote>=0 && adjnote <128){
		lastNote[patternNum][seqPos[patternNum]] = adjnote;
		MM::sendNoteOn(adjnote, velocity, PatternChannel(playingPattern));

		// keep track of adjusted note when pressed so that when key is released we send
		// the correct note off message
		midiKeyState[notenum] = adjnote;

		// CV
		cvNoteOn(adjnote);
	}

	strip.setPixelColor(notenum, MIDINOTEON);         //  Set pixel's color (in RAM)
	dirtyPixels = true;	
	dirtyDisplay = true;
}

void seqNoteOff(int notenum, int patternNum){
	// we use the key state captured at the time we pressed the key to send the correct note off message
	int adjnote = midiKeyState[notenum];
	if (adjnote>=0 && adjnote <128){
		MM::sendNoteOff(adjnote, 0, PatternChannel(playingPattern));
		// CV off
		cvNoteOff();
	}
	
	strip.setPixelColor(notenum, LEDOFF); 
	dirtyPixels = true;
	dirtyDisplay = true;
}


// Play a note / step (SEQUENCERS)
void playNote(int patternNum) {
	//Serial.println(stepNoteP[patternNum][seqPos].note); // Debug

	switch (stepNoteP[patternNum][seqPos[patternNum]].stepType) {
	case STEPTYPE_MUTE:
		break;      

	case STEPTYPE_PLAY:	// regular note on
		seq_velocity = stepNoteP[playingPattern][seqPos[patternNum]].vel;

		noteoff_micros = micros() + ( stepNoteP[patternNum][seqPos[patternNum]].len + 1 )* step_micros ;
		pendingNoteOffs.insert(stepNoteP[patternNum][seqPos[patternNum]].note, PatternChannel(patternNum), noteoff_micros);

//		MM::sendNoteOn(stepNoteP[patternNum][seqPos[patternNum]].note, seq_velocity, PatternChannel(patternNum));


		// is there swing ? 
//		if ((swing != 0) && (seqPos[patternNum] % 2 == 0)) {
//Serial.print("swing:");
//Serial.print(patternNum);
//Serial.print(":");
//Serial.println(patternSettings[patternNum].swing);

		if ((patternSettings[patternNum].swing != 0) && (seqPos[patternNum] % 2 == 0)) {
			noteon_micros = micros() + (ppqInterval/2 * swing); // constrain(swing, 0, 5);
//			Serial.println(ppqInterval/2 * swing);
		} else {
			noteon_micros = micros();
		}
//		pendingNoteOns.insert(stepNoteP[patternNum][seqPos[patternNum]].note, seq_velocity, PatternChannel(patternNum), micros() 
		pendingNoteOns.insert(stepNoteP[patternNum][seqPos[patternNum]].note, seq_velocity, PatternChannel(patternNum), noteon_micros );

		
		// send param locks // {notenum,vel,len,p1,p2,p3,p4,p5}
		for (int q=0; q<4; q++){	
			int tempCC = stepNoteP[patternNum][seqPos[patternNum]].params[q];
			if (tempCC > -1) {
				MM::sendControlChange(pots[q],tempCC,PatternChannel(patternNum));
				prevPlock[q] = tempCC;
			} else if (prevPlock[q] != potValues[q]) {
				//if (tempCC != prevPlock[q]) {
				MM::sendControlChange(pots[q],potValues[q],PatternChannel(patternNum));
				prevPlock[q] = potValues[q];
			}
		}
		lastNote[patternNum][seqPos[patternNum]] = stepNoteP[patternNum][seqPos[patternNum]].note;
		
		// CV
		cvNoteOn(stepNoteP[patternNum][seqPos[patternNum]].note);
		
		break;

	case STEPTYPE_ACCENT:		 // NOT USED?      
//		MM::sendNoteOn(stepNoteP[patternNum][seqPos[patternNum]][0], seq_acc_velocity, midiChannel);
//		lastNote[patternNum][seqPos[patternNum]] = stepNoteP[patternNum][seqPos[patternNum]].note;
//      	stepCV = map (lastNote[patternNum][seqPos[patternNum]], 35, 90, 0, 4096);
//      	digitalWrite(CVGATE_PIN, HIGH);
//      	analogWrite(CVPITCH_PIN, stepCV);

	case STEPTYPE_RESTART:
		// Skip the remaining notes
		seqPos[patternNum] = 15;
		break;
      break;
  }
}

void allNotesOff() {
	pendingNoteOffs.allOff();
}

void allNotesOffPanic() {
	analogWrite(CVPITCH_PIN, 0);
	digitalWrite(CVGATE_PIN, LOW);
	for (int j=0; j<128; j++){
		MM::sendNoteOff(j, 0, midiChannel);  // NEEDS FIXING
	}
}

void transposeSeq(int patternNum, int amt) {
	for (int k=0; k<NUM_STEPS; k++){
		stepNoteP[patternNum][k].note += amt;
	}
}

void seqReset(){
	if (seqResetFlag) {
		for (int k=0; k<8; k++){
			if (patternSettings[k].reverse) { // REVERSE
				seqPos[k] = PatternLength(k) - 1;
			} else {
				seqPos[k] = 0;
			}
		}
		MM::stopClock();
		MM::startClock();
		seqResetFlag = false;
	}
}

void seqStart() {
	playing = 1;
	nextStepTime = micros();
	if (!seqResetFlag) {
		MM::continueClock();
	}
}

void seqStop() {
	ticks = 0;
	playing = 0;
	MM::stopClock();
	allNotesOff();
}

void seqContinue() {
	playing = 1;
}

void rotatePattern(int patternNum, int rot) {
	if ( patternNum < 0 || patternNum >= NUM_PATTERNS )
		return;
	int size = PatternLength(patternNum);
	StepNote arr[size];
	//rot = rot % size;
	rot = (rot + size) % size;
//	Serial.println(rot);
	for (int d = rot, s = 0; s < size; d = (d+1) % size, ++s)
		arr[d] = stepNoteP[patternNum][s];
	for (int i = 0; i < size; ++i)
		stepNoteP[patternNum][i] = arr[i];
}

void resetPatternDefaults(int patternNum){
	for (int i = 0; i < NUM_STEPS; i++){
		// {notenum,vel,len,stepType,{p1,p2,p3,p4,p5}}
		stepNoteP[patternNum][i].note = patternDefaultNoteMap[patternNum];
		stepNoteP[patternNum][i].len = 0;
	}
}

void clearPattern(int patternNum){
	for (int i = 0; i < NUM_STEPS; i++){
		// {notenum,vel,len,stepType,{p1,p2,p3,p4,p5}}
		stepNoteP[patternNum][i].note = patternDefaultNoteMap[patternNum];
		stepNoteP[patternNum][i].vel = 100;
		stepNoteP[patternNum][i].len = 0;
		stepNoteP[patternNum][i].stepType = STEPTYPE_MUTE;
		stepNoteP[patternNum][i].params[0] = -1;
		stepNoteP[patternNum][i].params[1] = -1;
		stepNoteP[patternNum][i].params[2] = -1;
		stepNoteP[patternNum][i].params[3] = -1;
		stepNoteP[patternNum][i].params[4] = -1;
	}
}

void copyPattern(int patternNum){
	//for( int i = 0 ; i < NUM_STEPS ; ++i ){
	//	copyPatternBuffer[i] = stepNoteP[patternNum][i];
	//}

	memcpy( &copyPatternBuffer, &stepNoteP[patternNum], NUM_STEPS * sizeof(StepNote) );
}

void pastePattern(int patternNum){
	//for( int i = 0 ; i < NUM_STEPS ; ++i ){
	//	stepNoteP[patternNum][i] = copyPatternBuffer[i] ;
	//}

	memcpy( &stepNoteP[patternNum], &copyPatternBuffer, NUM_STEPS * sizeof(StepNote) );
}

void u8g2centerText(const char* s, int16_t x, int16_t y, uint16_t w, uint16_t h) {
//  int16_t bx, by;
  uint16_t bw, bh;
  bw = u8g2_display.getUTF8Width(s);
  bh = u8g2_display.getFontAscent();
  u8g2_display.setCursor(
    x + (w - bw) / 2,
    y + (h - bh) / 2
  );
  u8g2_display.print(s);
}

void u8g2centerNumber(int n, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  char buf[8];
  itoa(n, buf, 10);
  u8g2centerText(buf, x, y, w, h);
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

void drawLoading(void) {
	const char* loader[] = {"\u25f0", "\u25f1", "\u25f2", "\u25f3"};
	display.clearDisplay();
	u8g2_display.setFontMode(0);  
	for(int16_t i=0; i<16; i+=1) {
		display.clearDisplay();
		u8g2_display.setCursor(18,18);
		u8g2_display.setFont(FONT_TENFAT);
		u8g2_display.print("OMX-27");
		u8g2_display.setFont(FONT_SYMB_BIG);
		u8g2centerText(loader[i%4], 80, 10, 32, 32); // "\u00BB\u00AB" // // dice: "\u2685"
		display.display();
		delay(100);
	}

  delay(100);
}

void initPatterns( void ) {
	// default to GM Drum Map for now
	uint8_t initNotes[NUM_PATTERNS] = { 
		36,
		38,
		37,
		39,
		42,
		46,
		49,
		51 };

	StepNote stepNote = { 0, 100, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1 }  };

	for ( int i=0; i<NUM_PATTERNS; i++ ) {
		stepNote.note = initNotes[i];
		for ( int j=0; j<NUM_STEPS; j++ ) {			
			memcpy( &stepNoteP[i][j], &stepNote, sizeof(StepNote) );
		}

		patternSettings[i].len = 15;
		patternSettings[i].channel = i;		// 0 - 15 becomes 1 - 16
		patternSettings[i].mute = false;
		patternSettings[i].reverse = false;
		patternSettings[i].swing = 0;
	}
}

void saveHeader( void ) {
	// 1 byte for EEPROM version
	EEPROM.update( EEPROM_HEADER_ADDRESS + 0, EEPROM_VERSION );

	// 1 byte for mode
	EEPROM.update( EEPROM_HEADER_ADDRESS + 1, (uint8_t)omxMode );

	// 1 byte for the active pattern
	EEPROM.update( EEPROM_HEADER_ADDRESS + 2, (uint8_t)playingPattern );

	// 1 byte for Midi channel
	uint8_t unMidiChannel = (uint8_t)( midiChannel - 1 );
	EEPROM.update( EEPROM_HEADER_ADDRESS + 3, unMidiChannel );

	for ( int i=0; i<NUM_CC_POTS; i++ ) {
		EEPROM.update( EEPROM_HEADER_ADDRESS + 4 + i, pots[i] );
	}

	// 23 bytes remain for header fields
}

// returns true if the header contained initialized data
// false means we shouldn't attempt to load any further information
bool loadHeader( void ) {
	uint8_t version = EEPROM.read( EEPROM_HEADER_ADDRESS + 0 );

	//char buf[64];
	//snprintf( buf, sizeof(buf), "EEPROM Header Version is %d\n", version );
	//Serial.print( buf );

	// Uninitalized EEPROM memory is filled with 0xFF
	if ( version == 0xFF ) {
		// EEPROM was uninitialized
		//Serial.println( "version was 0xFF" );
		return false;
	}

	if ( version != EEPROM_VERSION ) {
		// write an adapter if we ever need to increment the EEPROM version and also save the existing patterns
		// for now, return false will essentially reset the state
		return false;
	}
	
	omxMode = (OMXMode)EEPROM.read( EEPROM_HEADER_ADDRESS + 1 );

	playingPattern = EEPROM.read( EEPROM_HEADER_ADDRESS + 2 );

	uint8_t unMidiChannel = EEPROM.read( EEPROM_HEADER_ADDRESS + 3 );
	midiChannel = unMidiChannel + 1;

	for ( int i=0; i<NUM_CC_POTS; i++ ) {
		pots[i] = EEPROM.read( EEPROM_HEADER_ADDRESS + 4 + i );
	}

	return true;
}

void savePatterns( void ) {
	int nLocalAddress = EEPROM_PATTERN_ADDRESS;
	int s = sizeof( StepNote );

	// EEPROM.put uses EEPROM.update under the hood, so writes here of the same data are a noop

	// for each pattern
	for ( int i=0; i<NUM_PATTERNS; i++ ) {
		for ( int j=0; j<NUM_STEPS; j++ ) {
			EEPROM.put( nLocalAddress, stepNoteP[i][j] );
			nLocalAddress += s;
		}
	}

	nLocalAddress = EEPROM_PATTERN_SETTINGS_ADDRESS;
	s = sizeof( PatternSettings );

	// save pattern settings
	for ( int i=0; i<NUM_PATTERNS; i++ ) {
		EEPROM.put( nLocalAddress, patternSettings[i] );
		nLocalAddress += s;
	}
}

void loadPatterns( void ) {
	//Serial.println( "load patterns" );

	int nLocalAddress = EEPROM_PATTERN_ADDRESS;
	int s = sizeof( StepNote );

	// for each pattern
	for ( int i=0; i<NUM_PATTERNS; i++ ) {
		for ( int j=0; j<NUM_STEPS; j++ ) {
			EEPROM.get( nLocalAddress, stepNoteP[i][j] );
			nLocalAddress += s;
		}
	}

	nLocalAddress = EEPROM_PATTERN_SETTINGS_ADDRESS;
	s = sizeof( PatternSettings );

	// load pattern length
	for ( int i=0; i<NUM_PATTERNS; i++ ) {
		EEPROM.get( nLocalAddress, patternSettings[i] );
		nLocalAddress += s;
	}
}

// currently saves everything ( mode + patterns )
void saveToEEPROM( void ) {
	//Serial.println( "saving..." );
	saveHeader();
	savePatterns();
}

// currently loads everything ( mode + patterns )
bool loadFromEEPROM( void ) {
	// This load can happen soon after Serial.begin - enable this 'wait for Serial' if you need to Serial.print during loading
	//while( !Serial );

	bool bContainedData = loadHeader();

	//Serial.println( "read the header" );

	if ( bContainedData ) {
		//Serial.println( "loading patterns" );
		loadPatterns();
		return true;
	}	

	return false;
}