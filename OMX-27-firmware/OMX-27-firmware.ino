// OMX-27 MIDI KEYBOARD / SEQUENCER
// v 1.6.0
//
// Steven Noreyko, Last update: July 2022
//
//
//	Big thanks to:
//	John Park and Gerald Stevens for initial testing and feature ideas
//	mzero for immense amounts of code coaching/assistance
//	drjohn for support
//  Additional code contributions: Matt Boone, Steven Zydek, Chris Atkins, Will Winder


#include <functional>
// #include <Adafruit_NeoPixel.h>
#include <ResponsiveAnalogRead.h>

#include "consts.h"
#include "config.h"
#include "colors.h"
#include "MM.h"
#include "ClearUI.h"
#include "sequencer.h"
#include "noteoffs.h"
#include "storage.h"
#include "sysex.h"
#include "omx_keypad.h"
#include "omx_util.h"
#include "omx_disp.h"

#include "omx_mode_midi_keyboard.h"
#include "omx_mode_sequencer.h"
#include "omx_screensaver.h"
#include "omx_leds.h"



OmxModeMidiKeyboard omxModeMidi;
OmxModeMidiKeyboard omxModeOM;
OmxModeSequencer omxModeS1;
OmxModeSequencer omxModeS2;

OmxModeInterface* activeOmxMode;

OmxScreensaver omxScreensaver;

// storage of pot values; current is in the main loop; last value is for midi output
int volatile currentValue[potCount];
int lastMidiValue[potCount];
int potMin = 0;
int potMax = 8190;
int temp;

// Timers and such
// elapsedMillis blink_msec = 0;
// elapsedMillis slow_blink_msec = 0;
elapsedMillis pots_msec = 0;

elapsedMicros clksTimer = 0;		// is this still in use?

//unsigned long clksDelay;
//elapsedMillis keyPressTime[27] = {0};

using Micros = unsigned long;
Micros lastProcessTime;
volatile unsigned long step_micros;
volatile unsigned long noteon_micros;
volatile unsigned long noteoff_micros;
volatile unsigned long ppqInterval;


// bool screenSaverMode = false;

// int sleepTick = 80;

// DEFAULT COLOR VARIABLES


int nspage = 0;
// int pppage = 0;
// int sqpage = 0;
// int srpage = 0;
int mmpage = 0;

int miparam = 0;	// midi params item counter
// int nsparam = 0;	// note select params
// int ppparam = 0;	// pattern params
// int sqparam = 0;	// seq params
// int srparam = 0;	// step record params
int tmpmmode = 9;

// VARIABLES / FLAGS
// float step_delay;
// bool dirtyPixels = false;

// bool blinkState = false;
// bool slowBlinkState = false;

// bool patternParams = false;
// bool seqPages = false;
int noteSelectPage = 0;

bool dialogFlags[] = {false, false, false, false, false, false};
unsigned dialogDuration = 1000;

int pitchCV;
uint8_t RES;
uint16_t AMAX;
int V_scale;

// unsigned long blinkInterval = clockConfig.clockbpm * 2;

//int midiChannel; // the MIDI channel number to send messages (MIDI/OM mode)

// ENCODER
Encoder myEncoder(12, 11); 	// encoder pins on hardware
Button encButton(0);		// encoder button pin on hardware
//long newPosition = 0;
//long oldPosition = -999;

// KEYPAD
//initialize an instance of custom Keypad class
unsigned long longPressInterval = 800;
unsigned long clickWindow = 200;
OMXKeypad keypad(longPressInterval, clickWindow, makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// // Declare NeoPixel strip object
// Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// setup EEPROM/FRAM storage
Storage* storage;
SysEx* sysEx;

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
	ppqInterval = 60000000/(PPQ * clockConfig.clockbpm); 		// ppq interval is in microseconds
	step_micros = ppqInterval * (PPQ/4); 			// 16th note step in microseconds (quarter of quarter note)

	// 16th note step length in milliseconds
	step_delay = step_micros * 0.001; 	// ppqInterval * 0.006; // 60000 / clockbpm / 4;
}

