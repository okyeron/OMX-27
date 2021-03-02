#include "noteoffs.h"

#include "MM.h"

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
		MM::sendNoteOff(queue[i].note, 0, queue[i].channel);

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