#include "noteoffs.h"

#include <MIDI.h>

PendingNoteOffs::PendingNoteOffs() {
	for (int i = 0; i < queueSize; ++i)
		queue[i].inUse = false;
}


bool PendingNoteOffs::insert(int note, int channel, uint32_t time) {
	for (int i = 0; i < queueSize; ++i) {
		if (queue[i].inUse) continue;
		queue[i].inUse = true;
		queue[i].note = note;
		queue[i].time = time;
		queue[i].channel = channel;
		return true;
	}
	return false; // couldn't find room!
}

void PendingNoteOffs::play(uint32_t now) {
	for (int i = 0; i < queueSize; ++i) {
		if (queue[i].inUse && queue[i].time <= now) {
		usbMIDI.sendNoteOff(queue[i].note, 0, queue[i].channel);
// 		MIDI.sendNoteOff(queue[i].note, 0, queue[i].channel);

// 		const int CVGATE_PIN = 13; // ;	// 13 on beta1 boards, 22 on test, 23 on 1.0
// 		const int CVPITCH_PIN = A14;
// 		analogWrite(CVPITCH_PIN, 0);
// 		digitalWrite(CVGATE_PIN, LOW);

		queue[i].inUse = false;
		}
	}
}

void PendingNoteOffs::allOff() {
	play(UINT32_MAX);
}

PendingNoteOffs pendingNoteOffs;