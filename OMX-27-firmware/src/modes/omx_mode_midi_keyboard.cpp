#include "omx_mode_midi_keyboard.h"
#include "../config.h"
#include "../consts/colors.h"
#include "../utils/omx_util.h"
#include "../hardware/omx_disp.h"
#include "../hardware/omx_leds.h"
#include "../midi/midi.h"
#include "../utils/music_scales.h"
#include "../midi/noteoffs.h"
// #include "sequencer.h"

// const int kSelMidiFXOffColor = SALMON;
// const int kMidiFXOffColor = RED;

// const int kSelMidiFXColor = LTCYAN;
// const int kMidiFXColor = BLUE;

enum MIKeyModePage {
    MIPAGE_OUTMIDI,
    MIPAGE_MIDIINSPECT,
    MIPAGE_OUTCC,
    MIPAGE_POTSANDMACROS,
    MIPAGE_SCALES,
    MIPAGE_CFG
};

OmxModeMidiKeyboard::OmxModeMidiKeyboard()
{
	// Add 4 pages
	params.addPage(3); // Oct, CH, Vel
	params.addPage(1); // Sent Pot CC, Last Note, Last Vel, Not editable, just FYI
	params.addPage(4); // RR - Midi Round Robin, RROF - Round Robin Offset, PGm, BNK
	params.addPage(4); // PotBank, Thru, Macro, Macro Channel
	params.addPage(4); // Root, Scale, Lock Scale Notes, Group notes. 
	params.addPage(4); // Pot CC CFG

	// subModeMidiFx.setNoteOutputFunc(&OmxModeMidiKeyboard::onNotePostFXForwarder, this);

	m8Macro_.setDoNoteOn(&OmxModeMidiKeyboard::doNoteOnForwarder, this);
	m8Macro_.setDoNoteOff(&OmxModeMidiKeyboard::doNoteOffForwarder, this);
	nornsMarco_.setDoNoteOn(&OmxModeMidiKeyboard::doNoteOnForwarder, this);
	nornsMarco_.setDoNoteOff(&OmxModeMidiKeyboard::doNoteOffForwarder, this);
}

void OmxModeMidiKeyboard::InitSetup()
{
	initSetup = true;
}

void OmxModeMidiKeyboard::onModeActivated()
{
	// auto init when activated
	if (!initSetup)
	{
		InitSetup();
	}

	// sequencer.playing = false;
	stopSequencers();

	omxLeds.setDirty();
	omxDisp.setDirty();

	for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		subModeMidiFx[i].setEnabled(true);
		subModeMidiFx[i].onModeChanged();
		subModeMidiFx[i].setNoteOutputFunc(&OmxModeMidiKeyboard::onNotePostFXForwarder, this);
	}

	pendingNoteOffs.setNoteOffFunction(&OmxModeMidiKeyboard::onPendingNoteOffForwarder, this);

	params.setSelPageAndParam(0, 0);
	encoderSelect = true;

	selectMidiFx(mfxIndex_, false);
}

void OmxModeMidiKeyboard::onModeDeactivated()
{
	// sequencer.playing = false;
	stopSequencers();

	for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		subModeMidiFx[i].setEnabled(false);
		subModeMidiFx[i].onModeChanged();
	}
}

void OmxModeMidiKeyboard::stopSequencers()
{
	omxUtil.stopClocks();
	// MM::stopClock();
	pendingNoteOffs.allOff();
}

void OmxModeMidiKeyboard::selectMidiFx(uint8_t mfxIndex, bool dispMsg)
{
	this->mfxIndex_ = mfxIndex;

	for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		subModeMidiFx[i].setSelected(i == mfxIndex);
	}

	if (dispMsg)
	{
		if (mfxIndex < NUM_MIDIFX_GROUPS)
		{
			omxDisp.displayMessageTimed("MidiFX " + String(mfxIndex + 1), 5);
		}
		else
		{
			omxDisp.displayMessageTimed("MidiFX Off", 5);
		}
	}
}

// void OmxModeMidiKeyboard::changePage(int amt)
// {
//     midiPageParams.mmpage = constrain(midiPageParams.mmpage + amt, 0, midiPageParams.numPages - 1);
//     midiPageParams.miparam = midiPageParams.mmpage * NUM_DISP_PARAMS;
// }

// void OmxModeMidiKeyboard::setParam(int paramIndex)
// {
//     if (paramIndex >= 0)
//     {
//         midiPageParams.miparam = paramIndex % midiPageParams.numParams;
//     }
//     else
//     {
//         midiPageParams.miparam = (paramIndex + midiPageParams.numParams) % midiPageParams.numParams;
//     }

//     // midiPageParams.miparam  = (midiPageParams.miparam  + 1) % 15;
//     midiPageParams.mmpage = midiPageParams.miparam / NUM_DISP_PARAMS;
// }

void OmxModeMidiKeyboard::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
{
	if (isSubmodeEnabled() && activeSubmode->usesPots())
	{
		activeSubmode->onPotChanged(potIndex, prevValue, newValue, analogDelta);
		return;
	}

	auto activeMacro = getActiveMacro();

	bool macroConsumesPots = false;
	if (activeMacro != nullptr)
	{
		macroConsumesPots = activeMacro->consumesPots();
	}

	// Note, these get sent even if macro mode is not active
	if (macroConsumesPots)
	{
		activeMacro->onPotChanged(potIndex, prevValue, newValue, analogDelta);
	}
	else
	{
		omxUtil.sendPots(potIndex, sysSettings.midiChannel);
	}

	// if (midiMacroConfig.midiMacro)
	// {
	//     omxUtil.sendPots(potIndex, midiMacroConfig.midiMacroChan);
	// }
	// else
	// {
	// }

	omxDisp.setDirty();
}

