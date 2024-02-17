#include "submode_midifxgroup.h"
#include "../../hardware/omx_disp.h"
#include "../../hardware/omx_leds.h"
#include "../../consts/colors.h"
#include "../../midifx/midifx_randomizer.h"
#include "../../midifx/midifx_chance.h"
#include "../../midifx/midifx_scaler.h"
#include "../../midifx/midifx_monophonic.h"
#include "../../midifx/midifx_harmonizer.h"
#include "../../midifx/midifx_transpose.h"
// #include "midifx_arpeggiator.h"

using namespace midifx;

SubModeMidiFxGroup subModeMidiFx[NUM_MIDIFX_GROUPS];

// const int kSelMFXColor = 0xFACAE2;
// const int kMFXColor = ROSE;
// const int kMFXEmptyColor = 0x600030;

const int kSelMFXTypeColor = 0xE6FFCF;
const int kMFXTypeColor = DKGREEN;
const int kMFXTypeEmptyColor = 0x400000;

// None, Chance, Randomizer, Harmonizer = Heliotrope gray, Scaler = Spanish viridian, Monophonic = Maroon (Crayola),
// const int kMFXTypeColors[16] = {kMFXTypeEmptyColor, CYAN, RED, 0xAA98A9, 0x007F5C, 0xC32148, kMFXTypeEmptyColor, kMFXTypeEmptyColor,
// kMFXTypeEmptyColor, kMFXTypeEmptyColor, kMFXTypeEmptyColor, kMFXTypeEmptyColor, kMFXTypeEmptyColor, kMFXTypeEmptyColor, kMFXTypeEmptyColor, kMFXTypeEmptyColor};

enum MidiFxPage
{
	MFXPAGE_FX,
	MFXPAGE_FX2,
	MFXPAGE_EXIT
};

SubModeMidiFxGroup::SubModeMidiFxGroup()
{
	params_.addPage(4); // 4 Midi FX slots
	params_.addPage(4); // 4 Midi FX slots
	params_.addPage(1); // Exit submode

	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		midifx_.push_back(nullptr);
		midifxTypes_[i] = 0;
	}

	doNoteOutput_ = &SubModeMidiFxGroup::noteFuncForwarder;
	doNoteOutputContext_ = this;

	for (uint8_t i = 0; i < 32; i++)
	{
		onNoteGroups[i].prevNoteNumber = 255;
	}
}

uint8_t SubModeMidiFxGroup::getArpIndex()
{
	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		auto mfx = getMidiFX(i);
		if (mfx != nullptr)
		{
			if (mfx->getFXType() == MIDIFX_ARP)
			{
				return i;
			}
		}
	}

	return 255;
}

midifx::MidiFXArpeggiator *SubModeMidiFxGroup::getArp(bool autoCreate)
{
	bool canAddArp = false;
	uint8_t addArpIndex = 0;

	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		auto mfx = getMidiFX(i);
		if (mfx != nullptr)
		{
			mfx->setSelected(selected_);
			if (mfx->getFXType() == MIDIFX_ARP)
			{
				return static_cast<midifx::MidiFXArpeggiator *>(mfx);
			}
		}
		else // empty slot
		{
			if (!canAddArp)
			{
				canAddArp = true;
				addArpIndex = i;
			}
		}
	}

	// If we are here, no arp was found

	if (autoCreate && canAddArp)
	{
		changeMidiFXType(addArpIndex, MIDIFX_ARP, true);
		return getArp(false);
	}

	return nullptr;
}

void SubModeMidiFxGroup::toggleArp()
{
	auto arp = getArp(true);
	if (arp != nullptr)
	{
		arp->toggleArp();
	}
}

void SubModeMidiFxGroup::toggleArpHold()
{
	auto arp = getArp(true);
	if (arp != nullptr)
	{
		arp->toggleHold();
	}
}

bool SubModeMidiFxGroup::isArpOn()
{
	auto arp = getArp(false);
	if (arp != nullptr)
	{
		return arp->isOn();
	}
	return false;
}

bool SubModeMidiFxGroup::isArpHoldOn()
{
	auto arp = getArp(false);
	if (arp != nullptr)
	{
		return arp->isHoldOn();
	}
	return false;
}

void SubModeMidiFxGroup::nextArpPattern()
{
	auto arp = getArp(true);

	if (arp != nullptr)
	{
		arp->nextArpPattern();
	}
}

