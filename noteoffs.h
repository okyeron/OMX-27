#pragma once

#include <stdint.h>


class PendingNoteOffs {
	public:
		PendingNoteOffs();
		bool insert(int note, int channel, uint32_t time);
		void play(uint32_t time);
		void allOff();

	private:    
		struct Entry {
			bool inUse;
			int note;
			int channel;
			uint32_t time;
		};
		static const int queueSize = 32;
		Entry queue[queueSize];
};

extern PendingNoteOffs pendingNoteOffs;


		
		