void OmxModeMidiKeyboard::onClockTick()
{
	for (uint8_t i = 0; i < 5; i++)
	{
		// Lets them do things in background
		subModeMidiFx[i].onClockTick();
	}
}

void OmxModeMidiKeyboard::loopUpdate(Micros elapsedTime)
{

	// if (elapsedTime > 0)
	// {
	// 	if (!sequencer.playing)
	// 	{
	//         // Needed to make pendingNoteOns/pendingNoteOffs work
	// 		omxUtil.advanceSteps(elapsedTime);
	// 	}
	// }

	for (uint8_t i = 0; i < 5; i++)
	{
		// Lets them do things in background
		subModeMidiFx[i].loopUpdate();
	}

	// Can be modified by scale MidiFX
	musicScale->calculateScaleIfModified(scaleConfig.scaleRoot, scaleConfig.scalePattern);

	// if (isSubmodeEnabled())
	// {
	//     activeSubmode->loopUpdate();
	// }
}

// Handles selecting params using encoder
// void OmxModeMidiKeyboard::onEncoderChangedSelectParam(Encoder::Update enc)
// {
//     if(enc.dir() == 0) return;

//     if (enc.dir() < 0) // if turn CCW
//     {
//         params.decrementParam();
//     }
//     else if (enc.dir() > 0) // if turn CW
//     {
//         params.incrementParam();
//     }

//     omxDisp.setDirty();
// }

void OmxModeMidiKeyboard::onEncoderChanged(Encoder::Update enc)
{
	if (isSubmodeEnabled())
	{
		activeSubmode->onEncoderChanged(enc);
		return;
	}

	bool macroConsumesDisplay = false;

	if (macroActive_ && activeMacro_ != nullptr)
	{
		macroConsumesDisplay = activeMacro_->consumesDisplay();
	}

	if (macroConsumesDisplay)
	{
		activeMacro_->onEncoderChanged(enc);
		return;
	}

	if (encoderSelect && !midiSettings.midiAUX)
	{
		// onEncoderChangedSelectParam(enc);
		params.changeParam(enc.dir());
		omxDisp.setDirty();
		return;
	}

	if (organelleMotherMode)
	{
		// CHANGE PAGE
		if (params.getSelParam() == 0)
		{
			if (enc.dir() < 0)
			{ // if turn ccw
				MM::sendControlChange(CC_OM2, 0, sysSettings.midiChannel);
			}
			else if (enc.dir() > 0)
			{ // if turn cw
				MM::sendControlChange(CC_OM2, 127, sysSettings.midiChannel);
			}
		}

		omxDisp.setDirty();
	}

	// if (midiSettings.midiAUX)
	// {
	//     // if (enc.dir() < 0)
	//     // { // if turn ccw
	//     //     setParam(midiPageParams.miparam - 1);
	//     //     omxDisp.setDirty();
	//     // }
	//     // else if (enc.dir() > 0)
	//     // { // if turn cw
	//     //     setParam(midiPageParams.miparam + 1);
	//     //     omxDisp.setDirty();
	//     // }

	//     // change MIDI Background Color
	//     // midiBg_Hue = constrain(midiBg_Hue + (amt * 32), 0, 65534); // 65535
	//     return; // break;
	// }

	auto amt = enc.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)

	int8_t selPage = params.getSelPage(); 
	int8_t selParam = params.getSelParam() + 1; // Add one for readability

	if (selPage == MIPAGE_OUTMIDI)
	{
		if (selParam == 1)
		{
			// set octave
			midiSettings.octave = constrain(midiSettings.octave + amt, -5, 4);
		}
		else if (selParam == 2)
		{
			int newchan = constrain(sysSettings.midiChannel + amt, 1, 16);
			if (newchan != sysSettings.midiChannel) // Is this if necessary?
			{
				sysSettings.midiChannel = newchan;
			}
		}
		else if (selParam == 3)
		{
			midiSettings.defaultVelocity = constrain((int)midiSettings.defaultVelocity + amt, 0, 127); // cast to int to prevent rollover
		}
	}
	else if (selPage == MIPAGE_OUTCC)
	{
		if (selParam == 1)
		{
			int newrrchan = constrain(midiSettings.midiRRChannelCount + amt, 1, 16);
			if (newrrchan != midiSettings.midiRRChannelCount)
			{
				midiSettings.midiRRChannelCount = newrrchan;
				if (midiSettings.midiRRChannelCount == 1)
				{
					midiSettings.midiRoundRobin = false;
				}
				else
				{
					midiSettings.midiRoundRobin = true;
				}
			}
		}
		else if (selParam == 2)
		{
			midiSettings.midiRRChannelOffset = constrain(midiSettings.midiRRChannelOffset + amt, 0, 15);
		}
		else if (selParam == 3)
		{
			midiSettings.currpgm = constrain(midiSettings.currpgm + amt, 0, 127);

			if (midiSettings.midiRoundRobin)
			{
				for (int q = midiSettings.midiRRChannelOffset + 1; q < midiSettings.midiRRChannelOffset + midiSettings.midiRRChannelCount + 1; q++)
				{
					MM::sendProgramChange(midiSettings.currpgm, q);
				}
			}
			else
			{
				MM::sendProgramChange(midiSettings.currpgm, sysSettings.midiChannel);
			}
		}
		else if (selParam == 4)
		{
			midiSettings.currbank = constrain(midiSettings.currbank + amt, 0, 127);
			// Bank Select is 2 mesages
			MM::sendControlChange(0, 0, sysSettings.midiChannel);
			MM::sendControlChange(32, midiSettings.currbank, sysSettings.midiChannel);
			MM::sendProgramChange(midiSettings.currpgm, sysSettings.midiChannel);
		}
	}
	else if (selPage == MIPAGE_POTSANDMACROS)
	{
		if (selParam == 1)
		{
			potSettings.potbank = constrain(potSettings.potbank + amt, 0, NUM_CC_BANKS - 1);
		}
		if (selParam == 2)
		{
			midiSettings.midiSoftThru = constrain(midiSettings.midiSoftThru + amt, 0, 1);
		}
		if (selParam == 3)
		{
			midiMacroConfig.midiMacro = constrain(midiMacroConfig.midiMacro + amt, 0, nummacromodes);
		}
		if (selParam == 4)
		{
			midiMacroConfig.midiMacroChan = constrain(midiMacroConfig.midiMacroChan + amt, 1, 16);
		}
	}
	else if (selPage == MIPAGE_SCALES)
	{
		if (selParam == 1)
		{
			int prevRoot = scaleConfig.scaleRoot;
			scaleConfig.scaleRoot = constrain(scaleConfig.scaleRoot + amt, 0, 12 - 1);
			if (prevRoot != scaleConfig.scaleRoot)
			{
				musicScale->calculateScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
			}
		}
		if (selParam == 2)
		{
			int prevPat = scaleConfig.scalePattern;
			scaleConfig.scalePattern = constrain(scaleConfig.scalePattern + amt, -1, musicScale->getNumScales() - 1);
			if (prevPat != scaleConfig.scalePattern)
			{
				omxDisp.displayMessage(musicScale->getScaleName(scaleConfig.scalePattern));
				musicScale->calculateScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
			}
		}
		if (selParam == 3)
		{
			scaleConfig.lockScale = constrain(scaleConfig.lockScale + amt, 0, 1);
		}
		if (selParam == 4)
		{
			scaleConfig.group16 = constrain(scaleConfig.group16 + amt, 0, 1);
		}
	}

	omxDisp.setDirty();
}

