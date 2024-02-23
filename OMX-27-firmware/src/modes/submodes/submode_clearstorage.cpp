#include "submode_clearstorage.h"
#include "../../hardware/omx_disp.h"
#include "../../hardware/omx_leds.h"
#include "../../consts/colors.h"

const char *confirmLabel = "Erase Storage?";
const char *erasedLabel = "Erased";
const char *restartLabel = "Restart OMX";
const char *confirmOptions[2] = {"NO", "YES"};

enum ClearStorageState
{
    CLRSTOR_CONFIRM,
    CLRSTOR_RESTART
};

SubModeClearStorage::SubModeClearStorage()
{
    state = CLRSTOR_CONFIRM;
	params_.addPage(2);
}

void SubModeClearStorage::onEnabled()
{
    state = CLRSTOR_CONFIRM;
	params_.setSelPageAndParam(0, 0);
	encoderSelect_ = true;
	omxLeds.setDirty();
	omxDisp.setDirty();

	auxReleased_ = !midiSettings.keyState[0];
}

void SubModeClearStorage::onDisabled()
{
	strip.clear();
	omxLeds.setDirty();
	omxDisp.setDirty();
}

// void SubModeClearStorage::configure(SubmodePresetMode mode, uint8_t selPreset, uint8_t numPresets, bool autoSave)
// {
// 	if(selPreset >= numPresets || numPresets >= 16)
// 	{
// 		// Too many presets, or selPreset out of range
// 		return;
// 	}

// 	this->mode = mode;
// 	this->selPreset = selPreset;
// 	this->numPresets = numPresets;
// 	this->autoSave = autoSave;
// }

// void SubModeClearStorage::setContextPtr(void *context)
// {
// 	fptrContext_ = context;
// }
// void SubModeClearStorage::setDoSaveFunc(void (*fptr)(void *, uint8_t))
// {
// 	doSaveFptr_ = fptr;
// }
// void SubModeClearStorage::setDoLoadFunc(void (*fptr)(void *, uint8_t))
// {
// 	doLoadFptr_ = fptr;
// }

// void SubModeClearStorage::doSave(uint8_t presetIndex)
// {
// 	doSaveFptr_(fptrContext_, presetIndex);
// }

// void SubModeClearStorage::doLoad(uint8_t presetIndex)
// {
// 	doLoadFptr_(fptrContext_, presetIndex);
// }

void SubModeClearStorage::loopUpdate()
{
}

bool SubModeClearStorage::updateLEDs()
{
	strip.clear();

	strip.setPixelColor(0, RED);

    if (state == CLRSTOR_CONFIRM)
    {
        bool blinkState = omxLeds.getBlinkState();

        int8_t selParam = params_.getSelParam();    

        auto keyColor = blinkState ? (selParam == 0 ? RED : GREEN) : LEDOFF;

        for (uint8_t k = 1; k < 27; k++)
        {
            strip.setPixelColor(k, keyColor);
        }
    }

    return true;
}

void SubModeClearStorage::onEncoderChanged(Encoder::Update enc)
{
	SubmodeInterface::onEncoderChanged(enc);
}

void SubModeClearStorage::onEncoderChangedEditParam(Encoder::Update enc)
{
	omxDisp.setDirty();
	omxLeds.setDirty();
}

void SubModeClearStorage::onEncoderButtonDown()
{
    int8_t selParam = params_.getSelParam();

    if (state == CLRSTOR_CONFIRM)
    {
        if (selParam == 0) // No
        {
            omxDisp.displayMessage(exitMsg);
            setEnabled(false);
            return;
        }
        else if (selParam == 1) // Yes
        {
            state = CLRSTOR_RESTART;
            eraseStorage();
            omxDisp.displayMessage(erasedLabel);
            return;
        }
    }

    omxDisp.setDirty();
	omxLeds.setDirty();
}

void SubModeClearStorage::setStoragePtr(Storage *storagePtr)
{
    this->storagePtr = storagePtr;
}

void SubModeClearStorage::eraseStorage()
{
    if (storagePtr != nullptr)
    {
        storagePtr->write(EEPROM_HEADER_ADDRESS + 0, 0); // Saves EEPROM_VERSION as 0 so storage is cleared on restart.
    }
}

bool SubModeClearStorage::onKeyUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();

    if (e.down())
    {
        if (thisKey == 0)
        {
            // Aux key to cancel and go back
			if (auxReleased_)
			{
            	omxDisp.displayMessage(exitMsg);
				setEnabled(false);
				return true;
			}
            return true;
        }
    }
	// Key Up
	else
	{
		if (thisKey == 0)
		{
			// Used to prevent quickly exiting if entered through aux shortcut.
			auxReleased_ = true;
		}
	}

	omxDisp.setDirty();
	omxLeds.setDirty();

	return true;
}

void SubModeClearStorage::onDisplayUpdate()
{
    if (omxDisp.isDirty())
    {
        if (!encoderConfig.enc_edit)
        {
            if (state == CLRSTOR_CONFIRM)
            {
                int8_t selParam = params_.getSelParam();
                omxDisp.dispOptionCombo(confirmLabel, confirmOptions, 2, selParam, true);
            }
            else if (state == CLRSTOR_RESTART)
            {
                omxDisp.dispGenericModeLabel(restartLabel, 1, 0);
            }
        }
    }
}