void SubModeMidiFxGroup::nextArpOctRange()
{
	auto arp = getArp(true);

	if (arp != nullptr)
	{
		arp->nextOctRange();
	}
}

void SubModeMidiFxGroup::gotoArpParams()
{
	midiFXParamView_ = true;
	arpParamView_ = true;
	heldMidiFX_ = -1;

	getArp(true); // Create arp if empty

	uint8_t arpIndex = getArpIndex();

	if (arpIndex < NUM_MIDIFX_SLOTS)
	{
		selectedMidiFX_ = arpIndex;
	}

	omxDisp.displayMessage(mfxArpEditMsg);
}

uint8_t SubModeMidiFxGroup::getArpOctaveRange()
{
	auto arp = getArp(false);

	if (arp != nullptr)
	{
		return arp->getOctaveRange() + 1;
	}

	return 0;
}

void SubModeMidiFxGroup::onModeChanged()
{
	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		auto mfx = getMidiFX(i);
		if (mfx != nullptr)
		{
			mfx->onModeChanged();
		}
	}
}

void SubModeMidiFxGroup::setSelected(bool newSelected)
{
	selected_ = newSelected;

	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		auto mfx = getMidiFX(i);
		if (mfx != nullptr)
		{
			mfx->setSelected(newSelected);
		}
	}
}

void SubModeMidiFxGroup::setAuxDown(bool auxDown)
{
	auxDown_ = auxDown;

	for (uint8_t i = 0; i < midifx_.size(); i++)
	{
		if (midifx_[i] != nullptr)
		{
			midifx_[i]->setAuxDown(auxDown_);
		}
	}
}

void SubModeMidiFxGroup::onEnabled()
{
	// params_.setSelPageAndParam(0, 0);
	midiFXParamView_ = true;

	// Goto first available midifx if selected one is empty.
	auto mfx = getMidiFX(selectedMidiFX_);
	if (mfx == nullptr)
	{
		for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
		{
			auto mfx = getMidiFX(i);
			if (mfx != nullptr)
			{
				selectedMidiFX_ = i;
				break;
			}
		}
	}

	heldMidiFX_ = -1;

	encoderSelect_ = true;
	omxLeds.setDirty();
	omxDisp.setDirty();

	auxReleased_ = !midiSettings.keyState[0];
	setAuxDown(false);

	// for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	// {
	//     auto mfx = getMidiFX(i);
	//     if (mfx != nullptr)
	//     {
	//         mfx->setEnabled(true);
	//     }
	// }
}

void SubModeMidiFxGroup::onDisabled()
{
	strip.clear();
	omxLeds.setDirty();
	omxDisp.setDirty();

	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		auto mfx = getMidiFX(i);
		if (mfx != nullptr)
		{
			mfx->setEnabled(false);
		}
	}
}

void SubModeMidiFxGroup::updateFuncKeyMode()
{
	auto keyState = midiSettings.keyState;

	uint8_t prevMode = funcKeyMode_;

	funcKeyMode_ = FUNCKEYMODE_NONE;

	if (keyState[1] && !keyState[2])
	{
		funcKeyMode_ = FUNCKEYMODE_F1;
	}
	else if (!keyState[1] && keyState[2])
	{
		funcKeyMode_ = FUNCKEYMODE_F2;
	}
	else if (keyState[1] && keyState[2])
	{
		funcKeyMode_ = FUNCKEYMODE_F3;
	}
	else
	{
		funcKeyMode_ = FUNCKEYMODE_NONE;
	}

	if (funcKeyMode_ != prevMode)
	{
		omxDisp.setDirty();
		omxLeds.setDirty();
	}
}

void SubModeMidiFxGroup::resync()
{
	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		auto mfx = getMidiFX(i);
		if (mfx != nullptr)
		{
			mfx->resync();
		}
	}
}

void SubModeMidiFxGroup::onClockTick()
{
	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		auto mfx = getMidiFX(i);
		if (mfx != nullptr)
		{
			mfx->onClockTick();
		}
	}
}

void SubModeMidiFxGroup::loopUpdate()
{
	if (enabled_)
	{
		updateFuncKeyMode();
	}

	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		auto mfx = getMidiFX(i);
		if (mfx != nullptr)
		{
			mfx->loopUpdate();
		}
	}

	// Animation
	if (heldMidiFX_ >= 0 && heldAnimPos_ < 100)
	{
		if ((micros() - prevAnimTime_) > (1000 * 10))
		{
			heldAnimPos_ += 1;
			prevAnimTime_ = micros();
			omxDisp.setDirty();
		}
	}
}

