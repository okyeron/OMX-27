// OMX-27 MIDI KEYBOARD / SEQUENCER

//	v1.13.3
//	Last update: Feb 2024
//
//	Original concept and initial code by Steven Noreyko
//  Additional code contributions:
// 		Matt Boone, Steven Zydek,
// 		Chris Atkins, Will Winder,
// 		Michael P Jones
//
//	Big thanks to:
//	John Park and Gerald Stevens for initial testing and feature ideas
//	mzero for immense amounts of code coaching/assistance
//	drjohn for support
//

#include <functional>
#include <ResponsiveAnalogRead.h>
#include "src/consts/consts.h"
#include "src/config.h"
#include "src/consts/colors.h"
#include "src/midi/midi.h"
#include "src/ClearUI/ClearUI.h"
#include "src/modes/sequencer.h"
#include "src/midi/noteoffs.h"
#include "src/hardware/storage.h"
#include "src/midi/sysex.h"
#include "src/hardware/omx_keypad.h"
#include "src/utils/omx_util.h"
#include "src/utils/cvNote_util.h"
#include "src/hardware/omx_disp.h"
#include "src/modes/omx_mode_midi_keyboard.h"
#include "src/modes/omx_mode_drum.h"
#include "src/modes/omx_mode_sequencer.h"
#include "src/modes/omx_mode_grids.h"
#include "src/modes/omx_mode_euclidean.h"
#include "src/modes/omx_mode_chords.h"
#include "src/modes/omx_screensaver.h"
#include "src/hardware/omx_leds.h"
#include "src/utils/music_scales.h"

// Allows code to compile with smallest code LTO
extern "C"{
  int _getpid(){ return -1;}
  int _kill(int pid, int sig){ return -1; }
  int _write(){return -1;}
}

// #define RAM_MONITOR
// #ifdef RAM_MONITOR
// #include "src/utils/RamMonitor.h"
// #endif

OmxModeMidiKeyboard omxModeMidi;
OmxModeDrum omxModeDrum;
OmxModeSequencer omxModeSeq;
#ifdef OMXMODEGRIDS
OmxModeGrids omxModeGrids;
#endif
OmxModeEuclidean omxModeEuclid;
OmxModeChords omxModeChords;

OmxModeInterface *activeOmxMode;

OmxScreensaver omxScreensaver;

MusicScales globalScale;

// storage of pot values; current is in the main loop; last value is for midi output
int volatile currentValue[NUM_CC_POTS];
int lastMidiValue[NUM_CC_POTS];

int temp;

Micros lastProcessTime;

uint8_t RES;
uint16_t AMAX;
int V_scale;

// ENCODER
Encoder myEncoder(12, 11); // encoder pins on hardware
const int buttonPin = 0;
int buttonState = 1;
Button encButton(buttonPin);

// long newPosition = 0;
// long oldPosition = -999;

