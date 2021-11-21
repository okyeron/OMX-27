#include "MM.h"

#include <MIDI.h>

namespace {
    using SerialMIDI = midi::SerialMIDI<HardwareSerial>;
    using MidiInterface = midi::MidiInterface<SerialMIDI>;

    SerialMIDI theSerialInstance(Serial1);
    MidiInterface HWMIDI(theSerialInstance);
}

namespace MM {
    void begin() {
        HWMIDI.begin();

    }
    void sendNoteOn(int note, int velocity, int channel) {
        usbMIDI.sendNoteOn(note, velocity, channel);
        HWMIDI.sendNoteOn(note, velocity, channel);

    }
    void sendNoteOff(int note, int velocity, int channel) {
        usbMIDI.sendNoteOff(note, velocity, channel);
        HWMIDI.sendNoteOff(note, velocity, channel);

    }
    void sendControlChange(int control, int value, int channel) {
        usbMIDI.sendControlChange(control, value, channel);
        HWMIDI.sendControlChange(control, value, channel);
    }

    void sendProgramChange(int program, int channel) {
        usbMIDI.sendProgramChange(program, channel);
        HWMIDI.sendProgramChange(program, channel);
    }
    void sendClock() {
        usbMIDI.sendRealTime(usbMIDI.Clock);
        HWMIDI.sendClock();
    }

    void startClock(){
        usbMIDI.sendRealTime(usbMIDI.Start);
        HWMIDI.sendStart();
    }
    void continueClock(){
        usbMIDI.sendRealTime(usbMIDI.Continue);
        HWMIDI.sendContinue();
    }
    void stopClock(){
        usbMIDI.sendRealTime(usbMIDI.Stop);
        HWMIDI.sendStop();
    }

    // NEED SOMETHING FOR usbMIDI.read() / MIDI.read()

    bool usbMidiRead(){
        return usbMIDI.read();
    }
    bool midiRead(){
        return HWMIDI.read();
    }
}