bool SubModeMidiFxGroup::updateLEDs()
{
	strip.clear();

	bool blinkState = omxLeds.getBlinkState();
	bool blinkStateSlow = omxLeds.getSlowBlinkState();

	// Serial.println("MidiFX Leds");
	auto auxColor = midiFXParamView_ ? (blinkStateSlow ? ORANGE : LEDOFF) : RED;
	strip.setPixelColor(0, auxColor);

	if (arpParamView_)
		return false;

	// for(uint8_t i = 1; i < 26; i++)
	// {
	//     strip.setPixelColor(i, LEDOFF);
	// }

	if (midiFXParamView_)
	{
		auto mfx = getMidiFX(selectedMidiFX_);
		if (mfx != nullptr && mfx->usesKeys())
		{
			mfx->updateLEDs(funcKeyMode_);
			return true;
		}
	}

	// Function Keys
	if (funcKeyMode_ == FUNCKEYMODE_F3)
	{
		auto f3Color = blinkState ? LEDOFF : FUNKTHREE;
		strip.setPixelColor(1, f3Color);
		strip.setPixelColor(2, f3Color);
	}
	else
	{
		auto f1Color = (funcKeyMode_ == FUNCKEYMODE_F1 && blinkState) ? LEDOFF : FUNKONE;
		strip.setPixelColor(1, f1Color);

		auto f2Color = (funcKeyMode_ == FUNCKEYMODE_F2 && blinkState) ? LEDOFF : FUNKTWO;
		strip.setPixelColor(2, f2Color);
	}

	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		// auto fxColor = midiFXParamView_ ? (i == selectedMidiFX_ ? WHITE : ORANGE) : BLUE;

		auto fxColor = getMidiFX(i) == nullptr ? colorConfig.midiFXEmptyColor : getMidiFX(i)->getColor();

		if (i == selectedMidiFX_)
		{
			fxColor = blinkState ? fxColor : LEDOFF;
		}

		// auto fxColor = (i == selectedMidiFX_ ? kSelMFXColor : (getMidiFX(i) == nullptr ? kMFXEmptyColor : kMFXColor));

		strip.setPixelColor(3 + i, fxColor);
	}

	// Change midifx while holding down midifx slot
	if (heldMidiFX_ >= 0 && midiFXParamView_ && !arpParamView_)
	{
		uint8_t selFXType = 0;

		if (getMidiFX(selectedMidiFX_) != nullptr)
		{
			// Serial.println("Selected MidiFX not null");
			selFXType = getMidiFX(selectedMidiFX_)->getFXType();
		}

		for (uint8_t i = 0; i < 16; i++)
		{
			auto fxColor = (i == selFXType ? kSelMFXTypeColor : ((i == MIDIFX_NONE || i >= MIDIFX_COUNT) ? kMFXTypeEmptyColor : kMFXTypeColor));

			strip.setPixelColor(11 + i, fxColor);
		}
	}

	return true;
}

void SubModeMidiFxGroup::moveSelectedMidiFX(int8_t direction)
{
	if (direction == 0)
		return;

	uint8_t newIndex = (selectedMidiFX_ + direction + NUM_MIDIFX_SLOTS) % NUM_MIDIFX_SLOTS;

	auto selMFX = getMidiFX(selectedMidiFX_);

	tempMidiFX_.clear();

	for (uint8_t i = 0; i < midifx_.size(); i++)
	{
		if (i != selectedMidiFX_)
		{
			tempMidiFX_.push_back(midifx_[i]);
		}
	}

	tempMidiFX_.insert(tempMidiFX_.begin() + newIndex, selMFX);

	midifx_.clear();

	for (uint8_t i = 0; i < tempMidiFX_.size(); i++)
	{
		midifx_.push_back(tempMidiFX_[i]);
	}

	tempMidiFX_.clear();

	if (midifx_.size() != NUM_MIDIFX_SLOTS)
	{
		Serial.println("ERROR: MidiFX size changed");
	}

	selectedMidiFX_ = newIndex;
	reconnectInputsOutputs();
}