// KEYPAD
// initialize an instance of custom Keypad class
unsigned long longPressInterval = 800;
unsigned long clickWindow = 200;
OMXKeypad keypad(longPressInterval, clickWindow, makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// setup EEPROM/FRAM storage
Storage *storage;
SysEx *sysEx;

#ifdef RAM_MONITOR
RamMonitor ram;
uint32_t reporttime;

void report_ram_stat(const char *aname, uint32_t avalue)
{
	Serial.print(aname);
	Serial.print(": ");
	Serial.print((avalue + 512) / 1024);
	Serial.print(" Kb (");
	Serial.print((((float)avalue) / ram.total()) * 100, 1);
	Serial.println("%)");
};

void report_profile_time(const char *aname, uint32_t avalue)
{
	Serial.print(aname);
	Serial.print(": ");
	Serial.print(avalue);
	Serial.println("\n");
};

void report_ram()
{
	bool lowmem;
	bool crash;

	Serial.println("==== memory report ====");

	report_ram_stat("free", ram.adj_free());
	report_ram_stat("stack", ram.stack_total());
	report_ram_stat("heap", ram.heap_total());

	lowmem = ram.warning_lowmem();
	crash = ram.warning_crash();
	if (lowmem || crash)
	{
		Serial.println();

		if (crash)
			Serial.println("**warning: stack and heap crash possible");
		else if (lowmem)
			Serial.println("**warning: unallocated memory running low");
	};

	Serial.println();
};
#endif



// ####### SEQUENCER LEDS #######

void changeOmxMode(OMXMode newOmxmode)
{
//	Serial.println((String)"NewMode: " + newOmxmode);
	sysSettings.omxMode = newOmxmode;
	sysSettings.newmode = newOmxmode;

	if(activeOmxMode != nullptr)
	{
		activeOmxMode->onModeDeactivated();
	}

	switch (newOmxmode)
	{
	case MODE_MIDI:
		omxModeMidi.setMidiMode();
		activeOmxMode = &omxModeMidi;
		break;
	case MODE_DRUM:
		activeOmxMode = &omxModeDrum;
		break;
	case MODE_CHORDS:
		activeOmxMode = &omxModeChords;
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
	case MODE_GRIDS:
#ifdef OMXMODEGRIDS
		activeOmxMode = &omxModeGrids;
#endif
		break;
	case MODE_EUCLID:
		activeOmxMode = &omxModeEuclid;
		break;
	default:
		omxModeMidi.setMidiMode();
		activeOmxMode = &omxModeMidi;
		break;
	}

	activeOmxMode->onModeActivated();

	omxLeds.setDirty();
	omxDisp.setDirty();
}

// ####### END LEDS



// ####### POTENTIOMETERS #######
void readPotentimeters()
{
	for (int k = 0; k < potCount; k++)
	{
		int prevValue = potSettings.analogValues[k];
		int prevAnalog = potSettings.analog[k]->getValue();

		temp = analogRead(analogPins[k]);
		potSettings.analog[k]->update(temp);

		// read from the smoother, constrain (to account for tolerances), and map it
		temp = potSettings.analog[k]->getValue();
		temp = constrain(temp, potMinVal, potMaxVal);
		temp = map(temp, potMinVal, potMaxVal, 0, 16383);
		potSettings.hiResPotVal[k] = temp;

		// map and update the value
		potSettings.analogValues[k] = temp >> 7;

		int newAnalog = potSettings.analog[k]->getValue();

		// delta is way smaller on T4 - what to do??
		int analogDelta = abs(newAnalog - prevAnalog);


		// if (k == 1)
		// {
		// 	Serial.print(analogPins[k]);
		// 	Serial.print(" ");
		// 	Serial.print(temp);
		// 	Serial.print(" ");
		// 	Serial.print(potSettings.analogValues[k]);
		// 	Serial.print("\n");
		// }

		if (potSettings.analog[k]->hasChanged())
		{
			// do stuff
			if (sysSettings.screenSaverMode)
			{
				omxScreensaver.onPotChanged(k, prevValue, potSettings.analogValues[k], analogDelta);
			}
			// don't send pots in screensaver
			else
			{
				activeOmxMode->onPotChanged(k, prevValue, potSettings.analogValues[k], analogDelta);
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
		cvNoteUtil.cvNoteOn(note);
	}

	omxScreensaver.resetCounter();

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
		cvNoteUtil.cvNoteOff(note);
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
	// Last is 28

	uint8_t midiMacroChan = (uint8_t)(midiMacroConfig.midiMacroChan - 1);
	storage->write(EEPROM_HEADER_ADDRESS + 29, midiMacroChan);

	uint8_t midiMacroId = (uint8_t)midiMacroConfig.midiMacro;
	storage->write(EEPROM_HEADER_ADDRESS + 30, midiMacroId);

	uint8_t scaleRoot = (uint8_t)scaleConfig.scaleRoot;
	storage->write(EEPROM_HEADER_ADDRESS + 31, scaleRoot);

	uint8_t scalePattern = (uint8_t)scaleConfig.scalePattern;
	storage->write(EEPROM_HEADER_ADDRESS + 32, scalePattern);

	uint8_t lockScale = (uint8_t)scaleConfig.lockScale;
	storage->write(EEPROM_HEADER_ADDRESS + 33, lockScale);

	uint8_t scaleGrp16 = (uint8_t)scaleConfig.group16 ;
	storage->write(EEPROM_HEADER_ADDRESS + 34, scaleGrp16);

	storage->write(EEPROM_HEADER_ADDRESS + 35, midiSettings.defaultVelocity);

	storage->write(EEPROM_HEADER_ADDRESS + 36, clockConfig.globalQuantizeStepIndex);

	storage->write(EEPROM_HEADER_ADDRESS + 37, cvNoteUtil.triggerMode);

	// 38 bytes
}

// returns true if the header contained initialized data
// false means we shouldn't attempt to load any further information
bool loadHeader(void)
{
	uint8_t version = storage->read(EEPROM_HEADER_ADDRESS + 0);

		char buf[64];
		snprintf( buf, sizeof(buf), "EEPROM Header Version is %d\n", version );
		Serial.print( buf );

	// Uninitalized EEPROM memory is filled with 0xFF
	if (version == 0xFF)
	{
		// EEPROM was uninitialized
				Serial.println( "version was 0xFF" );
		return false;
	}

	if (version != EEPROM_VERSION)
	{
		// write an adapter if we ever need to increment the EEPROM version and also save the existing patterns
		// for now, return false will essentially reset the state
				Serial.println( "version not matched" );
		return false;
	}

	sysSettings.omxMode = (OMXMode)storage->read(EEPROM_HEADER_ADDRESS + 1);

	sequencer.playingPattern = storage->read(EEPROM_HEADER_ADDRESS + 2);
	sysSettings.playingPattern = sequencer.playingPattern;

	uint8_t unMidiChannel = storage->read(EEPROM_HEADER_ADDRESS + 3);
	sysSettings.midiChannel = unMidiChannel + 1;

	Serial.println( "Loading banks" );
	for (int b = 0; b < NUM_CC_BANKS; b++)
	{
		for (int i = 0; i < NUM_CC_POTS; i++)
		{
			pots[b][i] = storage->read(EEPROM_HEADER_ADDRESS + 4 + i + (5 * b));
		}
	}

	uint8_t midiMacroChannel = storage->read(EEPROM_HEADER_ADDRESS + 29);
	midiMacroConfig.midiMacroChan = midiMacroChannel + 1;

	uint8_t midiMacro = storage->read(EEPROM_HEADER_ADDRESS + 30);
	midiMacroConfig.midiMacro = midiMacro;

	uint8_t scaleRoot = storage->read(EEPROM_HEADER_ADDRESS + 31);
	scaleConfig.scaleRoot = scaleRoot;

	int8_t scalePattern = (int8_t)storage->read(EEPROM_HEADER_ADDRESS + 32);
	scaleConfig.scalePattern = scalePattern;

	bool lockScale = (bool)storage->read(EEPROM_HEADER_ADDRESS + 33);
	scaleConfig.lockScale = lockScale;

	bool scaleGrp16 = (bool)storage->read(EEPROM_HEADER_ADDRESS + 34);
	scaleConfig.group16 = scaleGrp16;

	globalScale.calculateScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);

	midiSettings.defaultVelocity = storage->read(EEPROM_HEADER_ADDRESS + 35);

	clockConfig.globalQuantizeStepIndex = constrain(storage->read(EEPROM_HEADER_ADDRESS + 36), 0, kNumArpRates - 1);

	cvNoteUtil.triggerMode = constrain(storage->read(EEPROM_HEADER_ADDRESS + 37), 0, 1);

	return true;
}

void savePatterns(void)
{
	bool isEeprom = storage->isEeprom();

	int patternSize = serializedPatternSize(isEeprom);
	int nLocalAddress = EEPROM_PATTERN_ADDRESS;

	// Serial.println((String)"Seq patternSize: " + patternSize);
	int seqPatternNum = isEeprom ? NUM_SEQ_PATTERNS_EEPROM : NUM_SEQ_PATTERNS;

	for (int i = 0; i < seqPatternNum; i++)
	{
		auto pattern = (byte *)sequencer.getPattern(i);
		for (int j = 0; j < patternSize; j++)
		{
			storage->write(nLocalAddress + j, *pattern++);
		}

		nLocalAddress += patternSize;
	}

	if(isEeprom)
	{
		return;
	}
	Serial.println((String)"nLocalAddress: " + nLocalAddress); // 5784

#ifdef OMXMODEGRIDS
	Serial.println("Saving Grids");

	// Grids patterns
	patternSize = OmxModeGrids::serializedPatternSize(isEeprom);
	int numPatterns = OmxModeGrids::getNumPatterns();

	// Serial.println((String)"OmxModeGrids patternSize: " + patternSize);
	// Serial.println((String)"numPatterns: " + numPatterns);

	for (int i = 0; i < numPatterns; i++)
	{
		auto pattern = (byte *)omxModeGrids.getPattern(i);
		for (int j = 0; j < patternSize; j++)
		{
			storage->write(nLocalAddress + j, *pattern++);
		}

		nLocalAddress += patternSize;
	}
	Serial.println((String)"nLocalAddress: " + nLocalAddress); // 6008
#endif

	Serial.println("Saving Euclidean");
	nLocalAddress = omxModeEuclid.saveToDisk(nLocalAddress, storage);
	Serial.println((String)"nLocalAddress: " + nLocalAddress); // 7433

	Serial.println("Saving Chords");
	nLocalAddress = omxModeChords.saveToDisk(nLocalAddress, storage);
	Serial.println((String)"nLocalAddress: " + nLocalAddress); // 10505

	Serial.println("Saving Drums");
	nLocalAddress = omxModeDrum.saveToDisk(nLocalAddress, storage);
	Serial.println((String)"nLocalAddress: " + nLocalAddress); // 11545

	Serial.println("Saving MidiFX");
	for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		nLocalAddress = subModeMidiFx[i].saveToDisk(nLocalAddress, storage);
		// Serial.println((String)"Saved: " + i);
		// Serial.println((String)"nLocalAddress: " + nLocalAddress);
	}
	Serial.println((String)"nLocalAddress: " + nLocalAddress); // 11585

	// Starting 11545
	// MidiFX with nothing 11585
	// 1 MidiFX full ARPS 11913
	// 
	// OMX Frooze/Ran out of memory after creating 4 x 8 - 3 = 29  ARPs
	// Maybe build in a limit of 2 or one arps per MidiFX, or just recommend users not to
	// create 29 ARPs. 

	// Seq patternSize: 715
	// nLocalAddress: 5752
	// size of patterns: 5720
	// OmxModeGrids patternSize: 23
	// numPatterns: 8
	// nLocalAddress: 5936
	// size of grids: 184

}

void loadPatterns(void)
{
	bool isEeprom = storage->isEeprom();

	int patternSize = serializedPatternSize(isEeprom);
	int nLocalAddress = EEPROM_PATTERN_ADDRESS;

	Serial.print( "Seq patterns - nLocalAddress: " );
	Serial.println( nLocalAddress );

	int seqPatternNum = isEeprom ? NUM_SEQ_PATTERNS_EEPROM : NUM_SEQ_PATTERNS;

	for (int i = 0; i < seqPatternNum; i++)
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

	if(isEeprom)
	{
		return;
	}

	Serial.print( "Grids patterns - nLocalAddress: " );
	Serial.println( nLocalAddress );
	// 332 - eeprom size
	// 332 * 8 = 2656

	// Grids patterns
#ifdef OMXMODEGRIDS
	patternSize = OmxModeGrids::serializedPatternSize(isEeprom);
	int numPatterns = OmxModeGrids::getNumPatterns();

	for (int i = 0; i < numPatterns; i++)
	{
		auto pattern = grids::SnapShotSettings{};
		auto current = (byte *)&pattern;
		for (int j = 0; j < patternSize; j++)
		{
			*current = storage->read(nLocalAddress + j);
			current++;
		}
		omxModeGrids.setPattern(i, pattern);
		nLocalAddress += patternSize;
	}
#endif

	Serial.print( "Pattern size: " );
	Serial.print( patternSize );

	Serial.print( " - nLocalAddress: " );
	Serial.println( nLocalAddress );

	Serial.print("Loading Euclidean - ");
	nLocalAddress = omxModeEuclid.loadFromDisk(nLocalAddress, storage);
	Serial.println((String)"nLocalAddress: " + nLocalAddress); // 5988

	Serial.print("Loading Chords - ");
	nLocalAddress = omxModeChords.loadFromDisk(nLocalAddress, storage);
	Serial.println((String)"nLocalAddress: " + nLocalAddress); // 5988

	Serial.print("Loading Drums - ");
	nLocalAddress = omxModeDrum.loadFromDisk(nLocalAddress, storage);
	Serial.println((String)"nLocalAddress: " + nLocalAddress); // 5988

	// Serial.println((String)"nLocalAddress: " + nLocalAddress); // 5968

	Serial.print("Loading MidiFX - ");
	for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		nLocalAddress = subModeMidiFx[i].loadFromDisk(nLocalAddress, storage);
		// Serial.println((String)"Loaded: " + i);
		// Serial.println((String)"nLocalAddress: " + nLocalAddress);
	}
	Serial.println((String)"nLocalAddress: " + nLocalAddress); // 5988

	// with 8 note chords, 10929

	// Pattern size = 715
	// Pattern size eprom = 332
	// Total size of patterns = 5720
	// Total storage size = 5749
	// Fram = 32000 = 26251 available
	// Eeprom = 2048
	// Eeprom rom can save 6 patterns, plus 56 bytes

	// 2832 - size of 16 euclid patterns of 16 euclids

	// no arps = 9905, 5 arps = 10105, 25 arps = 11505

	// no arps = 10929, 5 arps = 11129, 25 arps = 12529
	//

}

