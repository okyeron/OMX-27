#pragma once
#include "midimacro_interface.h"
#include "../utils/PotPickupUtil.h"

// RAM:   [=====     ]  51.2% (used 33528 bytes from 65536 bytes)
// Flash: [======    ]  61.5% (used 161276 bytes from 262144 bytes)
// RAM:   [=====     ]  51.4% (used 33696 bytes from 65536 bytes)
// Flash: [======    ]  61.6% (used 161356 bytes from 262144 bytes)

namespace midimacro
{
	struct MidiParamBank
	{
		uint8_t keyMap;
		int keyColor = ORANGE;
		const char *bankName;
		uint8_t midiCCs[5] = {255, 255, 255, 255, 255};
		uint8_t midiValues[5] = {0, 0, 0, 0, 0};
		const char *paramNames[5];
		bool altBank = false;
		uint8_t activeParam = 0;

		void SetCCs(
			const char *nameA = "", uint8_t a = 255,
			const char *nameB = "", uint8_t b = 255,
			const char *nameC = "", uint8_t c = 255,
			const char *nameD = "", uint8_t d = 255,
			const char *nameE = "VOL", uint8_t e = 7)
		{
			SetCC(0, nameA, a);
			SetCC(1, nameB, b);
			SetCC(2, nameC, c);
			SetCC(3, nameD, d);
			SetCC(4, nameE, e);
		}

		void SetCC(uint8_t index, const char *name, uint8_t cc)
		{
			if (index >= 5)
				return;
			midiCCs[index] = cc;
			paramNames[index] = name;
		}

		int8_t ContainsCC(uint8_t cc)
		{
			for(int8_t i = 0; i < 5; i++)
			{
				if(midiCCs[i] == cc)
				{
					return i;
				}
			}
			return -1;
		}

		int8_t UpdateCCValue(uint8_t cc, uint8_t value)
		{
			int8_t index = ContainsCC(cc);
			if(index < 0) return index;
			midiValues[index] = value;
			return index;
		}

		void UpdatePotValue(uint8_t potIndex, uint8_t value)
		{
			midiValues[potIndex] = value;
		}
	};

	class MidiMacroDeluge : public MidiMacroInterface
	{
	public:
		MidiMacroDeluge();
		~MidiMacroDeluge() {}

		bool consumesPots() override { return true; }
		bool consumesDisplay() override { return true; }

		String getName() override;

		void loopUpdate() override;

		void onDisplayUpdate() override;

		void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) override;
		void onKeyUpdate(OMXKeypadEvent e) override;
		void drawLEDs() override;

		void inMidiControlChange(byte channel, byte control, byte value) override;
	protected:
		void onEnabled() override;
		void onDisabled() override;

		void onEncoderChangedEditParam(Encoder::Update enc) override;

	private:
		static const uint8_t kNumBanks = 16;

		MidiParamBank* getActiveBank();
		void keyDownBankShortcut(uint8_t keyIndex);
		void setActiveBank(uint8_t bankIndex);
		void updatePotPickups();

		MidiParamBank paramBanks[kNumBanks];
		// Maps each CC to a cached value
		uint8_t delVals[127];

		bool auxDown_ = false;
		uint8_t selBank = 0;
		uint8_t activeParam = 0;

		PotPickupUtil potPickups[5];

		// bool m8mutesolo_[16];

		// Control key mappings
		// static const uint8_t keyFilt1_ = 3;
		// static const uint8_t keyFilt2_ = 4;
		// static const uint8_t keyEnv1_ = 1;
		// static const uint8_t keyEnv2_ = 2;
	};
}