void SubModeMidiFxGroup::onEncoderChanged(Encoder::Update enc)
{
	if (midiFXParamView_)
	{
		if (heldMidiFX_ >= 0)
		{
			auto amt = constrain(enc.accel(1), -1, 1);

			moveSelectedMidiFX(amt);
			omxDisp.setDirty();
			omxLeds.setDirty();
			return;
		}

		if (getMidiFX(selectedMidiFX_) != nullptr)
		{
			getMidiFX(selectedMidiFX_)->onEncoderChanged(enc);
		}
	}
	else
	{
		SubmodeInterface::onEncoderChanged(enc);
	}
}

void SubModeMidiFxGroup::onEncoderChangedEditParam(Encoder::Update enc)
{
	omxDisp.setDirty();
	omxLeds.setDirty();
}

void SubModeMidiFxGroup::onEncoderButtonDown()
{
	if (midiFXParamView_)
	{
		if (getMidiFX(selectedMidiFX_) != nullptr)
		{
			getMidiFX(selectedMidiFX_)->onEncoderButtonDown();
		}
	}
	else
	{
		if (params_.getSelPage() == MFXPAGE_FX)
		{
			selectMidiFX(params_.getSelParam());
		}
		else if (params_.getSelPage() == MFXPAGE_FX2)
		{
			selectMidiFX(params_.getSelParam() + 4);
		}
		else if (params_.getSelPage() == MFXPAGE_EXIT && params_.getSelParam() == 0)
		{
			setEnabled(false);
		}
		else
		{
			SubmodeInterface::onEncoderButtonDown();
		}
	}
	omxDisp.setDirty();
	omxLeds.setDirty();
}

bool SubModeMidiFxGroup::onKeyUpdate(OMXKeypadEvent e)
{
	if (e.held())
	{
		// if(arpParamView_) return false; // Don't consume key update
		return true;
	}

	int thisKey = e.key();

	// Aux logic
	if (thisKey == 0)
	{
		omxDisp.setDirty();
		omxLeds.setDirty();

		if (!auxReleased_)
		{
			if (!e.down())
			{
				// Used to prevent quickly exiting if entered through aux shortcut.
				auxReleased_ = true;
			}
		}
		else
		{
			if (e.down())
			{
				setAuxDown(true);
			}
			else
			{
				setAuxDown(false);
			}

			// exit
			// if(!e.down() && e.clicks() == 2)
			if (e.quickClicked())
			{
				arpParamView_ = false;
				midiFXParamView_ = false;
				setEnabled(false);
				omxDisp.displayMessage(exitMsg);
				return true;
			}
		}

		if (arpParamView_)
			return false; // Don't consume key update
		return true;	  // Consume key
	}

	// auto keyState = midiSettings.keyState;

	bool mfxKeysActive = false;

	auto mfx = getMidiFX(selectedMidiFX_);
	if (midiFXParamView_ && mfx != nullptr && mfx->usesKeys())
	{
		mfxKeysActive = true;
	}

	if (e.down())
	{
		// if (thisKey == 0)
		// {
		//     if(arpParamView_)
		//     {
		//         arpParamView_ = false;
		//         midiFXParamView_ = false;
		//         setEnabled(false);
		//         return true;
		//     }
		//     // // Exit MidiFX view
		//     // if (midiFXParamView_)
		//     // {
		//     //     midiFXParamView_ = false;
		//     //     encoderSelect_ = true;
		//     // }
		//     // // Exit submode
		//     // else

		//     if (auxReleased_)
		//     {
		//         setEnabled(false);
		//         return true;
		//     }
		// }

		if (arpParamView_)
		{
			return false;
		}

		if (mfxKeysActive == false)
		{
			// Quick Select FX Slot
			if (thisKey >= 3 && thisKey < 3 + NUM_MIDIFX_SLOTS)
			{
				if (funcKeyMode_ == FUNCKEYMODE_NONE)
				{
					heldMidiFX_ = thisKey - 3;
					heldAnimPos_ = 0;
					prevAnimTime_ = micros();
					selectMidiFX(thisKey - 3);
				}
				else if (funcKeyMode_ == FUNCKEYMODE_F1)
				{
					// Copy
					copyMidiFX(thisKey - 3);
					omxDisp.displayMessage("Copy");
				}
				else if (funcKeyMode_ == FUNCKEYMODE_F2)
				{
					// Paste
					pasteMidiFX(thisKey - 3);
					omxDisp.displayMessage("Paste");
				}
				else if (funcKeyMode_ == FUNCKEYMODE_F3)
				{
					// Cut
					cutMidiFX(thisKey - 3);
					omxDisp.displayMessage("Cut");
				}
			}

			// Change FX type
			if (heldMidiFX_ >= 0 && midiFXParamView_ && !arpParamView_)
			{
				if (thisKey >= 11 && thisKey < 11 + 16)
				{
					changeMidiFXType(selectedMidiFX_, thisKey - 11);
					// selectMidiFX(thisKey - 19);
				}
			}

			if (funcKeyMode_ == FUNCKEYMODE_NONE)
			{
			}
		}
	}

	// if(!e.down() && thisKey == 0)
	// {
	//     // Used to prevent quickly exiting if entered through aux shortcut.
	//     auxReleased_ = true;
	// }

	// release held midiFX whenever a midifx key is released.
	if (!e.down() && thisKey >= 3 && thisKey < 3 + NUM_MIDIFX_SLOTS)
	{
		heldMidiFX_ = -1;
	}

	if (arpParamView_)
	{
		return false;
	}

	if (mfxKeysActive)
	{
		mfx->onKeyUpdate(e, funcKeyMode_);
	}

	omxDisp.setDirty();
	omxLeds.setDirty();

	return true;
}

