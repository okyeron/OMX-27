#pragma once

#include <stdint.h>

class PendingNoteHistory
{
public:
	PendingNoteHistory();
	void clear();
	void clearIfChanged(uint32_t time);
	bool insert(int note, int channel);
	bool eventThisFrame(int note, int channel);

private:
	struct Entry
	{
		bool inUse = false;
		int note : 7;
		int channel : 5;
	};
	static const int queueSize = 32;
	Entry queue[queueSize];

	uint32_t prevTime;
};

extern PendingNoteHistory pendingNoteHistory;

class PendingNoteOffs {
	public:
		PendingNoteOffs();
		bool insert(int note, int channel, uint32_t time, bool sendCV);
		void play(uint32_t time);

		// Finds any pending note offs for this note and kills them
		// so they won't later fire
		// then sends the note off event now
		bool sendOffIfPresent(int note, int channel, bool sendCV);
		void sendOffNow(int note, int channel, bool sendCV);
		void allOff();

		void setNoteOffFunction(void (*fptr)(void *, int note, int channel), void *context);
	private:
		struct Entry {
			bool inUse;
			int note;
			int channel;
			bool sendCV;
			uint32_t time;
		};
		static const int queueSize = 32;
		Entry queue[queueSize];

		void onNoteOff(int note, int channel);

		// Pointer to external function that notes are sent out of fxgroup to
		void *setNoteOffFuncPtrContext = nullptr;
		void (*setNoteOffFuncPtr)(void *, int note, int channel);
};

extern PendingNoteOffs pendingNoteOffs;


class PendingNoteOns {
	public:
		PendingNoteOns();
		bool insert(int note, int velocity, int channel, uint32_t time, bool sendCV);

		// Remove any notes matching description
		bool remove(int note, int channel);
		void play(uint32_t time);
	private:
		struct Entry {
			bool inUse;
			int note;
			int channel;
			int velocity;
			bool sendCV;
			uint32_t time;
		};
		static const int queueSize = 32;
		Entry queue[queueSize];
};

extern PendingNoteOns pendingNoteOns;



