namespace MM {

	void begin();

	void sendNoteOn(int note, int velocity, int channel);
	void sendNoteOff(int note, int velocity, int channel);
	void sendControlChange(int control, int value, int channel);
	void sendProgramChange(int program, int channel);


	void sendClock();
	void startClock();
	void continueClock();
	void stopClock();

	bool usbMidiRead();
	bool midiRead();
}