void SubModeMidiFxGroup::selectMidiFX(uint8_t fxIndex)
{
	uint8_t prevSelMFX = selectedMidiFX_;
	midiFXParamView_ = true;
	selectedMidiFX_ = fxIndex;

	if (selectedMidiFX_ != prevSelMFX)
	{
		auto prevMFX = getMidiFX(prevSelMFX);
		auto newMFX = getMidiFX(selectedMidiFX_);

		if (prevMFX != nullptr)
		{
			prevMFX->setEnabled(false);
		}

		if (newMFX != nullptr)
		{
			newMFX->setEnabled(true);
		}

		arpParamView_ = false;
	}

	// displayMidiFXName(fxIndex);
}

void SubModeMidiFxGroup::copyMidiFX(uint8_t fxIndex)
{
	if (copyBuffer != nullptr)
	{
		delete copyBuffer;
		copyBuffer = nullptr;
	}
	auto mfx = getMidiFX(fxIndex);
	if (mfx != nullptr)
	{
		copyBuffer = mfx->getClone();
	}
}
void SubModeMidiFxGroup::cutMidiFX(uint8_t fxIndex)
{
	copyMidiFX(fxIndex);

	if (getMidiFX(fxIndex) != nullptr)
	{
		midifx::MidiFXInterface *midifxptr = midifx_[fxIndex];

		midifx_[fxIndex] = nullptr;

		delete midifxptr;
	}

	midifxTypes_[fxIndex] = MIDIFX_NONE;
	reconnectInputsOutputs();
}
void SubModeMidiFxGroup::pasteMidiFX(uint8_t fxIndex)
{
	if (getMidiFX(fxIndex) != nullptr)
	{
		midifx::MidiFXInterface *midifxptr = midifx_[fxIndex];

		midifx_[fxIndex] = nullptr;

		delete midifxptr;
	}

	if (copyBuffer != nullptr)
	{
		setMidiFX(fxIndex, copyBuffer->getClone());
	}

	if (getMidiFX(fxIndex) != nullptr)
	{
		midifxTypes_[fxIndex] = getMidiFX(fxIndex)->getFXType();
	}
	else
	{
		midifxTypes_[fxIndex] = MIDIFX_NONE;
	}

	reconnectInputsOutputs();
}

void SubModeMidiFxGroup::displayMidiFXName(uint8_t index)
{
	auto mfx = getMidiFX(index);

	if (mfx != nullptr)
	{
		omxDisp.displayMessage(mfx->getName());
	}
	else
	{
		omxDisp.displayMessage("None");
	}
}

const char *SubModeMidiFxGroup::getMFXDispName(uint8_t index)
{
	auto mfx = getMidiFX(index);

	if (mfx != nullptr)
	{
		return mfx->getDispName();
	}
	return "-";
}

bool SubModeMidiFxGroup::getEncoderSelect()
{
	return encoderSelect_ && !auxDown_;
}

midifx::MidiFXInterface *SubModeMidiFxGroup::getMidiFX(uint8_t index)
{
	return midifx_[index];
}

