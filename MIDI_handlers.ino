void HandleNoteOn(byte channel, byte pitch, byte velocity) {
	bothNoteOff(channel, pitch, velocity);
}

void HandleNoteOff(byte channel, byte pitch, byte velocity) {
  bothNoteOff(channel, pitch, velocity);
}

void bothNoteOff(byte channel, byte pitch, byte velocity) { //this is called by handle noteoff and note on when velocity = 0

}

void HandleControlChange (byte channel, byte number, byte value) {

}