void setGlobalSwing(int swng_amt){
	for(int z=0; z<NUM_PATTERNS; z++) {
		sequencer.getPattern(z)->swing = swng_amt;
	}
}

// ####### POTENTIOMETERS #######

// void sendPots(int val, int channel){
// 	MM::sendControlChange(pots[potbank][val], analogValues[val], channel);
// 	potCC = pots[potbank][val];
// 	potVal = analogValues[val];
// 	potValues[val] = potVal;
// }

void readPotentimeters(){
	for(int k=0; k<potCount; k++) {
		temp = analogRead(analogPins[k]);
		potSettings.analog[k]->update(temp);

		// read from the smoother, constrain (to account for tolerances), and map it
		temp = potSettings.analog[k]->getValue();
		temp = constrain(temp, potMin, potMax);
		temp = map(temp, potMin, potMax, 0, 16383);

		// map and update the value
		potSettings.analogValues[k] = temp >> 7;

		if(potSettings.analog[k]->hasChanged()) {
			 // do stuff
			if (sysSettings.screenSaverMode){
				omxScreensaver.OnPotChanged(k, potSettings.analogValues[k]);
			}
			else { // don't send pots in screensaver
			{
				activeOmxMode->OnPotChanged(k, potSettings.analogValues[k]);
			}

/*
			switch(sysSettings.omxMode) {
				case MODE_OM:
						// fall through - same as MIDI
				case MODE_MIDI: // MIDI
					if (midiMacro && !screenSaverMode){
						omxutil.sendPots(k, midiMacroChan);
					} else if (screenSaverMode) {
						// don't send pots in screensaver
					} else {
						omxutil.sendPots(k, sysSettings.midiChannel);
					}
					omxidsp.setDispDirty();
					break;

				case MODE_S2: // SEQ2
						// fall through - same as SEQ1
				case MODE_S1: // SEQ1
					if (noteSelect && noteSelection){ // note selection - do P-Locks
						potNum = k;
						potCC = pots[potbank][k];
						potVal = analogValues[k];

						if (k < 4){ // only store p-lock value for first 4 knobs
							getSelectedStep()->params[k] = analogValues[k];
							omxutil.sendPots(k, sequencer.getPatternChannel(sequencer.playingPattern));
						}
						omxutil.sendPots(k, sequencer.getPatternChannel(sequencer.playingPattern));
						omxidsp.setDispDirty();
					} else if (stepRecord){
						potNum = k;
						potCC = pots[potbank][k];
						potVal = analogValues[k];

						if (k < 4){ // only store p-lock value for first 4 knobs
							sequencer.getCurrentPattern()->steps[sequencer.seqPos[sequencer.playingPattern]].params[k] = analogValues[k];
							omxutil.sendPots(k, sequencer.getPatternChannel(sequencer.playingPattern));
						} else if (k == 4){
							sequencer.getCurrentPattern()->steps[sequencer.seqPos[sequencer.playingPattern]].vel = analogValues[k]; // SET POT 5 to NOTE VELOCITY HERE
						}
						omxidsp.setDispDirty();
					} else if (!noteSelect || !stepRecord){
						omxutil.sendPots(k, sequencer.getPatternChannel(sequencer.playingPattern));
					}
					break;

				default:
					break;
				}
			}
			*/
	}
}


// ####### SETUP #######

void setup() {
	Serial.begin(115200);
//	while( !Serial );

	storage = Storage::initStorage();
	sysEx = new SysEx(storage, &sysSettings);

	// incoming usbMIDI callbacks
	usbMIDI.setHandleNoteOff(OnNoteOff);
	usbMIDI.setHandleNoteOn(OnNoteOn);
	usbMIDI.setHandleControlChange(OnControlChange);
	usbMIDI.setHandleSystemExclusive(OnSysEx);

	clksTimer = 0;
	screenSaverCounter = 0;
	ssstep = 0;
	
	lastProcessTime = micros();
	resetClocks();

	randomSeed(analogRead(13));
	srand(analogRead(13));

	// SET ANALOG READ resolution to teensy's 13 usable bits
	analogReadResolution(13);

	// initialize ResponsiveAnalogRead
	for (int i = 0; i < potCount; i++){
		potSettings.analog[i] = new ResponsiveAnalogRead(0, true, .001);
		potSettings.analog[i]->setAnalogResolution(1 << 13);

		// ResponsiveAnalogRead is designed for 10-bit ADCs
		// meanining its threshold defaults to 4. Let's bump that for
		// our 13-bit adc by setting it to 4 << (13-10)
		potSettings.analog[i]->setActivityThreshold(32);

		currentValue[i] = 0;
		lastMidiValue[i] = 0;
	}

	activeOmxMode = &omxModeMidi;

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
	bool bLoaded = loadFromStorage();
	if ( !bLoaded )
	{
		// Failed to load due to initialized EEPROM or version mismatch
		// defaults
		sysSettings.omxMode = DEFAULT_MODE;
		sequencer.playingPattern = 0;
		sysSettings.playingPattern = 0;
		sysSettings.midiChannel = 1;
		pots[0][0] = CC1;
		pots[0][1] = CC2;
		pots[0][2] = CC3;
		pots[0][3] = CC4;
		pots[0][4] = CC5;
		initPatterns();
		saveToStorage();
	}

	// Init Display
	omxidsp.setup();

	// Startup screen
	omxidsp.drawStartupScreen();

	// Keypad
//	customKeypad.begin();
	keypad.begin();
	
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

	
}

// ####### END SETUP #######


// ####### MIDI LEDS #######

void midi_leds() {
	blinkInterval = step_delay*2;

	if (blink_msec >= blinkInterval){
		blinkState = !blinkState;
		blink_msec = 0;
	}

	if (midiAUX){
		// Blink left/right keys for octave select indicators.
		auto color1 = blinkState ? LIME : LEDOFF;
		auto color2 = blinkState ? MAGENTA : LEDOFF;
		auto color3 = blinkState ? ORANGE : LEDOFF;
		auto color4 = blinkState ? RBLUE : LEDOFF;

		for (int q = 1; q < LED_COUNT; q++){				
			if (midiKeyState[q] == -1){
				if (midiBg_Hue == 0){
					strip.setPixelColor(q, LEDOFF);
				} else if (midiBg_Hue == 32){
					strip.setPixelColor(q, LOWWHITE);
				} else {
					strip.setPixelColor(q, strip.ColorHSV(midiBg_Hue, midiBg_Sat, midiBg_Brightness));
				}
			}
		}
		strip.setPixelColor(0, RED);
		strip.setPixelColor(1, color1);
		strip.setPixelColor(2, color2);
		strip.setPixelColor(11, color3);
		strip.setPixelColor(12, color4);


	// Macros

	} else if (m8AUX){
		auto color5 = blinkState ? ORANGE : LEDOFF;
		auto color6 = blinkState ? RED : LEDOFF;

		strip.setPixelColor(0, BLUE);
		strip.setPixelColor(1, ORANGE); // all mute
		strip.setPixelColor(3, LIME); // MIXER
		strip.setPixelColor(4, CYAN); // snap load
		strip.setPixelColor(5, MAGENTA); // snap save

		for(int m = 11; m < LED_COUNT-8; m++){
			if (m8mutesolo[m-11]){
				strip.setPixelColor(m, color5);
			}else{
				strip.setPixelColor(m, ORANGE);
			}
		}
		
		strip.setPixelColor(6, RED); // all solo
		for(int m = 19; m < LED_COUNT; m++){
			if (m8mutesolo[m-11]){
				strip.setPixelColor(m, color6);
			}else{
				strip.setPixelColor(m, RED);
			}
		}
		strip.setPixelColor(2, LEDOFF);
		strip.setPixelColor(7, LEDOFF);
		strip.setPixelColor(8, LEDOFF);

		strip.setPixelColor(9, YELLOW); // WAVES
		strip.setPixelColor(10, BLUE); // PLAY


	} else {
		// AUX key
		strip.setPixelColor(0, LEDOFF);
		
		// Other keys
		if (!sysSettings.screenSaverMode){
			// clear not held leds
			for (int q = 1; q < LED_COUNT; q++){				
				if (midiKeyState[q] == -1){
					if (midiBg_Hue == 0){
						strip.setPixelColor(q, LEDOFF);
					} else if (midiBg_Hue == 32){
						strip.setPixelColor(q, LOWWHITE);
					} else {
						strip.setPixelColor(q, strip.ColorHSV(midiBg_Hue, midiBg_Sat, midiBg_Brightness));
					}
				}
			}
		}
	}
	dirtyPixels = true;
}

// ####### SEQUENCER LEDS #######

void show_current_step(int patternNum) {
	omxLeds.updateLeds();

	omxModeS1.showCurrentStep(patternNum);

	/*
	blinkInterval = step_delay*2;
	unsigned long slowBlinkInterval = blinkInterval * 2;

	if (blink_msec >= blinkInterval){
		blinkState = !blinkState;
		blink_msec = 0;
	}
	if (slow_blink_msec >= slowBlinkInterval){
		slowBlinkState = !slowBlinkState;
		slow_blink_msec = 0;
	}
	*/
}

// ####### END LEDS


// ####### DISPLAY FUNCTIONS #######


// ############## MAIN LOOP ##############

void loop() {
//	customKeypad.tick();
	keypad.tick();
	clksTimer = 0;

	Micros now = micros();
	Micros passed = now - lastProcessTime;
	lastProcessTime = now;

	sysSettings.timeElasped = passed;

	if (passed > 0) {
		if (sequencer.playing){
			omxScreensaver.resetCounter(); // screenSaverCounter = 0;
			advanceClock(passed);
			advanceSteps(passed);
		}
	}
	doStep();

	// DISPLAY SETUP
	display.clearDisplay();

	// ############### SLEEP MODE ###############
	//
//	Serial.println(screenSaverCounter);
	omxScreensaver.updateScreenSaverState();
	sysSettings.screenSaverMode = omxScreensaver.shouldShowScreenSaver();

	// ############### POTS ###############
	//
	readPotentimeters();


	// ############### EXTERNAL MODE CHANGE / SYSEX ###############
	if ((!encoderConfig.enc_edit && (sysSettings.omxMode != sysSettings.newmode)) || sysSettings.refresh){
		sysSettings.newmode = sysSettings.omxMode;
		sequencer.playingPattern = sysSettings.playingPattern;
		omxDisp.setDirty();
		setAllLEDS(0,0,0);
		omxLeds.setDirty();
		sysSettings.refresh = false;
	}


	// ############### ENCODER ###############
	//
	auto u = myEncoder.update();
	if (u.active()) {
		auto amt = u.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)
		omxScreensaver.resetCounter(); // screenSaverCounter = 0;
//    	Serial.println(u.dir() < 0 ? "ccw " : "cw ");
//    	Serial.println(amt);

		// Change Mode
		if (encoderConfig.enc_edit) {
			// set mode
//			int modesize = NUM_OMX_MODES;
			sysSettings.newmode = (OMXMode)constrain(sysSettings.newmode + amt, 0, NUM_OMX_MODES - 1);
			omxDisp.dispMode();
			omxDisp.bumpDisplayTimer();
			omxDisp.setDirty();
		} else {
			activeOmxMode->onEncoderChanged(u);
		} 
	}
	// END ENCODER

	// ############### ENCODER BUTTON ###############
	//
	auto s = encButton.update();
	switch (s) {
		// SHORT PRESS
		case Button::Down: //Serial.println("Button down");
			omxScreensaver.resetCounter(); // screenSaverCounter = 0;
			
			// what page are we on?
			if (sysSettings.newmode != sysSettings.omxMode && encoderConfig.enc_edit) {
				sysSettings.omxMode = sysSettings.newmode;
				seqStop();
				omxLeds.setAllLEDS(0,0,0);
				encoderConfig.enc_edit = false;
				omxDisp.dispMode();
			} else if (encoderConfig.enc_edit){
				encoderConfig.enc_edit = false;
			}

			activeOmxMode->onEncoderButtonDown();

			omxDisp.setDirty();
			break;

		// LONG PRESS
		case Button::DownLong: //Serial.println("Button downlong");
			if(activeOmxMode->shouldBlockEncEdit())
			{
				activeOmxMode->onEncoderButtonDown();
			}
			else{
				encoderConfig.enc_edit = true;
				sysSettings.newmode = sysSettings.omxMode;
				omxDisp.dispMode();
			}

			omxDisp.setDirty();
			break;
		case Button::Up: //Serial.println("Button up");
			activeOmxMode->onEncoderButtonUp();
			break;
		case Button::UpLong: //Serial.println("Button uplong");
			activeOmxMode->onEncoderButtonUpLong();
			break;
		default:
			break;
	}
	// END ENCODER BUTTON

	// ############### KEY HANDLING ###############
	//
	while(keypad.available()) {
		auto e = keypad.next();
		int thisKey = e.key();
		// int keyPos = thisKey - 11;
		// int seqKey = keyPos + (sequencer.patternPage[sequencer.playingPattern] * NUM_STEPKEYS);

		if (e.down()){
			omxScreensaver.resetCounter(); // screenSaverCounter = 0;
			midiSettings.keyState[thisKey] = true;
		}
	
		if (e.down() && thisKey == 0 && encoderConfig.enc_edit) {
			// temp - save whenever the 0 key is pressed in encoder edit mode
			saveToStorage();
			//	Serial.println("EEPROM saved");
		}

		activeOmxMode->onKeyUpdate(e);

		// END MODE SWITCH

		if (!e.down()){
			midiSettings.keyState[thisKey] = false;
		}

		// TODO I believe this is handled in omx_mode_sequencer.onKeyUpdate()
		// need to test and make sure this works
		// if (!midiSettings.keyState[1] && !midiSettings.keyState[2]) {
		// 	seqPages = false;
		// }
		
		// ### LONG KEY SWITCH PRESS
		if (e.held()) {
			// DO LONG PRESS THINGS
			activeOmxMode->onKeyHeldUpdate(e); // Only the sequencer uses this, could probably be handled in onKeyUpdate() but keyStates are modified before this stuff happens. 
		} // END IF HELD

	}  // END KEYS WHILE

	if (!sysSettings.screenSaverMode){

		// ############### MODES DISPLAY  ##############
		//

		omxDisp.UpdateMessageTextTimer();
	
		switch(sysSettings.omxMode){
			case MODE_OM: 						// ############## ORGANELLE MODE
				// FALL THROUGH
			case MODE_MIDI:							// ############## MIDI KEYBOARD
				//playingPattern = 0; 		// DEFAULT MIDI MODE TO THE FIRST PATTERN SLOT
				midi_leds();				// SHOW LEDS
				if (dirtyDisplay){			// DISPLAY
					if (!encoderConfig.enc_edit){
						int pselected = miparam % NUM_DISP_PARAMS;
						if (mmpage == 0){
							dispGenericMode(SUBMODE_MIDI, pselected);
						} else if (mmpage == 1){
							dispGenericMode(SUBMODE_MIDI2, pselected);
						} else if (mmpage == 2){
							dispGenericMode(SUBMODE_MIDI3, pselected);
						}
					}
				}
				break;
			case MODE_S1: 						// ############## SEQUENCER 1
				// FALL THROUGH
			case MODE_S2: 						// ############## SEQUENCER 2
				// MIDI SOLO
				if (sequencer.getCurrentPattern()->solo) {
					midi_leds();
				}
				if (dirtyDisplay){			// DISPLAY
					if (!encoderConfig.enc_edit && messageTextTimer == 0){	// show only if not encoder edit or dialog display
						if (!noteSelect and !patternParams and !stepRecord){
							int pselected = sqparam % NUM_DISP_PARAMS;
							if (sqpage == 0){
								dispGenericMode(SUBMODE_SEQ, pselected);
							} else if (sqpage == 1){
								dispGenericMode(SUBMODE_SEQ2, pselected);
							}
						}
						if (noteSelect) {
							int pselected = nsparam % NUM_DISP_PARAMS;
							if (nspage == 0){
								dispGenericMode(SUBMODE_NOTESEL, pselected);
							} else if (nspage == 1){
								dispGenericMode(SUBMODE_NOTESEL2, pselected);
							} else if (nspage == 2){
								dispGenericMode(SUBMODE_NOTESEL3, pselected);
							}
						}
						if (patternParams) {
							int pselected = ppparam % NUM_DISP_PARAMS;
							if (pppage == 0){
								dispGenericMode(SUBMODE_PATTPARAMS, pselected);
							} else if (pppage == 1){
								dispGenericMode(SUBMODE_PATTPARAMS2, pselected);
							} else if (pppage == 2){
								dispGenericMode(SUBMODE_PATTPARAMS3, pselected);
							}

						}
						if (stepRecord) {
							int pselected = srparam % NUM_DISP_PARAMS;
							if (srpage == 0){
								dispGenericMode(SUBMODE_STEPREC, pselected);
							} else if (srpage == 1){
								dispGenericMode(SUBMODE_NOTESEL2, pselected);
							}
						}
					}
				}
				break;
			default:
				break;
		} // END SWITCH

	} else {	// if !screensaver
		switch(sysSettings.omxMode){
			case MODE_OM: 						// ############## ORGANELLE MODE
				// FALL THROUGH
			case MODE_MIDI:							// ############## MIDI KEYBOARD
					// sleepModeOne();
					omxScreensaver.updateLEDs();
				break;
			default:
				break;
		}
		// clear display
		display.clearDisplay();
		omxDisp.setDirty();
	}

	// DISPLAY at end of loop

	omxDisp.showDisplay();

	

	omxLeds.showLeds();

	while (MM::usbMidiRead()) {
		// incoming messages - see handlers
	}
	while (MM::midiRead()) {
		// ignore incoming messages
	}

} // ######## END MAIN LOOP ########


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

// #### Inbound MIDI callbacks
void OnNoteOn(byte channel, byte note, byte velocity) {
	if (midiSoftThru) {
		MM::sendNoteOnHW(note, velocity, channel);
	}
	if (midiInToCV){
		cvNoteOn(note);
	}
	if (sysSettings.omxMode == MODE_MIDI){
		midiLastNote = note;
		int whatoct = (note / 12);
		int thisKey;
		uint32_t keyColor = MIDINOTEON;

		if ( (whatoct % 2) == 0) {
			thisKey = note - (12 * whatoct);
		} else {
			thisKey = note - (12 * whatoct) + 12;
		}
		if (whatoct == 0){ // ORANGE,YELLOW,GREEN,MAGENTA,CYAN,BLUE,LIME,LTPURPLE
		} else if(whatoct == 1){ keyColor = ORANGE;
		} else if(whatoct == 2){ keyColor = YELLOW;
		} else if(whatoct == 3){ keyColor = GREEN;
		} else if(whatoct == 4){ keyColor = MAGENTA;
		} else if(whatoct == 5){ keyColor = CYAN;
		} else if(whatoct == 6){ keyColor = LIME;
		} else if(whatoct == 7){ keyColor = CYAN;
		}
		strip.setPixelColor(midiKeyMap[thisKey], keyColor);         //  Set pixel's color (in RAM)
	//	dirtyPixels = true;
		strip.show();
		omxDisp.setDirty();
	}
}
void OnNoteOff(byte channel, byte note, byte velocity) {
	if (midiInToCV){
		cvNoteOff();
	}
	if (sysSettings.omxMode == MODE_MIDI){
		int whatoct = (note / 12);
		int thisKey;
		if ( (whatoct % 2) == 0) {
			thisKey = note - (12 * whatoct);
		} else {
			thisKey = note - (12 * whatoct) + 12;
		}
		strip.setPixelColor(midiKeyMap[thisKey], LEDOFF);         //  Set pixel's color (in RAM)
	//	dirtyPixels = true;
		strip.show();
		omxDisp.setDirty();
	}
	if (midiSoftThru) {
		MM::sendNoteOffHW(note, velocity, channel);
	}
}
void OnControlChange(byte channel, byte control,  byte value) {
	if (midiSoftThru) {
		MM::sendControlChangeHW(control, value, channel);
	}
}

void OnSysEx(const uint8_t *data, uint16_t length, bool complete) {
	sysEx->processIncomingSysex(data, length);
}

// #### Outbound MIDI Mode note on/off
void midiNoteOn(int notenum, int velocity, int channel) {
	int adjnote = notes[notenum] + (octave * 12); // adjust key for octave range
	rrChannel = (rrChannel % midiRRChannelCount) + 1;
	int adjchan = rrChannel;

	if (adjnote>=0 && adjnote <128){
		midiLastNote = adjnote;

		// keep track of adjusted note when pressed so that when key is released we send
		// the correct note off message
		midiKeyState[notenum] = adjnote;

		// RoundRobin Setting?
		if (midiRoundRobin) {
			adjchan = rrChannel + midiRRChannelOffset;
		} else {
			adjchan = channel;
		}
		midiChannelState[notenum] = adjchan;
		MM::sendNoteOn(adjnote, velocity, adjchan);
		// CV
		cvNoteOn(adjnote);
	}

	strip.setPixelColor(notenum, MIDINOTEON);         //  Set pixel's color (in RAM)
	dirtyPixels = true;
	omxidsp.setDispDirty();
}

void midiNoteOff(int notenum, int channel) {
	// we use the key state captured at the time we pressed the key to send the correct note off message
	int adjnote = midiKeyState[notenum];
	int adjchan = midiChannelState[notenum];
	if (adjnote>=0 && adjnote <128){
		MM::sendNoteOff(adjnote, 0, adjchan);
		// CV off
		cvNoteOff();
		midiKeyState[notenum] = -1;
	}
	
	strip.setPixelColor(notenum, LEDOFF);
	dirtyPixels = true;
	omxidsp.setDispDirty();
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





// #### OLED STUFF




// TODO: move to sequencer.h
void initPatterns( void ) {
	// default to GM Drum Map for now -- GET THIS FROM patternDefaultNoteMap instead
//	uint8_t initNotes[NUM_PATTERNS] = {
//		36,
//		38,
//		37,
//		39,
//		42,
//		46,
//		49,
//		51 };

	StepNote stepNote = { 0, 100, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1 }, 100, 0, STEPTYPE_NONE };
					// {note, vel, len, TRIGTYPE, {params0, params1, params2, params3, params4}, prob, condition, STEPTYPE}

	for ( int i=0; i<NUM_PATTERNS; i++ ) {
		auto pattern = sequencer.getPattern(i);

		stepNote.note = sequencer.patternDefaultNoteMap[i]; // Defined in sequencer.h

		for ( int j=0; j<NUM_STEPS; j++ ) {
			memcpy( &pattern->steps[j], &stepNote, sizeof(StepNote) );
		}

		// TODO: move to sequencer.h
		pattern->len = 15;
		pattern->channel = i;		// 0 - 15 becomes 1 - 16
		pattern->startstep = 0;
		pattern->autoresetstep = 0;
		pattern->autoresetfreq = 0;
		pattern->current_cycle = 1;
		pattern->rndstep = 3;
		pattern->clockDivMultP = 2;
		pattern->autoresetprob = 0;
		pattern->swing = 0;
		pattern->reverse = false;
		pattern->mute = false;
		pattern->autoreset = false;
		pattern->solo = false;
		pattern->sendCV = false;
	}
}

void saveHeader( void ) {
	// 1 byte for EEPROM version
	storage->write( EEPROM_HEADER_ADDRESS + 0, EEPROM_VERSION );

	// 1 byte for mode
	storage->write( EEPROM_HEADER_ADDRESS + 1, (uint8_t)sysSettings.omxMode );

	// 1 byte for the active pattern
	storage->write(EEPROM_HEADER_ADDRESS + 2, (uint8_t)sequencer.playingPattern);

	// 1 byte for Midi channel
	uint8_t unMidiChannel = (uint8_t)(sysSettings.midiChannel - 1);
	storage->write( EEPROM_HEADER_ADDRESS + 3, unMidiChannel );

	for (int b=0; b< NUM_CC_BANKS; b++){
		for ( int i=0; i<NUM_CC_POTS; i++ ) {
			storage->write( EEPROM_HEADER_ADDRESS + 4 + i + (5*b), pots[b][i] );
		}
	}
}

// returns true if the header contained initialized data
// false means we shouldn't attempt to load any further information
bool loadHeader( void ) {
	uint8_t version = storage->read(EEPROM_HEADER_ADDRESS + 0);

//	char buf[64];
//	snprintf( buf, sizeof(buf), "EEPROM Header Version is %d\n", version );
//	Serial.print( buf );

	// Uninitalized EEPROM memory is filled with 0xFF
	if ( version == 0xFF ) {
		// EEPROM was uninitialized
//		Serial.println( "version was 0xFF" );
		return false;
	}

	if ( version != EEPROM_VERSION ) {
		// write an adapter if we ever need to increment the EEPROM version and also save the existing patterns
		// for now, return false will essentially reset the state
//		Serial.println( "version not matched" );
		return false;
	}

	sysSettings.omxMode = (OMXMode)storage->read( EEPROM_HEADER_ADDRESS + 1 );
	sequencer.playingPattern = storage->read(EEPROM_HEADER_ADDRESS + 2);
	sysSettings.playingPattern = sequencer.playingPattern;

	uint8_t unMidiChannel = storage->read( EEPROM_HEADER_ADDRESS + 3 );
	sysSettings.midiChannel = unMidiChannel + 1;

	for (int b=0; b < NUM_CC_BANKS; b++){
		for ( int i=0; i<NUM_CC_POTS; i++ ) {
			pots[b][i] = storage->read( EEPROM_HEADER_ADDRESS + 4 + i + (5*b));
		}
	}
	return true;
}

void savePatterns( void ) {
	int patternSize = serializedPatternSize(storage->isEeprom());
	int nLocalAddress = EEPROM_PATTERN_ADDRESS;

	for (int i=0; i<NUM_PATTERNS; i++) {
		auto pattern = (byte*) sequencer.getPattern(i);
		for (int j = 0; j < patternSize; j++) {
			storage->write(nLocalAddress + j, *pattern++);
		}

		nLocalAddress += patternSize;
	}
}

void loadPatterns( void ) {
	int patternSize = serializedPatternSize(storage->isEeprom());
	int nLocalAddress = EEPROM_PATTERN_ADDRESS;

	for (int i = 0; i < NUM_PATTERNS; i++) {
		auto pattern = Pattern{};
		auto current = (byte*)&pattern;
		for (int j = 0; j < patternSize; j++) {
			*current = storage->read(nLocalAddress + j);
			current++;
		}
		sequencer.patterns[i] = pattern;

		nLocalAddress += patternSize;
	}
}

// currently saves everything ( mode + patterns )
void saveToStorage( void ) {
//	Serial.println( "saving..." );
	saveHeader();
	savePatterns();
}

// currently loads everything ( mode + patterns )
bool loadFromStorage( void ) {
	// This load can happen soon after Serial.begin - enable this 'wait for Serial' if you need to Serial.print during loading
	// while( !Serial );

//	Serial.println( "read the header" );
	bool bContainedData = loadHeader();

	if ( bContainedData ) {
		// Serial.println( "loading patterns" );
		loadPatterns();
		return true;
	}

	return false;
}