void SubModeMidiFxGroup::setMidiFX(uint8_t index, midifx::MidiFXInterface *midifx)
{
	midifx_[index] = midifx;
}

void SubModeMidiFxGroup::changeMidiFXType(uint8_t slotIndex, uint8_t typeIndex, bool fromLoad)
{
	// Serial.println(String("changeMidiFXType slotIndex: ") + String(slotIndex) + " typeIndex: " + String(typeIndex));
	if (!fromLoad)
	{
		if (!midiFXParamView_)
			return;
	}

	if (typeIndex == midifxTypes_[slotIndex])
		return;

	if (getMidiFX(slotIndex) != nullptr)
	{
		// Serial.println("Deleting FX");

		midifx::MidiFXInterface *midifxptr = midifx_[slotIndex];

		midifx_[slotIndex] = nullptr;

		delete midifxptr;
	}

	switch (typeIndex)
	{
	case MIDIFX_CHANCE:
	{
		setMidiFX(slotIndex, new MidiFXChance());
	}
	break;
	case MIDIFX_TRANSPOSE:
	{
		setMidiFX(slotIndex, new MidiFXTranspose());
	}
	break;
	case MIDIFX_RANDOMIZER:
	{
		setMidiFX(slotIndex, new MidiFXRandomizer());
	}
	break;
	case MIDIFX_HARMONIZER:
	{
		setMidiFX(slotIndex, new MidiFXHarmonizer());
	}
	break;
	case MIDIFX_SCALER:
	{
		setMidiFX(slotIndex, new MidiFXScaler());
	}
	break;
	case MIDIFX_MONOPHONIC:
	{
		setMidiFX(slotIndex, new MidiFXMonophonic());
	}
	break;
	case MIDIFX_ARP:
	{
		setMidiFX(slotIndex, new MidiFXArpeggiator());
	}
	break;
	default:
		break;
	}

	auto mfx = getMidiFX(slotIndex);
	if (mfx != nullptr)
	{
		mfx->setSelected(selected_);
	}

	if (!fromLoad)
	{
		displayMidiFXName(slotIndex);
	}

	midifxTypes_[slotIndex] = typeIndex;

	reconnectInputsOutputs();
}

// Where the magic happens
void SubModeMidiFxGroup::reconnectInputsOutputs()
{
	// Serial.println("SubModeMidiFxGroup::reconnectInputsOutputs");
	bool validMidiFXFound = false;
	midifx::MidiFXInterface *lastValidMidiFX = nullptr;

	for (int8_t i = NUM_MIDIFX_SLOTS - 1; i >= 0; --i)
	{
		// Serial.println("i = " + String(i));

		midifx::MidiFXInterface *fx = getMidiFX(i);

		if (fx == nullptr)
		{
			// Serial.println("midifx is null");

			continue;
		}

		// Last valid MidiFX, connect it's output to the main midifxgroup output
		if (!validMidiFXFound)
		{
			// Serial.println("connecting midifx to main output");

			fx->setNoteOutput(&SubModeMidiFxGroup::noteFuncForwarder, this);
			lastValidMidiFX = fx;
			validMidiFXFound = true;
		}
		// connect the output of this midiFX to the input of the next one
		else
		{
			// if(lastValidMidiFX == nullptr)
			// {
			//     Serial.println("lastValidMidiFX is null");
			// }

			// Serial.println("connecting midifx to previous midifx");

			fx->setNoteOutput(&MidiFXInterface::onNoteInputForwarder, lastValidMidiFX);
			lastValidMidiFX = fx;
		}
	}

	// Connect doNoteOutput_ to the lastValidMidiFX
	if (validMidiFXFound)
	{
		// Serial.println("connecting group to lastValidMidiFX");

		doNoteOutput_ = &MidiFXInterface::onNoteInputForwarder;
		doNoteOutputContext_ = lastValidMidiFX;
	}
	// No valid midifx, connect groups input to it's output
	else
	{
		// Serial.println("connecting group to self output");

		doNoteOutput_ = &SubModeMidiFxGroup::noteFuncForwarder;
		doNoteOutputContext_ = this;
	}
}

void SubModeMidiFxGroup::noteInput(MidiNoteGroup note)
{
	note.prevNoteNumber = note.noteNumber; // Cache the initial note number

	if (doNoteOutputContext_ == nullptr)
	{
		// bypass effects, sends out
		noteOutputFunc(note);
		return;
	}

	// Sends to connected function ptr
	doNoteOutput_(doNoteOutputContext_, note);
}

