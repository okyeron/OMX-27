#pragma once

#include <stdint.h>

class PendingNoteOffs {
	public:
		PendingNoteOffs();
		bool insert(int note, int channel, uint32_t time, bool sendCV);
		void play(uint32_t time);
		void allOff();

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
};

extern PendingNoteOffs pendingNoteOffs;


class PendingNoteOns {
	public:
		PendingNoteOns();
		bool insert(int note, int velocity, int channel, uint32_t time, bool sendCV);
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


		
