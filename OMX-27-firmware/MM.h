namespace MM {

	void begin();

	void sendNoteOn(int note, int velocity, int channel);
	void sendNoteOff(int note, int velocity, int channel);
	void sendControlChange(int control, int value, int channel);
	void sendProgramChange(int program, int channel);
	void sendNoteOnHW(int note, int velocity, int channel);
	void sendNoteOffHW(int note, int velocity, int channel);
	void sendControlChangeHW(int control, int value, int channel);
	void sendSysEx(uint32_t length, const uint8_t *sysexData, bool hasBeginEnd);

	void sendClock();
	void startClock();
	void continueClock();
	void stopClock();

	bool usbMidiRead();
	bool midiRead();
}