// Sets function pointer to send notes out of FX Group
void SubModeMidiFxGroup::setNoteOutputFunc(void (*fptr)(void *, MidiNoteGroup), void *context)
{
	sendNoteOutFuncPtr_ = fptr;
	sendNoteOutFuncPtrContext_ = context;
}

void SubModeMidiFxGroup::onPendingNoteOff(int note, int channel)
{
	// Serial.println("SubModeMidiFxGroup::onPendingNoteOff " + String(note) + " " + String(channel));

	// find and remove notes matching description
	for (uint8_t i = 0; i < 32; i++)
	{
		if (onNoteGroups[i].prevNoteNumber != 255)
		{
			if (onNoteGroups[i].channel == channel && onNoteGroups[i].noteNumber == note)
			{
				// Serial.println("found note, marking empty");

				onNoteGroups[i].prevNoteNumber = 255; // mark empty
			}
		}
	}
}

// Notes come here after passing through midifx
void SubModeMidiFxGroup::noteOutputFunc(MidiNoteGroup note)
{
	if (note.noteOff)
	{
		// Serial.println("Note off");
		// See if note was previously effected
		// Adjust note number if it was and remove from vector
		for (uint8_t i = 0; i < 32; i++)
		{
			if (onNoteGroups[i].prevNoteNumber != 255)
			{
				if (onNoteGroups[i].channel == note.channel && onNoteGroups[i].prevNoteNumber == note.prevNoteNumber)
				{
					// Serial.println("Note Found: " + String(note.prevNoteNumber));

					// Send note off with adjusted note number

					if (sendNoteOutFuncPtrContext_ != nullptr)
					{
						note.noteNumber = onNoteGroups[i].noteNumber;
						// MidiNoteGroup noteOff = onNoteGroups[i];
						// noteOff.noteOff = true;
						// noteOff.velocity = 0;
						// Serial.println("Note off sent: " + String(note.noteNumber));

						sendNoteOutFuncPtr_(sendNoteOutFuncPtrContext_, note);
					}
					onNoteGroups[i].prevNoteNumber = 255; // mark empty
				}
			}
		}
	}
	else if (!note.noteOff)
	{
		// Serial.println("Note on");

		// Keep track of note, up to 32 notes tracked at once
		for (uint8_t i = 0; i < 32; i++)
		{
			if (onNoteGroups[i].prevNoteNumber == 255)
			{
				// Serial.println("Found empty slot: " + String(note.prevNoteNumber));

				// onNoteGroups[i] = note;
				onNoteGroups[i].channel = note.channel;
				onNoteGroups[i].prevNoteNumber = note.prevNoteNumber;
				onNoteGroups[i].noteNumber = note.noteNumber;

				// Send the note out of FX group
				if (sendNoteOutFuncPtrContext_ != nullptr)
				{
					// Serial.println("Note on sent: " + String(note.noteNumber));
					sendNoteOutFuncPtr_(sendNoteOutFuncPtrContext_, note);
				}

				return;
			}
		}
	}
}

void SubModeMidiFxGroup::setupPageLegends()
{
	omxDisp.clearLegends();

	// omxDisp.dispPage = page + 1;

	int8_t page = params_.getSelPage();

	switch (page)
	{
	case MFXPAGE_FX:
	{
		omxDisp.legends[0] = "FX 1";
		omxDisp.legends[1] = "FX 2";
		omxDisp.legends[2] = "FX 3";
		omxDisp.legends[3] = "FX 4";
		omxDisp.legendVals[0] = -127;
		omxDisp.legendVals[1] = -127;
		omxDisp.legendVals[2] = -127;
		omxDisp.legendVals[3] = -127;
		omxDisp.legendText[0] = getMFXDispName(0);
		omxDisp.legendText[1] = getMFXDispName(1);
		omxDisp.legendText[2] = getMFXDispName(2);
		omxDisp.legendText[3] = getMFXDispName(3);
	}
	break;
	case MFXPAGE_FX2:
	{
		omxDisp.legends[0] = "FX 5";
		omxDisp.legends[1] = "FX 6";
		omxDisp.legends[2] = "FX 7";
		omxDisp.legends[3] = "FX 8";
		omxDisp.legendVals[0] = -127;
		omxDisp.legendVals[1] = -127;
		omxDisp.legendVals[2] = -127;
		omxDisp.legendVals[3] = -127;
		omxDisp.legendText[0] = getMFXDispName(4);
		omxDisp.legendText[1] = getMFXDispName(5);
		omxDisp.legendText[2] = getMFXDispName(6);
		omxDisp.legendText[3] = getMFXDispName(7);
	}
	break;
	case MFXPAGE_EXIT:
	{
		omxDisp.legends[0] = "Exit";
		omxDisp.legends[1] = "";
		omxDisp.legends[2] = "";
		omxDisp.legends[3] = "";
		omxDisp.legendVals[0] = -127;
		omxDisp.legendVals[1] = -127;
		omxDisp.legendVals[2] = -127;
		omxDisp.legendVals[3] = -127;
		omxDisp.legendText[0] = "Exit";
		omxDisp.legendText[1] = "";
		omxDisp.legendText[2] = "";
		omxDisp.legendText[3] = "";
	}
	break;
	default:
		break;
	}
}