void OmxModeMidiKeyboard::onEncoderButtonDown()
{
	if (isSubmodeEnabled())
	{
		activeSubmode->onEncoderButtonDown();
		return;
	}

	bool macroConsumesDisplay = false;
	if (macroActive_ && activeMacro_ != nullptr)
	{
		macroConsumesDisplay = activeMacro_->consumesDisplay();
	}

	if (macroConsumesDisplay)
	{
		activeMacro_->onEncoderButtonDown();
		return;
	}

	if (params.getSelPage() == MIPAGE_CFG && params.getSelParam() == 0)
	{
		enableSubmode(&subModePotConfig_);
		omxDisp.isDirty();
		return;
	}

	encoderSelect = !encoderSelect;
	omxDisp.isDirty();
}

void OmxModeMidiKeyboard::onEncoderButtonUp()
{
	if (organelleMotherMode)
	{
		//				MM::sendControlChange(CC_OM1,0,sysSettings.midiChannel);
	}
}

void OmxModeMidiKeyboard::onEncoderButtonDownLong()
{
}

bool OmxModeMidiKeyboard::shouldBlockEncEdit()
{
	if (isSubmodeEnabled())
	{
		return activeSubmode->shouldBlockEncEdit();
	}

	if (macroActive_)
	{
		return true;
	}

	return false;
}

