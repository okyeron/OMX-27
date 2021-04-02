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

Hold a key (long press) to access parameters for that pattern. This is "Pattern-params".

__"White keys" (bottom row)__

Step Keys - These are your sequencer step on/off keys. 

Hold a key (long press) to access parameters for that step. This is "Note-select".

__Encoder__

Long press encoder to enable mode change. Turn to switch modes, short-press to enter mode.

Within the modes a short press on the encoder will select a parameter to edit.

In some modes, turning the encoder is set to a default param (like BPM)


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



# MIDI Switch for the mini TRS jack connection

A hardware switch on the device will let you swap between Type-A and Type-B for the hardware MIDI TRS jack.  


Products That Use Type-B mini TRS Jack Connections  
- Arturia BeatStep Pro  
- Novation products  
- 1010music Original Series 1 modules, Series 2 modules, Blackbox, MX4 and Euroshield  

Products That Use Type-A mini TRS Jack Connections   
- ADDAC System products  
- Arturia BeatStep (not to be confused with the BeatStep Pro)  
- IK Multimedia products  
- inMusic (Akai) products  
- Korg products  
- Line 6 products  
- little Bits w5 MIDI module  
- Make Noise 0-Coast  

See https://1010music.com/stereo-minijacks-midi-connections-compatibility-guide for more information