void SubModeMidiFxGroup::onDisplayUpdateMidiFX()
{
	if (heldMidiFX_ >= 0)
	{
		const char *slotNames[NUM_MIDIFX_SLOTS];

		for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
		{
			auto mfx = getMidiFX(i);
			if (mfx == nullptr)
			{
				slotNames[i] = "-";
			}
			else
			{
				slotNames[i] = mfx->getDispName();
			}
		}

		omxDisp.dispSlots(slotNames, NUM_MIDIFX_SLOTS, selectedMidiFX_, heldAnimPos_, getEncoderSelect(), false, nullptr, 0);
		return;
	}

	if (funcKeyMode_ == FUNCKEYMODE_F1)
	{
		omxDisp.dispGenericModeLabel("Copy", params_.getNumPages(), params_.getSelPage());
	}
	else if (funcKeyMode_ == FUNCKEYMODE_F2)
	{
		omxDisp.dispGenericModeLabel("Paste", params_.getNumPages(), params_.getSelPage());
	}
	else if (funcKeyMode_ == FUNCKEYMODE_F3)
	{
		omxDisp.dispGenericModeLabel("Cut", params_.getNumPages(), params_.getSelPage());
	}
	else
	{
		MidiFXInterface *selFX = getMidiFX(selectedMidiFX_);

		if (selFX == nullptr)
		{
			omxDisp.displayMessage("No FX");
		}
		else
		{
			// Serial.println("Selected MidiFX not null");

			selFX->onDisplayUpdate(funcKeyMode_);
		}
	}
}

void SubModeMidiFxGroup::onDisplayUpdate()
{
	// omxLeds.updateBlinkStates();

	// if (omxLeds.isDirty())
	// {
	//     updateLEDs();
	// }

	if (omxDisp.isDirty())
	{
		if (!encoderConfig.enc_edit)
		{

			if (midiFXParamView_)
			{
				onDisplayUpdateMidiFX();
			}
			else
			{
				setupPageLegends();
				omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
			}
		}
	}
}

int SubModeMidiFxGroup::saveToDisk(int startingAddress, Storage *storage)
{
	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		MidiFXInterface *mFX = getMidiFX(i);

		if (mFX == nullptr)
		{
			// Serial.println("NoMFX");
			storage->write(startingAddress, MIDIFX_NONE);
			startingAddress++;
		}
		else
		{
			int mfxType = mFX->getFXType();
			// Serial.println((String)"MFX: " + mfxType);
			storage->write(startingAddress, mfxType);
			startingAddress++;

			startingAddress = mFX->saveToDisk(startingAddress, storage);
		}

		// Serial.println((String)"startingAddress: " + startingAddress);
	}

	return startingAddress;
}

int SubModeMidiFxGroup::loadFromDisk(int startingAddress, Storage *storage)
{
	for (uint8_t i = 0; i < NUM_MIDIFX_SLOTS; i++)
	{
		int mfxType = storage->read(startingAddress);
		startingAddress++;

		// Serial.println((String)"MFX: " + mfxType);

		changeMidiFXType(i, mfxType, true);

		MidiFXInterface *mFX = getMidiFX(i);

		if (mFX != nullptr)
		{
			startingAddress = mFX->loadFromDisk(startingAddress, storage);
		}
		else
		{
			// Serial.println("mfx is null");
		}

		// Serial.println((String)"startingAddress: " + startingAddress);
	}

	return startingAddress;
}