void OmxModeMidiKeyboard::onKeyUpdate(OMXKeypadEvent e)
{
	if (isSubmodeEnabled())
	{
		if (activeSubmode->onKeyUpdate(e))
			return;
	}

	int thisKey = e.key();

	// // Aux key debugging
	// if(thisKey == 0){
	//     const char* dwn = e.down() ? " Down: True" : " Down: False";
	//     Serial.println(String("Clicks: ") + String(e.clicks()) + dwn);
	// }

	// Aux double click toggle macro
	if (!isSubmodeEnabled() && midiMacroConfig.midiMacro > 0)
	{
		if (!macroActive_)
		{
			// Enter M8 Mode
			if (!e.down() && thisKey == 0 && e.clicks() == 2)
			{
				midiSettings.midiAUX = false;

				activeMacro_ = getActiveMacro();
				if (activeMacro_ != nullptr)
				{
					macroActive_ = true;
					activeMacro_->setEnabled(true);
					activeMacro_->setScale(musicScale);
					omxLeds.setDirty();
					omxDisp.setDirty();
					return;
				}
				// midiMacroConfig.m8AUX = true;
				return;
			}
		}
		else // Macro mode active
		{
			if (!e.down() && thisKey == 0 && e.clicks() == 2)
			{
				// exit macro mode
				if (activeMacro_ != nullptr)
				{
					activeMacro_->setEnabled(false);
					activeMacro_ = nullptr;
				}

				midiSettings.midiAUX = false;
				macroActive_ = false;
				omxLeds.setDirty();
				omxDisp.setDirty();

				// Clear LEDs
				for (int m = 1; m < LED_COUNT; m++)
				{
					strip.setPixelColor(m, LEDOFF);
				}
			}
			else
			{
				if (activeMacro_ != nullptr)
				{
					activeMacro_->onKeyUpdate(e);
				}
			}
			return;

			// if(activeMarco_->getEnabled() == false)
			// {
			//     macroActive_ = false;
			//     midiSettings.midiAUX = false;
			//     activeMarco_ = nullptr;

			//     // Clear LEDs
			//     for (int m = 1; m < LED_COUNT; m++)
			//     {
			//         strip.setPixelColor(m, LEDOFF);
			//     }
			//     return;
			// }
			// // Exit M8 mode
			// if (!e.down() && thisKey == 0 && e.clicks() == 2)
			// {
			//     midiMacroConfig.m8AUX = false;
			//     midiSettings.midiAUX = false;
			//     macroActive_ = true;

			//     // Clear LEDs
			//     for (int m = 1; m < LED_COUNT; m++)
			//     {
			//         strip.setPixelColor(m, LEDOFF);
			//     }
			//     return;
			// }

			// onKeyUpdateM8Macro(e);
			// return;
		}
	}

	if (onKeyUpdateSelMidiFX(e))
		return;

	// REGULAR KEY PRESSES
	if (!e.held())
	{ // IGNORE LONG PRESS EVENTS
		if (e.down() && thisKey != 0)
		{
			bool keyConsumed = false; // If used for aux, key will be consumed and not send notes.

			if (midiSettings.midiAUX) // Aux mode
			{
				keyConsumed = true;

				if (thisKey == 11 || thisKey == 12) // Change Octave
				{
					int amt = thisKey == 11 ? -1 : 1;
					midiSettings.octave = constrain(midiSettings.octave + amt, -5, 4);
				}
				else if (thisKey == 1 || thisKey == 2) // Change Param selection
				{
					if (thisKey == 1)
					{
						params.decrementParam();
					}
					else if (thisKey == 2)
					{
						params.incrementParam();
					}
					// int chng = thisKey == 1 ? -1 : 1;

					// setParam(constrain((midiPageParams.miparam + chng) % midiPageParams.numParams, 0, midiPageParams.numParams - 1));
				}
				// else if(thisKey == 5)
				// {
				//     // Turn off midiFx
				//     selectMidiFx(127, true);
				//     // mfxIndex = 127;
				// }
				// else if (thisKey >= 6 && thisKey < 11)
				// {
				//     // Change active midiFx
				//     // mfxIndex = thisKey - 6;
				//     selectMidiFx(thisKey - 6, true);
				//     // enableSubmode(&subModeMidiFx[thisKey - 6]);
				// }
				// else if(thisKey == 25)
				// {
				//     if (mfxIndex_ < NUM_MIDIFX_GROUPS)
				//     {
				//         subModeMidiFx[mfxIndex_].toggleArpHold();

				//         if (subModeMidiFx[mfxIndex_].isArpHoldOn())
				//         {
				//             omxDisp.displayMessageTimed("Arp Hold: On", 5);
				//         }
				//         else
				//         {
				//             omxDisp.displayMessageTimed("Arp Hold: Off", 5);
				//         }
				//     }
				//     else
				//     {
				//         omxDisp.displayMessageTimed("MidiFX are Off", 5);
				//     }
				// }
				// else if(thisKey == 26)
				// {
				//     if(mfxIndex_ < NUM_MIDIFX_GROUPS)
				//     {
				//         subModeMidiFx[mfxIndex_].toggleArp();

				//         if (subModeMidiFx[mfxIndex_].isArpOn())
				//         {
				//             omxDisp.displayMessageTimed("Arp On", 5);
				//         }
				//         else
				//         {
				//             omxDisp.displayMessageTimed("Arp Off", 5);
				//         }
				//     }
				//     else
				//     {
				//         omxDisp.displayMessageTimed("MidiFX are Off", 5);
				//     }
				// }
				// else if (e.down() && thisKey == 10)
				// {
				//     enableSubmode(&subModeMidiFx);
				//     keyConsumed = true;
				// }
				// else if (thisKey == 26)
				// {
				// 	keyConsumed = true;
				// }
			}

			if (!keyConsumed)
			{
				doNoteOn(thisKey);
				// omxUtil.midiNoteOn(musicScale, thisKey, midiSettings.defaultVelocity, sysSettings.midiChannel);
			}
		}
		else if (!e.down() && thisKey != 0)
		{
			doNoteOff(thisKey);
			// omxUtil.midiNoteOff(thisKey, sysSettings.midiChannel);
		}
	}
	//				Serial.println(e.clicks());

	// AUX KEY
	if (e.down() && thisKey == 0)
	{
		// Hard coded Organelle stuff
		//					MM::sendControlChange(CC_AUX, 100, sysSettings.midiChannel);

		// if (!midiMacroConfig.m8AUX)
		// {
		//     midiSettings.midiAUX = true;
		// }

		if (!macroActive_)
		{
			midiSettings.midiAUX = true;
		}

		//					if (midiAUX) {
		//						// STOP CLOCK
		//						Serial.println("stop clock");
		//					} else {
		//						// START CLOCK
		//						Serial.println("start clock");
		//					}
		//					midiAUX = !midiAUX;
	}
	else if (!e.down() && thisKey == 0)
	{
		// Hard coded Organelle stuff
		//					MM::sendControlChange(CC_AUX, 0, sysSettings.midiChannel);
		if (midiSettings.midiAUX)
		{
			midiSettings.midiAUX = false;
		}
		// turn off leds
		strip.setPixelColor(0, LEDOFF);
		strip.setPixelColor(1, LEDOFF);
		strip.setPixelColor(2, LEDOFF);
		strip.setPixelColor(11, LEDOFF);
		strip.setPixelColor(12, LEDOFF);
	}

	omxLeds.setDirty();
	omxDisp.setDirty();
}

