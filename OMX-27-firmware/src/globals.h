#include "config.h"
#include "hardware/storage.h"
#include "midi/sysex.h"
#include "midi/MIDIClockStats.h"

extern SysSettings sysSettings;
extern PotSettings potSettings;
extern MidiMacroConfig midiMacroConfig;
extern MIDIClockStats clockstats;

// setup EEPROM/FRAM storage
extern Storage *storage;
extern SysEx *sysEx;

