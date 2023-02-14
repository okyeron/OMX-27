#include "noteoffs.h"

#include <Arduino.h>
#include "consts.h"
#include "config.h"
#include "MM.h"


PendingNoteOffs::PendingNoteOffs() {
	for (int i = 0; i < queueSize; ++i)
		queue[i].inUse = false;
}


bool PendingNoteOffs::insert(int note, int channel, uint32_t time, bool sendCV) {
	for (int i = 0; i < queueSize; ++i) {
		if (queue[i].inUse) continue;
		queue[i].inUse = true;
		queue[i].note = note;
		queue[i].time = time;
		queue[i].channel = channel;
		queue[i].sendCV = sendCV;
		return true;
	}
	return false; // couldn't find room!
}

void PendingNoteOffs::play(uint32_t now) {
	for (int i = 0; i < queueSize; ++i) {
		if (queue[i].inUse && queue[i].time <= now) {
			MM::sendNoteOff(queue[i].note, 0, queue[i].channel);
//	 		analogWrite(CVPITCH_PIN, 0);
			if (queue[i].sendCV) {
				digitalWrite(CVGATE_PIN, LOW);
			}
		queue[i].inUse = false;
		}
	}
}

void PendingNoteOffs::allOff() {
	play(UINT32_MAX);
}

PendingNoteOffs pendingNoteOffs;

///

PendingNoteOns::PendingNoteOns() {
	for (int i = 0; i < queueSize; ++i)
		queue[i].inUse = false;
}

bool PendingNoteOns::insert(int note, int velocity, int channel, uint32_t time, bool sendCV) {
	for (int i = 0; i < queueSize; ++i) {
		if (queue[i].inUse) continue;
		queue[i].inUse = true;
		queue[i].note = note;
		queue[i].time = time;
		queue[i].channel = channel;
		queue[i].velocity = velocity;
		queue[i].sendCV = sendCV;
		return true;
	}
	return false; // couldn't find room!
}

void PendingNoteOns::play(uint32_t now) {
	int pCV;
	for (int i = 0; i < queueSize; ++i) {
		if (queue[i].inUse && queue[i].time <= now) {
		MM::sendNoteOn(queue[i].note, queue[i].velocity, queue[i].channel);

		if (queue[i].sendCV){
			if (queue[i].note>=midiLowestNote && queue[i].note <midiHightestNote){
				pCV = static_cast<int>(roundf( (queue[i].note - midiLowestNote) * stepsPerSemitone)); // map (adjnote, 36, 91, 0, 4080);
				digitalWrite(CVGATE_PIN, HIGH);
				#if T4
					dac.setVoltage(pCV, false);
				#else		
					analogWrite(CVPITCH_PIN, pCV);
				#endif
			}
		}
		queue[i].inUse = false;
		}
	}
}

PendingNoteOns pendingNoteOns;
