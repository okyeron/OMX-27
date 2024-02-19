#include "submode_preset.h"
#include "../../hardware/omx_disp.h"
#include "../../hardware/omx_leds.h"
#include "../../consts/colors.h"

const char *saveLabel = "Save to";
const char *loadLabel = "Load from";

enum PotConfigPage
{
	POTPAGE_1,
	POTPAGE_2,
	POTPAGE_EXIT
};

SubModePreset::SubModePreset()
{
	params_.addPage(4);
	params_.addPage(4);
	params_.addPage(1); // Exit submode
}

void SubModePreset::onEnabled()
{
	params_.setSelPageAndParam(0, 0);
	encoderSelect_ = true;
	omxLeds.setDirty();
	omxDisp.setDirty();

	auxReleased_ = !midiSettings.keyState[0];
}

void SubModePreset::onDisabled()
{
	strip.clear();
	omxLeds.setDirty();
	omxDisp.setDirty();
}

void SubModePreset::configure(SubmodePresetMode mode, uint8_t selPreset, uint8_t numPresets, bool autoSave)
{
	if(selPreset >= numPresets || numPresets >= 16)
	{
		// Too many presets, or selPreset out of range
		return;
	}

	this->mode = mode;
	this->selPreset = selPreset;
	this->numPresets = numPresets;
	this->autoSave = autoSave;
}

void SubModePreset::setContextPtr(void *context)
{
	fptrContext_ = context;
}
void SubModePreset::setDoSaveFunc(void (*fptr)(void *, uint8_t))
{
	doSaveFptr_ = fptr;
}
void SubModePreset::setDoLoadFunc(void (*fptr)(void *, uint8_t))
{
	doLoadFptr_ = fptr;
}

void SubModePreset::doSave(uint8_t presetIndex)
{
	doSaveFptr_(fptrContext_, presetIndex);
}

void SubModePreset::doLoad(uint8_t presetIndex)
{
	doLoadFptr_(fptrContext_, presetIndex);
}

void SubModePreset::loopUpdate()
{
}

bool SubModePreset::updateLEDs()
{
	strip.clear();

	// bool blink = omxLeds.getBlinkState();
	bool slowBlink = omxLeds.getSlowBlinkState();

	// strip.setPixelColor(0, blink ? LTPURPLE : RED);

	strip.setPixelColor(0, RED);


    int keyColor = mode == PRESETMODE_LOAD ? BLUE : ORANGE;
    int highlightColor = mode == PRESETMODE_LOAD ? LTCYAN : LTYELLOW;

    for(uint8_t i = 11; i < 11 + numPresets; i++)
    {
        uint8_t presetIndex = i - 11;

        int color = (slowBlink && presetIndex == selPreset) ? highlightColor : keyColor;

        strip.setPixelColor(i, color);
    }

	return true;
}

void SubModePreset::onEncoderChanged(Encoder::Update enc)
{
	SubmodeInterface::onEncoderChanged(enc);
}

void SubModePreset::onEncoderChangedEditParam(Encoder::Update enc)
{
	omxDisp.setDirty();
	omxLeds.setDirty();
}

void SubModePreset::onEncoderButtonDown()
{
	omxDisp.setDirty();
	omxLeds.setDirty();
}

bool SubModePreset::onKeyUpdate(OMXKeypadEvent e)
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

        if (thisKey >= 11 && thisKey < 11 + numPresets)
        {
            uint8_t newPresetIndex = thisKey - 11;

            if (mode == PRESETMODE_LOAD)
            {
                // Auto save current selected drum kit if loading a new one
                if(newPresetIndex != selPreset && autoSave)
                {
					doSave(selPreset);
                }
				doLoad(newPresetIndex);
                omxDisp.displayMessage("Loaded " + String(newPresetIndex + 1));
            }
            else if (mode == PRESETMODE_SAVE)
            {
				doSave(newPresetIndex);
                omxDisp.displayMessage("Saved " + String(newPresetIndex + 1));
            }
            selPreset = thisKey - 11;
			setEnabled(false);
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

void SubModePreset::onDisplayUpdate()
{
	if (omxDisp.isDirty())
	{
		if (!encoderConfig.enc_edit)
		{
			omxDisp.dispGenericModeLabel(mode == PRESETMODE_LOAD ? loadLabel : saveLabel, 1, 0);
		}
	}
}
