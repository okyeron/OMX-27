# OMX-27 Documentation

## Concepts

Sequencer has 8 patterns (tracks).


Pots are mapped by default to CCs 21, 22, 23, 24 and 7 (volume). These can be changed in the firmware. Hopefully this will be configurable from a web-MIDI interface in future.


## Layout

Top left key = AUX  

### In sequencer modes:

AUX key is transport start/stop.

__"Black keys" (sharp/flat keyboard keys) __ 

The first 2 black keys are "Function Keys" (FUNC)
- Function One (F1): Reset all sequences to step one. 
- Function Two (F2): Press and hold and then press any pattern key to Mute that pattern

The next 8 are Pattern Keys and they select the active sequence pattern.

Hold a key (long press) to access parameters for that pattern.

__"White keys" (bottom row)__

Step Keys - These are your sequencer step on/off keys. Hold a key (long press) to access parameters for that step.




## Modes

### MI - MIDI 

MIDI Keyboard. Encoder selects octave or channel.

AUX does nothing here (yet)

### S1 - Sequencer 1

Step sequencer - One pattern active at a time.

 - AUX is Start/Stop 

### S2 - Sequencer 2

Step sequencer - All patterns active.

- AUX is Start/Stop 
- F2 + pattern key will mute that pattern  

### OM - Organelle Mother

Pretty much the same as MI, but with the following tweaks for Organelle Mother on norns/fates.

- AUX key sends CC 25 (127 on press, 0 on release)  
- Encoder button sends CC 26 (100 on press, 0 on release)  
- Encoder turn sends CC 28 (127 on CW, 0 on CCW)  