// currently saves everything ( mode + patterns )
void saveToStorage(void)
{
	Serial.println( "Saving to Storage..." );
	saveHeader();
	savePatterns();
}

// currently loads everything ( mode + patterns )
bool loadFromStorage(void)
{
	// This load can happen soon after Serial.begin - enable this 'wait for Serial' if you need to Serial.print during loading
	// while( !Serial );

	Serial.println( "Read the header" );
	bool bContainedData = loadHeader();

	if (bContainedData)
	{
		Serial.println( "Loading patterns" );
		loadPatterns();
		changeOmxMode(sysSettings.omxMode);

		omxDisp.isDirty();
		omxLeds.isDirty();
		return true;
	}

	Serial.println( "-- Failed to load --" );

	omxDisp.isDirty();
	omxLeds.isDirty();

	return false;
}

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

	seqConfig.currentFrameMicros = micros();
	// Micros timeStart = micros();
	activeOmxMode->loopUpdate(passed);
	cvNoteUtil.loopUpdate(passed);

	if (passed > 0) // This should always be true
	{
		if (sequencer.playing || omxUtil.areClocksRunning())
		{
			omxScreensaver.resetCounter(); // screenSaverCounter = 0;
		}
		omxUtil.advanceClock(activeOmxMode, passed);
		omxUtil.advanceSteps(passed);
	}

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

	bool omxModeChangedThisFrame = false;

	// ############### EXTERNAL MODE CHANGE / SYSEX ###############
	if ((!encoderConfig.enc_edit && (sysSettings.omxMode != sysSettings.newmode)) || sysSettings.refresh)
	{
		sysSettings.newmode = sysSettings.omxMode;
		changeOmxMode(sysSettings.omxMode);
		omxModeChangedThisFrame = true;

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
		auto amt = u.accel(1);		   // where 5 is the acceleration factor if you want it, 0 if you don't)
		omxScreensaver.resetCounter(); // screenSaverCounter = 0;
									   //    	Serial.println(u.dir() < 0 ? "ccw " : "cw ");
									   //    	Serial.println(amt);

		// Change Mode
		if (encoderConfig.enc_edit)
		{
			// set mode
			//			int modesize = NUM_OMX_MODES;
			sysSettings.newmode = (OMXMode)constrain(sysSettings.newmode + amt, 0, NUM_OMX_MODES - 1);
			// omxDisp.dispMode();
			// omxDisp.bumpDisplayTimer();
			omxDisp.setDirty();
			omxLeds.setDirty();
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
			omxModeChangedThisFrame = true;
			seqStop();
			omxLeds.setAllLEDS(0, 0, 0);
			encoderConfig.enc_edit = false;
			// omxDisp.dispMode();
			omxDisp.setDirty();
		}
		else if (encoderConfig.enc_edit)
		{
			encoderConfig.enc_edit = false;
		}

		// Prevents toggling encoder select when entering mode
		if (!omxModeChangedThisFrame)
		{
			activeOmxMode->onEncoderButtonDown();
		}

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
			// Enter mode change
			encoderConfig.enc_edit = true;
			sysSettings.newmode = sysSettings.omxMode;
			omxLeds.setAllLEDS(0, 0, 0);
			omxDisp.setDirty();
			// omxDisp.dispMode();
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
		bool keyConsumed = false;
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
			omxDisp.displayMessage("Saving...");
			omxDisp.isDirty();
			omxDisp.showDisplay();
			saveToStorage();
			//	Serial.println("EEPROM saved");
			omxDisp.displayMessage("Saved State");
			encoderConfig.enc_edit = false;
			omxLeds.setAllLEDS(0,0,0);
			activeOmxMode->onModeActivated();
			omxDisp.isDirty();
			omxLeds.isDirty();
			keyConsumed = true;
		}

		if (!keyConsumed)
		{
			activeOmxMode->onKeyUpdate(e);
		}

		// END MODE SWITCH

		if (!e.down())
		{
			midiSettings.keyState[thisKey] = false;
		}

		// ### LONG KEY SWITCH PRESS
		if (e.held() && !keyConsumed)
		{
			// DO LONG PRESS THINGS
			activeOmxMode->onKeyHeldUpdate(e); // Only the sequencer uses this, could probably be handled in onKeyUpdate() but keyStates are modified before this stuff happens.
		}									   // END IF HELD

	} // END KEYS WHILE

	if (!sysSettings.screenSaverMode)
	{
		omxLeds.updateBlinkStates();
		omxDisp.UpdateMessageTextTimer();

		if (encoderConfig.enc_edit)
		{
			omxDisp.dispMode();
		}
		else
		{
			activeOmxMode->onDisplayUpdate();
		}
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

	// Micros elapsed = micros() - timeStart;
	// if ((timeStart - reporttime) > 2000)
	// {
	// 	report_profile_time("Elapsed", elapsed);
	// 	reporttime = timeStart;
	// 	// report_ram();
	// };



#ifdef RAM_MONITOR
	uint32_t time = millis();

	if ((time - reporttime) > 2000)
	{
		reporttime = time;
		report_ram();
	};

	ram.run();
#endif

} // ######## END MAIN LOOP ########