bool OmxModeMidiKeyboard::onKeyUpdateSelMidiFX(OMXKeypadEvent e)
{
	int thisKey = e.key();

	bool keyConsumed = false;

	if (!e.held())
	{
		if (!e.down() && e.clicks() == 2 && thisKey >= 6 && thisKey < 11)
		{
			if (midiSettings.midiAUX) // Aux mode
			{
				enableSubmode(&subModeMidiFx[thisKey - 6]);
				keyConsumed = true;
			}
		}

		if (e.down() && thisKey != 0)
		{
			if (midiSettings.midiAUX) // Aux mode
			{
				if (thisKey == 5)
				{
					keyConsumed = true;
					// Turn off midiFx
					selectMidiFx(127, true);
					// mfxIndex_ = 127;
				}
				else if (thisKey >= 6 && thisKey < 11)
				{
					keyConsumed = true;
					selectMidiFx(thisKey - 6, true);
					// Change active midiFx
					// mfxIndex_ = thisKey - 6;
				}
				else if (thisKey == 22) // Goto arp params
				{
					keyConsumed = true;
					if (mfxIndex_ < NUM_MIDIFX_GROUPS)
					{
						enableSubmode(&subModeMidiFx[mfxIndex_]);
						subModeMidiFx[mfxIndex_].gotoArpParams();
						midiSettings.midiAUX = false;
					}
					else
					{
						omxDisp.displayMessage(mfxOffMsg);
					}
				}
				else if (thisKey == 23) // Next arp pattern
				{
					keyConsumed = true;
					if (mfxIndex_ < NUM_MIDIFX_GROUPS)
					{
						subModeMidiFx[mfxIndex_].nextArpPattern();
					}
					else
					{
						omxDisp.displayMessage(mfxOffMsg);
					}
				}
				else if (thisKey == 24) // Next arp octave
				{
					keyConsumed = true;
					if (mfxIndex_ < NUM_MIDIFX_GROUPS)
					{
						subModeMidiFx[mfxIndex_].nextArpOctRange();
					}
					else
					{
						omxDisp.displayMessage(mfxOffMsg);
					}
				}
				else if (thisKey == 25)
				{
					keyConsumed = true;
					if (mfxIndex_ < NUM_MIDIFX_GROUPS)
					{
						subModeMidiFx[mfxIndex_].toggleArpHold();

						if (subModeMidiFx[mfxIndex_].isArpHoldOn())
						{
							omxDisp.displayMessageTimed("Arp Hold: On", 5);
						}
						else
						{
							omxDisp.displayMessageTimed("Arp Hold: Off", 5);
						}
					}
					else
					{
						omxDisp.displayMessage(mfxOffMsg);
					}
				}
				else if (thisKey == 26)
				{
					keyConsumed = true;
					if (mfxIndex_ < NUM_MIDIFX_GROUPS)
					{
						subModeMidiFx[mfxIndex_].toggleArp();

						if (subModeMidiFx[mfxIndex_].isArpOn())
						{
							omxDisp.displayMessageTimed("Arp On", 5);
						}
						else
						{
							omxDisp.displayMessageTimed("Arp Off", 5);
						}
					}
					else
					{
						omxDisp.displayMessage(mfxOffMsg);
					}
				}
			}
		}
	}

	return keyConsumed;
}

bool OmxModeMidiKeyboard::onKeyHeldSelMidiFX(OMXKeypadEvent e)
{
	int thisKey = e.key();

	bool keyConsumed = false;

	if (midiSettings.midiAUX) // Aux mode
	{
		// Enter MidiFX mode
		if (thisKey >= 6 && thisKey < 11)
		{
			keyConsumed = true;
			enableSubmode(&subModeMidiFx[thisKey - 6]);
		}
	}

	return keyConsumed;
}

void OmxModeMidiKeyboard::onKeyHeldUpdate(OMXKeypadEvent e)
{
	if (isSubmodeEnabled())
	{
		activeSubmode->onKeyHeldUpdate(e);
		return;
	}

	if (onKeyHeldSelMidiFX(e))
		return;

	// int thisKey = e.key();

	// if (midiSettings.midiAUX) // Aux mode
	// {
	//     // Enter MidiFX mode
	//     if (thisKey >= 6 && thisKey < 11)
	//     {
	//         enableSubmode(&subModeMidiFx[thisKey - 6]);
	//     }
	// }
}

midimacro::MidiMacroInterface *OmxModeMidiKeyboard::getActiveMacro()
{
	switch (midiMacroConfig.midiMacro)
	{
	case 1:
		return &m8Macro_;
	case 2:
		return &nornsMarco_;
	}
	return nullptr;
}

// void OmxModeMidiKeyboard::onKeyUpdateM8Macro(OMXKeypadEvent e)
// {
//     if (!macroActive_)
//         return;
//     // if (!midiMacroConfig.m8AUX)
//     //     return;

//     auto activeMacro = getActiveMacro();
//     if(activeMacro == nullptr) return;

//     activeMacro->onKeyUpdate(e);
// }

