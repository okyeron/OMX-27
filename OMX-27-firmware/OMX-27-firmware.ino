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
// OmxModeMidiKeyboard omxModeOM;
OmxModeSequencer omxModeSeq;
// OmxModeSequencer omxModeS2;

OmxModeInterface *activeOmxMode;

OmxScreensaver omxScreensaver;

// storage of pot values; current is in the main loop; last value is for midi output
int volatile currentValue[NUM_CC_POTS];
int lastMidiValue[NUM_CC_POTS];
int potMin = 0;
int potMax = 8190;
int temp;

// TODO make sequencer.cpp not extern these
// int selectedStep = 0;
// int potbank = 0;
// int prevPlock[NUM_CC_POTS] = {0, 0, 0, 0, 0};
// int potValues[NUM_CC_POTS] = {0, 0, 0, 0, 0};

// Timers and such
// elapsedMillis blink_msec = 0;
// elapsedMillis slow_blink_msec = 0;
// elapsedMillis pots_msec = 0; // TODO - didn't see this used anywhere

// elapsedMicros clksTimer = 0; // TODO - didn't see this used anywhere

// unsigned long clksDelay;
// elapsedMillis keyPressTime[27] = {0};

// using Micros = unsigned long;
Micros lastProcessTime;
// volatile unsigned long step_micros;
// volatile unsigned long noteon_micros;
// volatile unsigned long noteoff_micros;
// volatile unsigned long ppqInterval;

// bool screenSaverMode = false;

// int sleepTick = 80;

// DEFAULT COLOR VARIABLES

// int nspage = 0;
// int pppage = 0;
// int sqpage = 0;
// int srpage = 0;
// int mmpage = 0;

// int miparam = 0;	// midi params item counter
// int nsparam = 0;	// note select params
// int ppparam = 0;	// pattern params
// int sqparam = 0;	// seq params
// int srparam = 0;	// step record params
// int tmpmmode = 9; TODO - didn't see this used anywhere

// VARIABLES / FLAGS
// float step_delay;
// bool dirtyPixels = false;

// bool blinkState = false;
// bool slowBlinkState = false;

// bool patternParams = false;
// bool seqPages = false;
// int noteSelectPage = 0; TODO - didn't see this used anywhere

// bool dialogFlags[] = {false, false, false, false, false, false}; // TODO - didn't see this used anywhere
// unsigned dialogDuration = 1000; // TODO - didn't see this used anywhere

uint8_t RES;
uint16_t AMAX;
int V_scale;

// unsigned long blinkInterval = clockConfig.clockbpm * 2;

// int midiChannel; // the MIDI channel number to send messages (MIDI/OM mode)

// ENCODER
Encoder myEncoder(12, 11); // encoder pins on hardware
Button encButton(0);	   // encoder button pin on hardware
// long newPosition = 0;
// long oldPosition = -999;

