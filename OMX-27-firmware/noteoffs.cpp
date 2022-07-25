#include "noteoffs.h"

#include <Arduino.h>
#include "consts.h"
#include "MM.h"


PendingNoteOffs::PendingNoteOffs() {
	for (int i = 0; i < queueSize; ++i)
		queue[i].inUse = false;
}

bool PendingNoteOffs::insert(int note, int channel, uint32_t time, bool sendCV)
{
	for (int i = 0; i < queueSize; ++i)
	{
		if (queue[i].inUse)
			continue;
		queue[i].inUse = true;
		queue[i].note = note;
		queue[i].time = time;
		queue[i].channel = channel;
		queue[i].sendCV = sendCV;
		return true;
	}
	return false; // couldn't find room!
}

void PendingNoteOffs::play(uint32_t now)
{
	for (int i = 0; i < queueSize; ++i)
	{
		if (queue[i].inUse && queue[i].time <= now)
		{
			MM::sendNoteOff(queue[i].note, 0, queue[i].channel);
			//	 		analogWrite(CVPITCH_PIN, 0);
			if (queue[i].sendCV)
			{
				digitalWrite(CVGATE_PIN, LOW);
			}
			queue[i].inUse = false;

			onNoteOff(queue[i].note, queue[i].channel);
		}
	}
}

void PendingNoteOffs::sendOffNow(int note, int channel, bool sendCV)
{
	bool noteOffSent = false;

	// Find notes in queue matching note number and channel
	for (int i = 0; i < queueSize; ++i)
	{
		if (queue[i].inUse && queue[i].channel == channel && queue[i].note == note)
		{
			// Send note off event for first note found
			// Other pending note offs just get set to not in use. 
			if (!noteOffSent)
			{
				MM::sendNoteOff(queue[i].note, 0, queue[i].channel);
				//	 		analogWrite(CVPITCH_PIN, 0);
				if (queue[i].sendCV)
				{
					digitalWrite(CVGATE_PIN, LOW);
				}
				noteOffSent = true;
				onNoteOff(queue[i].note, queue[i].channel);
			}
			queue[i].inUse = false;
		}
	}

	if(!noteOffSent)
	{
		MM::sendNoteOff(note, 0, channel);
		if (sendCV)
		{
			digitalWrite(CVGATE_PIN, LOW);
		}
		onNoteOff(note, channel);
	}
}

void PendingNoteOffs::allOff() {
	play(UINT32_MAX);
}

void PendingNoteOffs::setNoteOffFunction(void (*fptr)(void *, int note, int channel), void *context)
{
	setNoteOffFuncPtrContext = context;
	setNoteOffFuncPtr = fptr;
}

void PendingNoteOffs::onNoteOff(int note, int channel)
{
	Serial.println("PendingNoteOffs::onNoteOff " + String(note) + " " + String(channel));
	if (setNoteOffFuncPtrContext != nullptr)
	{
		Serial.println("PendingNoteOffs::onNoteOff sending to pointer");
		setNoteOffFuncPtr(setNoteOffFuncPtrContext, note, channel);
	}
	else{
		Serial.println("PendingNoteOffs::onNoteOff pointer not found");
	}
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

bool PendingNoteOns::remove(int note, int channel)
{
	bool foundNoteToRemove = false;

	// Find notes in queue matching note number and channel
	for (int i = 0; i < queueSize; ++i)
	{
		if (queue[i].inUse && queue[i].channel == channel && queue[i].note == note)
		{
			queue[i].inUse = false;
			foundNoteToRemove = true;
		}
	}

	return foundNoteToRemove;
}

void PendingNoteOns::play(uint32_t now)
{
	int pCV;
	for (int i = 0; i < queueSize; ++i)
	{
		if (queue[i].inUse && queue[i].time <= now)
		{
			MM::sendNoteOn(queue[i].note, queue[i].velocity, queue[i].channel);

			if (queue[i].sendCV)
			{
				if (queue[i].note >= midiLowestNote && queue[i].note < midiHightestNote)
				{
					pCV = static_cast<int>(roundf((queue[i].note - midiLowestNote) * stepsPerSemitone));
					digitalWrite(CVGATE_PIN, HIGH);
					analogWrite(CVPITCH_PIN, pCV);
				}
			}
			queue[i].inUse = false;
		}
	}
}

PendingNoteOns pendingNoteOns;