// ####### SETUP #######

void setup()
{
	Serial.begin(115200);
	//	while( !Serial );
#if T4
	Serial.println("Teensy 4.0");
// 	Serial.println("DAC Start!");
	dac.begin(DAC_ADDR);
#else
	Serial.println("Teensy 3.2");
#endif
	storage = Storage::initStorage();
	sysEx = new SysEx(storage, &sysSettings);

#ifdef RAM_MONITOR
	ram.initialize();
#endif

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
	omxUtil.subModeClearStorage.setStoragePtr(storage);

	// HW MIDI
	MM::begin();

	randomSeed(analogRead(13));
	srand(analogRead(13));

	// SET ANALOG READ resolution to teensy's 13 usable bits
#if T4
	analogReadResolution(10); // Teensy 4 = 10 bits
#else
	analogReadResolution(13); // Teensy 3.x = 13 bits
#endif

	// CV GATE pin
	pinMode(CVGATE_PIN, OUTPUT);
	// ENCODER BUTTON pin
	pinMode(buttonPin, INPUT_PULLUP);

// initialize ANALOG INPUTS and ResponsiveAnalogRead
	for (int i = 0; i < potCount; i++)
	{
// 		potSettings.analog[i] = new ResponsiveAnalogRead(0, true, .001);
// 		potSettings.analog[i]->setAnalogResolution(1 << 13);
		pinMode(analogPins[i], INPUT);
		potSettings.analog[i] = new ResponsiveAnalogRead(analogPins[i], true, .001);

		#if T4
//			potSettings.analog[i]->setAnalogResolution(10);
//			potSettings.analog[i]->setActivityThreshold(8);
		#else
			potSettings.analog[i]->setAnalogResolution(1 << 13);
			potSettings.analog[i]->setActivityThreshold(32);
		#endif

		currentValue[i] = 0;
		lastMidiValue[i] = 0;
	}

	// set DAC Resolution CV/GATE
	RES = 12;
	AMAX = pow(2, RES);
	V_scale = 64; // pow(2,(RES-7)); 4095 max

#if T4
	dac.setVoltage(0, false);
#else
	analogWriteResolution(RES); // set resolution for DAC
	analogWrite(CVPITCH_PIN, 0);
#endif

	globalScale.calculateScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
	omxModeMidi.SetScale(&globalScale);
	omxModeDrum.SetScale(&globalScale);
	omxModeSeq.SetScale(&globalScale);
#ifdef OMXMODEGRIDS
	omxModeGrids.SetScale(&globalScale);
#endif
	omxModeEuclid.SetScale(&globalScale);
	omxModeChords.SetScale(&globalScale);

	// Load from EEPROM
	bool bLoaded = loadFromStorage();
	if (!bLoaded)
	{
		Serial.println( "Init load fail. Reinitializing" );

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

	// changeOmxMode(MODE_EUCLID);

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

#ifdef RAM_MONITOR
	reporttime = millis();
#endif
}

// ####### END SETUP #######