// KEYPAD
// initialize an instance of custom Keypad class
unsigned long longPressInterval = 800;
unsigned long clickWindow = 200;
OMXKeypad keypad(longPressInterval, clickWindow, makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// // Declare NeoPixel strip object
// Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// setup EEPROM/FRAM storage
Storage *storage;
SysEx *sysEx;

// ####### SETUP #######

void setup()
{
	Serial.begin(115200);
	//	while( !Serial );

	storage = Storage::initStorage();
	sysEx = new SysEx(storage, &sysSettings);

	// incoming usbMIDI callbacks
	usbMIDI.setHandleNoteOff(OnNoteOff);
	usbMIDI.setHandleNoteOn(OnNoteOn);
	usbMIDI.setHandleControlChange(OnControlChange);
	usbMIDI.setHandleSystemExclusive(OnSysEx);

	// clksTimer = 0; // TODO - didn't see this used anywhere
	omxScreensaver.resetCounter();
	// ssstep = 0;

	lastProcessTime = micros();
	omxUtil.resetClocks();

	randomSeed(analogRead(13));
	srand(analogRead(13));

	// SET ANALOG READ resolution to teensy's 13 usable bits
	analogReadResolution(13);

	// initialize ResponsiveAnalogRead
	for (int i = 0; i < potCount; i++)
	{
		potSettings.analog[i] = new ResponsiveAnalogRead(0, true, .001);
		potSettings.analog[i]->setAnalogResolution(1 << 13);

		// ResponsiveAnalogRead is designed for 10-bit ADCs
		// meanining its threshold defaults to 4. Let's bump that for
		// our 13-bit adc by setting it to 4 << (13-10)
		potSettings.analog[i]->setActivityThreshold(32);

		currentValue[i] = 0;
		lastMidiValue[i] = 0;
	}


	// HW MIDI
	MM::begin();

	// CV gate pin
	pinMode(CVGATE_PIN, OUTPUT);

	// set DAC Resolution CV/GATE
	RES = 12;
	analogWriteResolution(RES); // set resolution for DAC
	AMAX = pow(2, RES);
	V_scale = 64; // pow(2,(RES-7)); 4095 max
	analogWrite(CVPITCH_PIN, 0);

	// Load from EEPROM
	bool bLoaded = loadFromStorage();
	if (!bLoaded)
	{
		// Failed to load due to initialized EEPROM or version mismatch
		// defaults
		// sysSettings.omxMode = DEFAULT_MODE;
		sequencer.playingPattern = 0;
		sysSettings.playingPattern = 0;
		sysSettings.midiChannel = 1;
		pots[0][0] = CC1;
		pots[0][1] = CC2;
		pots[0][2] = CC3;
		pots[0][3] = CC4;
		pots[0][4] = CC5;

		omxModeSeq.initPatterns();

		changeOmxMode(DEFAULT_MODE);
		// initPatterns();
		saveToStorage();
	}

	// Init Display
	omxDisp.setup();

	// Startup screen
	omxDisp.drawStartupScreen();

	// Keypad
	//	customKeypad.begin();
	keypad.begin();

	// LEDs
	omxLeds.initSetup();

	omxScreensaver.InitSetup();
}

// ####### END SETUP #######

// ####### SEQUENCER LEDS #######

void show_current_step(int patternNum)
{
	omxLeds.updateLeds();

	omxModeSeq.showCurrentStep(patternNum);

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

void changeOmxMode(OMXMode newOmxmode)
{
	sysSettings.omxMode = newOmxmode;
	sysSettings.newmode = newOmxmode;

	switch (newOmxmode)
	{
	case MODE_MIDI:
		omxModeMidi.setMidiMode();
		activeOmxMode = &omxModeMidi;
		break;
	case MODE_S1:
		omxModeSeq.setSeq1Mode();
		activeOmxMode = &omxModeSeq;
		break;
	case MODE_S2:
		omxModeSeq.setSeq2Mode();
		activeOmxMode = &omxModeSeq;
		break;
	case MODE_OM:
		omxModeMidi.setOrganelleMode();
		activeOmxMode = &omxModeMidi;
		break;
	default:
		omxModeMidi.setMidiMode();
		activeOmxMode = &omxModeMidi;
		break;
	}
}

// ####### END LEDS

// ############## MAIN LOOP ##############

void loop()
{
	//	customKeypad.tick();
	keypad.tick();
	// clksTimer = 0; // TODO - didn't see this used anywhere

	Micros now = micros();
	Micros passed = now - lastProcessTime;
	lastProcessTime = now;

	sysSettings.timeElasped = passed;

	if (passed > 0)
	{
		if (sequencer.playing)
		{
			omxScreensaver.resetCounter(); // screenSaverCounter = 0;
			omxUtil.advanceClock(passed);
			omxUtil.advanceSteps(passed);
		}
	}

	activeOmxMode->loopUpdate();

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
	if ((!encoderConfig.enc_edit && (sysSettings.omxMode != sysSettings.newmode)) || sysSettings.refresh)
	{
		sysSettings.newmode = sysSettings.omxMode;
		changeOmxMode(sysSettings.omxMode);

		sequencer.playingPattern = sysSettings.playingPattern;
		omxDisp.setDirty();
		omxLeds.setAllLEDS(0, 0, 0);
		omxLeds.setDirty();
		sysSettings.refresh = false;
	}

	// ############### ENCODER ###############
	//
	auto u = myEncoder.update();
	if (u.active())
	{
		auto amt = u.accel(5);		   // where 5 is the acceleration factor if you want it, 0 if you don't)
		omxScreensaver.resetCounter(); // screenSaverCounter = 0;
									   //    	Serial.println(u.dir() < 0 ? "ccw " : "cw ");
									   //    	Serial.println(amt);

		// Change Mode
		if (encoderConfig.enc_edit)
		{
			// set mode
			//			int modesize = NUM_OMX_MODES;
			sysSettings.newmode = (OMXMode)constrain(sysSettings.newmode + amt, 0, NUM_OMX_MODES - 1);
			omxDisp.dispMode();
			omxDisp.bumpDisplayTimer();
			omxDisp.setDirty();
		}
		else
		{
			activeOmxMode->onEncoderChanged(u);
		}
	}
	// END ENCODER

	// ############### ENCODER BUTTON ###############
	//
	auto s = encButton.update();
	switch (s)
	{
	// SHORT PRESS
	case Button::Down:				   // Serial.println("Button down");
		omxScreensaver.resetCounter(); // screenSaverCounter = 0;

		// what page are we on?
		if (sysSettings.newmode != sysSettings.omxMode && encoderConfig.enc_edit)
		{
			changeOmxMode(sysSettings.newmode);
			seqStop();
			omxLeds.setAllLEDS(0, 0, 0);
			encoderConfig.enc_edit = false;
			omxDisp.dispMode();
		}
		else if (encoderConfig.enc_edit)
		{
			encoderConfig.enc_edit = false;
		}

		activeOmxMode->onEncoderButtonDown();

		omxDisp.setDirty();
		break;

	// LONG PRESS
	case Button::DownLong: // Serial.println("Button downlong");
		if (activeOmxMode->shouldBlockEncEdit())
		{
			activeOmxMode->onEncoderButtonDown();
		}
		else
		{
			encoderConfig.enc_edit = true;
			sysSettings.newmode = sysSettings.omxMode;
			omxDisp.dispMode();
		}

		omxDisp.setDirty();
		break;
	case Button::Up: // Serial.println("Button up");
		activeOmxMode->onEncoderButtonUp();
		break;
	case Button::UpLong: // Serial.println("Button uplong");
		activeOmxMode->onEncoderButtonUpLong();
		break;
	default:
		break;
	}
	// END ENCODER BUTTON

	// ############### KEY HANDLING ###############
	//
	while (keypad.available())
	{
		auto e = keypad.next();
		int thisKey = e.key();
		// int keyPos = thisKey - 11;
		// int seqKey = keyPos + (sequencer.patternPage[sequencer.playingPattern] * NUM_STEPKEYS);

		if (e.down())
		{
			omxScreensaver.resetCounter(); // screenSaverCounter = 0;
			midiSettings.keyState[thisKey] = true;
		}

		if (e.down() && thisKey == 0 && encoderConfig.enc_edit)
		{
			// temp - save whenever the 0 key is pressed in encoder edit mode
			saveToStorage();
			//	Serial.println("EEPROM saved");
		}

		activeOmxMode->onKeyUpdate(e);

		// END MODE SWITCH

		if (!e.down())
		{
			midiSettings.keyState[thisKey] = false;
		}

		// TODO I believe this is handled in omx_mode_sequencer.onKeyUpdate()
		// need to test and make sure this works
		// if (!midiSettings.keyState[1] && !midiSettings.keyState[2]) {
		// 	seqPages = false;
		// }

		// ### LONG KEY SWITCH PRESS
		if (e.held())
		{
			// DO LONG PRESS THINGS
			activeOmxMode->onKeyHeldUpdate(e); // Only the sequencer uses this, could probably be handled in onKeyUpdate() but keyStates are modified before this stuff happens.
		}									   // END IF HELD

	} // END KEYS WHILE

	if (!sysSettings.screenSaverMode)
	{
		omxDisp.UpdateMessageTextTimer();
		activeOmxMode->onDisplayUpdate();
	}
	else
	{ // if screenSaverMode
		omxScreensaver.onDisplayUpdate();
	}

	// DISPLAY at end of loop
	omxDisp.showDisplay();

	omxLeds.showLeds();

	while (MM::usbMidiRead())
	{
		// incoming messages - see handlers
	}
	while (MM::midiRead())
	{
		// ignore incoming messages
	}

} // ######## END MAIN LOOP ########

// ####### POTENTIOMETERS #######

// void sendPots(int val, int channel){
// 	MM::sendControlChange(pots[potbank][val], analogValues[val], channel);
// 	potCC = pots[potbank][val];
// 	potVal = analogValues[val];
// 	potValues[val] = potVal;
// }

void readPotentimeters()
{
	for (int k = 0; k < potCount; k++)
	{
		temp = analogRead(analogPins[k]);
		potSettings.analog[k]->update(temp);

		// read from the smoother, constrain (to account for tolerances), and map it
		temp = potSettings.analog[k]->getValue();
		temp = constrain(temp, potMin, potMax);
		temp = map(temp, potMin, potMax, 0, 16383);

		// map and update the value
		potSettings.analogValues[k] = temp >> 7;

		if (potSettings.analog[k]->hasChanged())
		{
			// do stuff
			if (sysSettings.screenSaverMode)
			{
				omxScreensaver.OnPotChanged(k, potSettings.analogValues[k]);
			}
			else
			{ // don't send pots in screensaver
				{
					activeOmxMode->OnPotChanged(k, potSettings.analogValues[k]);
				}
			}
		}
	}
}

// ####### END POTENTIOMETERS #######

void handleNoteOn(byte channel, byte note, byte velocity)
{
	if (midiSettings.midiSoftThru)
	{
		MM::sendNoteOnHW(note, velocity, channel);
	}
	if (midiSettings.midiInToCV)
	{
		omxUtil.cvNoteOn(note);
	}

	activeOmxMode->inMidiNoteOn(channel, note, velocity);
}

void handleNoteOff(byte channel, byte note, byte velocity)
{
	if (midiSettings.midiSoftThru)
	{
		MM::sendNoteOffHW(note, velocity, channel);
	}

	if (midiSettings.midiInToCV)
	{
		omxUtil.cvNoteOff();
	}

	activeOmxMode->inMidiNoteOff(channel, note, velocity);
}

void handleControlChange(byte channel, byte control, byte value)
{
	if (midiSettings.midiSoftThru)
	{
		MM::sendControlChangeHW(control, value, channel);
	}

	activeOmxMode->inMidiControlChange(channel, control, value);
}

// #### Inbound MIDI callbacks
void OnNoteOn(byte channel, byte note, byte velocity)
{
	handleNoteOn(channel, note, velocity);
	
}
void OnNoteOff(byte channel, byte note, byte velocity)
{
	handleNoteOff(channel, note, velocity);
}
void OnControlChange(byte channel, byte control, byte value)
{
	handleControlChange(channel, control, value);
}

void OnSysEx(const uint8_t *data, uint16_t length, bool complete)
{
	sysEx->processIncomingSysex(data, length);
}

void saveHeader()
{
	// 1 byte for EEPROM version
	storage->write(EEPROM_HEADER_ADDRESS + 0, EEPROM_VERSION);

	// 1 byte for mode
	storage->write(EEPROM_HEADER_ADDRESS + 1, (uint8_t)sysSettings.omxMode);

	// 1 byte for the active pattern
	storage->write(EEPROM_HEADER_ADDRESS + 2, (uint8_t)sequencer.playingPattern);

	// 1 byte for Midi channel
	uint8_t unMidiChannel = (uint8_t)(sysSettings.midiChannel - 1);
	storage->write(EEPROM_HEADER_ADDRESS + 3, unMidiChannel);

	for (int b = 0; b < NUM_CC_BANKS; b++)
	{
		for (int i = 0; i < NUM_CC_POTS; i++)
		{
			storage->write(EEPROM_HEADER_ADDRESS + 4 + i + (5 * b), pots[b][i]);
		}
	}
}

// returns true if the header contained initialized data
// false means we shouldn't attempt to load any further information
bool loadHeader(void)
{
	uint8_t version = storage->read(EEPROM_HEADER_ADDRESS + 0);

	//	char buf[64];
	//	snprintf( buf, sizeof(buf), "EEPROM Header Version is %d\n", version );
	//	Serial.print( buf );

	// Uninitalized EEPROM memory is filled with 0xFF
	if (version == 0xFF)
	{
		// EEPROM was uninitialized
		//		Serial.println( "version was 0xFF" );
		return false;
	}

	if (version != EEPROM_VERSION)
	{
		// write an adapter if we ever need to increment the EEPROM version and also save the existing patterns
		// for now, return false will essentially reset the state
		//		Serial.println( "version not matched" );
		return false;
	}

	sysSettings.omxMode = (OMXMode)storage->read(EEPROM_HEADER_ADDRESS + 1);
	changeOmxMode(sysSettings.omxMode);

	sequencer.playingPattern = storage->read(EEPROM_HEADER_ADDRESS + 2);
	sysSettings.playingPattern = sequencer.playingPattern;

	uint8_t unMidiChannel = storage->read(EEPROM_HEADER_ADDRESS + 3);
	sysSettings.midiChannel = unMidiChannel + 1;

	for (int b = 0; b < NUM_CC_BANKS; b++)
	{
		for (int i = 0; i < NUM_CC_POTS; i++)
		{
			pots[b][i] = storage->read(EEPROM_HEADER_ADDRESS + 4 + i + (5 * b));
		}
	}
	return true;
}

void savePatterns(void)
{
	int patternSize = serializedPatternSize(storage->isEeprom());
	int nLocalAddress = EEPROM_PATTERN_ADDRESS;

	for (int i = 0; i < NUM_PATTERNS; i++)
	{
		auto pattern = (byte *)sequencer.getPattern(i);
		for (int j = 0; j < patternSize; j++)
		{
			storage->write(nLocalAddress + j, *pattern++);
		}

		nLocalAddress += patternSize;
	}
}

void loadPatterns(void)
{
	int patternSize = serializedPatternSize(storage->isEeprom());
	int nLocalAddress = EEPROM_PATTERN_ADDRESS;

	for (int i = 0; i < NUM_PATTERNS; i++)
	{
		auto pattern = Pattern{};
		auto current = (byte *)&pattern;
		for (int j = 0; j < patternSize; j++)
		{
			*current = storage->read(nLocalAddress + j);
			current++;
		}
		sequencer.patterns[i] = pattern;

		nLocalAddress += patternSize;
	}
}

// currently saves everything ( mode + patterns )
void saveToStorage(void)
{
	// Serial.println( "saving..." );
	saveHeader();
	savePatterns();
}

// currently loads everything ( mode + patterns )
bool loadFromStorage(void)
{
	// This load can happen soon after Serial.begin - enable this 'wait for Serial' if you need to Serial.print during loading
	// while( !Serial );

	// Serial.println( "read the header" );
	bool bContainedData = loadHeader();

	if (bContainedData)
	{
		// Serial.println( "loading patterns" );
		loadPatterns();
		return true;
	}

	// Serial.println( "failed to load" );

	return false;
}