void OmxModeMidiKeyboard::updateLEDs()
{
	if (isSubmodeEnabled())
	{
		if (activeSubmode->updateLEDs())
			return;
	}

	if (midiSettings.midiAUX)
	{
		bool blinkState = omxLeds.getBlinkState();

		// Blink left/right keys for octave select indicators.
		auto color1 = LIME;
		auto color2 = MAGENTA;

		for (int q = 1; q < LED_COUNT; q++)
		{
			if (midiSettings.midiKeyState[q] == -1)
			{
				if (colorConfig.midiBg_Hue == 0)
				{
					strip.setPixelColor(q, LEDOFF);
				}
				else if (colorConfig.midiBg_Hue == 32)
				{
					strip.setPixelColor(q, LOWWHITE);
				}
				else
				{
					strip.setPixelColor(q, strip.ColorHSV(colorConfig.midiBg_Hue, colorConfig.midiBg_Sat, colorConfig.midiBg_Brightness));
				}
			}
		}
		strip.setPixelColor(0, RED);
		strip.setPixelColor(1, color1);
		strip.setPixelColor(2, color2);

		auto octDnColor = ORANGE;
		auto octUpColor = RBLUE;

		if (midiSettings.octave == 0)
		{
			strip.setPixelColor(11, octDnColor);
			strip.setPixelColor(12, octUpColor);
		}
		else if (midiSettings.octave > 0)
		{
			bool blinkOctave = omxLeds.getBlinkPattern(midiSettings.octave);

			strip.setPixelColor(11, octDnColor);
			strip.setPixelColor(12, blinkOctave ? octUpColor : LEDOFF);
		}
		else
		{
			bool blinkOctave = omxLeds.getBlinkPattern(-midiSettings.octave);

			strip.setPixelColor(11, blinkOctave ? octDnColor : LEDOFF);
			strip.setPixelColor(12, octUpColor);
		}

		// MidiFX off
		strip.setPixelColor(5, (mfxIndex_ >= NUM_MIDIFX_GROUPS ? colorConfig.selMidiFXGRPOffColor : colorConfig.midiFXGRPOffColor));

		for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
		{
			auto mfxColor = (i == mfxIndex_) ? colorConfig.selMidiFXGRPColor : colorConfig.midiFXGRPColor;

			strip.setPixelColor(6 + i, mfxColor);
		}

		strip.setPixelColor(22, colorConfig.gotoArpParams);
		strip.setPixelColor(23, colorConfig.nextArpPattern);

		if (mfxIndex_ < NUM_MIDIFX_GROUPS)
		{
			uint8_t octaveRange = subModeMidiFx[mfxIndex_].getArpOctaveRange();
			if (octaveRange == 0)
			{
				strip.setPixelColor(24, colorConfig.nextArpOctave);
			}
			else
			{
				// Serial.println("Blink Octave: " + String(octaveRange));
				bool blinkOctave = omxLeds.getBlinkPattern(octaveRange);

				strip.setPixelColor(24, blinkOctave ? colorConfig.nextArpOctave : LEDOFF);
			}

			bool isOn = subModeMidiFx[mfxIndex_].isArpOn() && blinkState;
			bool isHoldOn = subModeMidiFx[mfxIndex_].isArpHoldOn();

			strip.setPixelColor(25, isHoldOn ? colorConfig.arpHoldOn : colorConfig.arpHoldOff);
			strip.setPixelColor(26, isOn ? colorConfig.arpOn : colorConfig.arpOff);
		}
		else
		{
			strip.setPixelColor(25, colorConfig.arpHoldOff);
			strip.setPixelColor(26, colorConfig.arpOff);
		}

		// strip.setPixelColor(10, color3); // MidiFX key

		// Macros
	}
	else
	{
		omxLeds.drawMidiLeds(musicScale); // SHOW LEDS
	}

	if (isSubmodeEnabled())
	{
		bool blinkStateSlow = omxLeds.getSlowBlinkState();

		auto auxColor = (blinkStateSlow ? RED : LEDOFF);
		strip.setPixelColor(0, auxColor);
	}
}

