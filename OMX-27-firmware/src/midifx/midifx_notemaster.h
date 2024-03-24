#pragma once
#include "../config.h"
// #include "../ClearUI/ClearUI_Input.h"
#include "../hardware/omx_keypad.h"
#include "../utils/param_manager.h"

namespace midifx
{
	/// I am the note master
	/// I am the master of the notes
	/// all your notes are belong to me
	/// Not a MidiFX, but a utility for MidiFX to use
	class MidiFXNoteMaster
	{
	public:
		MidiFXNoteMaster();
		~MidiFXNoteMaster();

		void setContext(void *context);
		void setProcessNoteFptr(void (*fptr)(void *, MidiNoteGroup*));
		void setSendNoteOutFptr(void (*fptr)(void *, MidiNoteGroup*));

		// Send note here in the case that it is passing through effect
		// due to chance. IE, passing to next MidiFX slot
		// If effect is off, you can send note straight through
		void trackNoteInputPassthrough(MidiNoteGroup *note);

		// Send note here in the case that is is going through the effect
		void trackNoteInput(MidiNoteGroup *note);

		void removeFromTracking(MidiNoteGroup *note);

		void clear();


	private:
		struct TrackingGroup
		{
			uint8_t channel : 5;
			uint8_t noteNumber;
			uint8_t prevNoteNumber;
		};

		void *outFunctionContext_;
		void (*processNoteFptr)(void *, MidiNoteGroup*);
		void (*sendNoteOutFptr)(void *, MidiNoteGroup*);

		static const uint8_t kTrackingSize = 8;
		static const uint8_t kEmptyIndex = 255;

		TrackingGroup trackingNoteGroups[kTrackingSize];
		TrackingGroup trackingNoteGroupsPassthrough[kTrackingSize];

		void handleNoteInputPassthrough(MidiNoteGroup *note);
		void handleNoteInput(MidiNoteGroup *note);

		// static bool findEmptySlot(TrackingGroup *trackingArray, uint8_t size, uint8_t *emptyIndex);

		void processNoteInput(MidiNoteGroup *note);
		void sendNoteOut(MidiNoteGroup *note);
	};
}