void OmxModeMidiKeyboard::onDisplayUpdate()
{
	// omxLeds.updateBlinkStates();

	if (isSubmodeEnabled())
	{
		if (omxLeds.isDirty())
		{
			updateLEDs();
		}
		activeSubmode->onDisplayUpdate();
		return;
	}

	bool macroConsumesDisplay = false;

	if (macroActive_ && activeMacro_ != nullptr)
	{
		activeMacro_->drawLEDs();
		macroConsumesDisplay = activeMacro_->consumesDisplay();
	}
	else
	{
		if (omxLeds.isDirty())
		{
			updateLEDs();
		}
		// if (omxLeds.isDirty())
		// {
		//     updateLEDs();
		//     // omxLeds.drawMidiLeds(musicScale); // SHOW LEDS
		// }
	}

	if (macroConsumesDisplay)
	{
		activeMacro_->onDisplayUpdate();
	}
	else
	{
		if (omxDisp.isDirty())
		{ // DISPLAY
			if (!encoderConfig.enc_edit)
			{
				if (params.getSelPage() == MIPAGE_OUTMIDI)
				{
					omxDisp.clearLegends();

					//			if (midiRoundRobin) {
					//				displaychan = rrChannel;
					//			}
					omxDisp.legends[0] = "OCT";
					omxDisp.legends[1] = "CH";
					omxDisp.legends[2] = "VEL";
					omxDisp.legends[3] = "";
					omxDisp.legendVals[0] = (int)midiSettings.octave + 4;
					omxDisp.legendVals[1] = sysSettings.midiChannel;
					omxDisp.legendVals[2] = midiSettings.defaultVelocity;
					omxDisp.legendVals[3] = -127;
					omxDisp.legendText[3] = "";
				}
				else if (params.getSelPage() == MIPAGE_MIDIINSPECT)
				{
					omxDisp.clearLegends();

					//			if (midiRoundRobin) {
					//				displaychan = rrChannel;
					//			}
					omxDisp.legends[0] = "P CC";
					omxDisp.legends[1] = "P VAL";
					omxDisp.legends[2] = "NOTE";
					omxDisp.legends[3] = "VEL";
					omxDisp.legendVals[0] = potSettings.potCC;
					omxDisp.legendVals[1] = potSettings.potVal;
					omxDisp.legendVals[2] = midiSettings.midiLastNote;
					omxDisp.legendVals[3] = midiSettings.midiLastVel;
				}
				else if (params.getSelPage() == MIPAGE_OUTCC)
				{
					omxDisp.clearLegends();

					omxDisp.legends[0] = "RR";
					omxDisp.legends[1] = "RROF";
					omxDisp.legends[2] = "PGM";
					omxDisp.legends[3] = "BNK";
					omxDisp.legendVals[0] = midiSettings.midiRRChannelCount;
					omxDisp.legendVals[1] = midiSettings.midiRRChannelOffset;
					omxDisp.legendVals[2] = midiSettings.currpgm + 1;
					omxDisp.legendVals[3] = midiSettings.currbank;
				}
				else if (params.getSelPage() == MIPAGE_POTSANDMACROS) // SUBMODE_MIDI3
				{
					omxDisp.clearLegends();

					omxDisp.legends[0] = "PBNK"; // Potentiometer Banks
					omxDisp.legends[1] = "THRU"; // MIDI thru (usb to hardware)
					omxDisp.legends[2] = "MCRO"; // Macro mode
					omxDisp.legends[3] = "M-CH";
					omxDisp.legendVals[0] = potSettings.potbank + 1;
					omxDisp.legendVals[1] = -127;
					if (midiSettings.midiSoftThru)
					{
						omxDisp.legendText[1] = "On";
					}
					else
					{
						omxDisp.legendText[1] = "Off";
					}
					omxDisp.legendVals[2] = -127;
					omxDisp.legendText[2] = macromodes[midiMacroConfig.midiMacro];
					omxDisp.legendVals[3] = midiMacroConfig.midiMacroChan;
				}
				else if (params.getSelPage() == MIPAGE_SCALES) // SCALES
				{
					omxDisp.clearLegends();
					omxDisp.legends[0] = "ROOT";
					omxDisp.legends[1] = "SCALE";
					omxDisp.legends[2] = "LOCK";
					omxDisp.legends[3] = "GROUP";
					omxDisp.legendVals[0] = -127;
					if (scaleConfig.scalePattern < 0)
					{
						omxDisp.legendVals[1] = -127;
						omxDisp.legendText[1] = "Off";
					}
					else
					{
						omxDisp.legendVals[1] = scaleConfig.scalePattern;
					}

					omxDisp.legendVals[2] = -127;
					omxDisp.legendVals[3] = -127;

					omxDisp.legendText[0] = musicScale->getNoteName(scaleConfig.scaleRoot);
					omxDisp.legendText[2] = scaleConfig.lockScale ? "On" : "Off";
					omxDisp.legendText[3] = scaleConfig.group16 ? "On" : "Off";
				}
				else if (params.getSelPage() == MIPAGE_CFG) // CONFIG
				{
					omxDisp.clearLegends();
					omxDisp.legends[0] = "CC";
					omxDisp.legends[1] = "";
					omxDisp.legends[2] = "";
					omxDisp.legends[3] = "";
					omxDisp.legendVals[0] = -127;
					omxDisp.legendVals[1] = -127;
					omxDisp.legendVals[2] = -127;
					omxDisp.legendVals[3] = -127;
					omxDisp.legendText[0] = "CFG";
					omxDisp.legendText[1] = "";
					omxDisp.legendText[2] = "";
					omxDisp.legendText[3] = "";
				}

				omxDisp.dispGenericMode2(params.getNumPages(), params.getSelPage(), params.getSelParam(), encoderSelect && !midiSettings.midiAUX);
			}
		}
	}
}

// incoming midi note on
void OmxModeMidiKeyboard::inMidiNoteOn(byte channel, byte note, byte velocity)
{
	if (organelleMotherMode)
		return;

	midiSettings.midiLastNote = note;
	midiSettings.midiLastVel = velocity;
	int whatoct = (note / 12);
	int thisKey;
	uint32_t keyColor = MIDINOTEON;

	if ((whatoct % 2) == 0)
	{
		thisKey = note - (12 * whatoct);
	}
	else
	{
		thisKey = note - (12 * whatoct) + 12;
	}
	if (whatoct == 0)
	{ // ORANGE,YELLOW,GREEN,MAGENTA,CYAN,BLUE,LIME,LTPURPLE
	}
	else if (whatoct == 1)
	{
		keyColor = ORANGE;
	}
	else if (whatoct == 2)
	{
		keyColor = YELLOW;
	}
	else if (whatoct == 3)
	{
		keyColor = GREEN;
	}
	else if (whatoct == 4)
	{
		keyColor = MAGENTA;
	}
	else if (whatoct == 5)
	{
		keyColor = CYAN;
	}
	else if (whatoct == 6)
	{
		keyColor = LIME;
	}
	else if (whatoct == 7)
	{
		keyColor = CYAN;
	}
	strip.setPixelColor(midiKeyMap[thisKey], keyColor); //  Set pixel's color (in RAM)
														//	dirtyPixels = true;
	strip.show();
	omxDisp.setDirty();
}

void OmxModeMidiKeyboard::inMidiNoteOff(byte channel, byte note, byte velocity)
{
	if (organelleMotherMode)
		return;

	int whatoct = (note / 12);
	int thisKey;
	if ((whatoct % 2) == 0)
	{
		thisKey = note - (12 * whatoct);
	}
	else
	{
		thisKey = note - (12 * whatoct) + 12;
	}
	strip.setPixelColor(midiKeyMap[thisKey], LEDOFF); //  Set pixel's color (in RAM)
													  //	dirtyPixels = true;
	strip.show();
	omxDisp.setDirty();
}

void OmxModeMidiKeyboard::SetScale(MusicScales *scale)
{
	this->musicScale = scale;
	m8Macro_.setScale(scale);
	nornsMarco_.setScale(scale);
}

void OmxModeMidiKeyboard::enableSubmode(SubmodeInterface *subMode)
{
	if (activeSubmode != nullptr)
	{
		activeSubmode->setEnabled(false);
	}

	activeSubmode = subMode;
	activeSubmode->setEnabled(true);
	omxDisp.setDirty();
}

void OmxModeMidiKeyboard::disableSubmode()
{
	if (activeSubmode != nullptr)
	{
		activeSubmode->setEnabled(false);
	}

	midiSettings.midiAUX = false;

	activeSubmode = nullptr;
	omxDisp.setDirty();
}

bool OmxModeMidiKeyboard::isSubmodeEnabled()
{
	if (activeSubmode == nullptr)
		return false;

	if (activeSubmode->isEnabled() == false)
	{
		disableSubmode();
		midiSettings.midiAUX = false;
		return false;
	}

	return true;
}

void OmxModeMidiKeyboard::doNoteOn(uint8_t keyIndex)
{
	MidiNoteGroup noteGroup = omxUtil.midiNoteOn2(musicScale, keyIndex, midiSettings.defaultVelocity, sysSettings.midiChannel);

	if (noteGroup.noteNumber == 255)
		return;

	// Serial.println("doNoteOn: " + String(noteGroup.noteNumber));

	noteGroup.unknownLength = true;
	noteGroup.prevNoteNumber = noteGroup.noteNumber;

	if (mfxIndex_ < NUM_MIDIFX_GROUPS)
	{
		subModeMidiFx[mfxIndex_].noteInput(noteGroup);
		// subModeMidiFx.noteInput(noteGroup);
	}
	else
	{
		onNotePostFX(noteGroup);
	}
}
void OmxModeMidiKeyboard::doNoteOff(uint8_t keyIndex)
{
	MidiNoteGroup noteGroup = omxUtil.midiNoteOff2(keyIndex, sysSettings.midiChannel);

	if (noteGroup.noteNumber == 255)
		return;

	// Serial.println("doNoteOff: " + String(noteGroup.noteNumber));

	noteGroup.unknownLength = true;
	noteGroup.prevNoteNumber = noteGroup.noteNumber;

	if (mfxIndex_ < NUM_MIDIFX_GROUPS)
	{
		subModeMidiFx[mfxIndex_].noteInput(noteGroup);
		// subModeMidiFx.noteInput(noteGroup);
	}
	else
	{
		onNotePostFX(noteGroup);
	}
}

// // Called by a euclid sequencer when it triggers a note
// void OmxModeMidiKeyboard::onNoteTriggered(uint8_t euclidIndex, MidiNoteGroup note)
// {
//     // Serial.println("OmxModeEuclidean::onNoteTriggered " + String(euclidIndex) + " note: " + String(note.noteNumber));

//     subModeMidiFx.noteInput(note);

//     omxDisp.setDirty();
// }

// Called by the midiFX group when a note exits it's FX Pedalboard
void OmxModeMidiKeyboard::onNotePostFX(MidiNoteGroup note)
{
	if (note.noteOff)
	{
		// Serial.println("OmxModeMidiKeyboard::onNotePostFX noteOff: " + String(note.noteNumber));

		if (note.sendMidi)
		{
			MM::sendNoteOff(note.noteNumber, note.velocity, note.channel);
		}
		if (note.sendCV)
		{
			omxUtil.cvNoteOff();
		}
	}
	else
	{
		if (note.unknownLength == false)
		{
			uint32_t noteOnMicros = note.noteonMicros; // TODO Might need to be set to current micros
			pendingNoteOns.insert(note.noteNumber, note.velocity, note.channel, noteOnMicros, note.sendCV);

			// Serial.println("StepLength: " + String(note.stepLength));

			uint32_t noteOffMicros = noteOnMicros + (note.stepLength * clockConfig.step_micros);
			pendingNoteOffs.insert(note.noteNumber, note.channel, noteOffMicros, note.sendCV);

			// Serial.println("noteOnMicros: " + String(noteOnMicros));
			// Serial.println("noteOffMicros: " + String(noteOffMicros));
		}
		else
		{
			// Serial.println("OmxModeMidiKeyboard::onNotePostFX noteOn: " + String(note.noteNumber));

			if (note.sendMidi)
			{
				MM::sendNoteOn(note.noteNumber, note.velocity, note.channel);
			}
			if (note.sendCV)
			{
				omxUtil.cvNoteOn(note.noteNumber);
			}
		}
	}

	// uint32_t noteOnMicros = note.noteonMicros; // TODO Might need to be set to current micros
	// pendingNoteOns.insert(note.noteNumber, note.velocity, note.channel, noteOnMicros, note.sendCV);

	// uint32_t noteOffMicros = noteOnMicros + (note.stepLength * clockConfig.step_micros);
	// pendingNoteOffs.insert(note.noteNumber, note.channel, noteOffMicros, note.sendCV);
}

void OmxModeMidiKeyboard::onPendingNoteOff(int note, int channel)
{
	// Serial.println("OmxModeEuclidean::onPendingNoteOff " + String(note) + " " + String(channel));
	// subModeMidiFx.onPendingNoteOff(note, channel);

	for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		subModeMidiFx[i].onPendingNoteOff(note, channel);
	}
}
