# OMX-27 Documentation

## Concepts

OMX-27 is a MIDI Keyboard and Sequencer. Both USBMIDI and hardware MIDI (via 1.8" TRS jack) are supported. Various "modes" can be accessed with the encoder and specific functions, parameters or sub-modes can be accessed with the encoder or key-presses/key-combinations.

Sequencer modes have 8 patterns (tracks). Sequencer modes currently send MIDI clock and transport control (start/stop) by default.

CV pitch output is limited to about 4.5 octaves.

USBMIDI should be plug-and-play with any USBMIDI compatible host. iPad works great with the camera connection kit. Hardware MIDI TRS jack is switchable between Type-A and Type-B.


## Layout

Top left key = AUX Key 

Potentiometers are mapped by default to CCs 21, 22, 23, 24 and 7 (volume). These can be changed in the firmware. Hopefully this will be configurable from a web-MIDI interface in future.

__Encoder__

Long press encoder to enable mode change. Turn to switch modes, short-press to enter mode.

Within the modes a short press on the encoder will select a parameter to edit.

In some modes, turning the encoder is set to a default param (like octave or BPM)


### Key Switches

In MIDI modes the key switches work as a normal MIDI keyboard.

In sequencer modes the switches are broken into groups - Function Keys, Pattern Keys and Step Keys.


<img src="omx27_layout.png" alt="layout" width="884" height="220" />


__"Black keys" (sharp/flat keyboard keys)__ 

The first 2 black keys are Function Keys (FUNC)  
- F1  
- F2

The next 8 are Pattern Keys and they select the active sequence pattern (P1-P8).

Hold a key (long press) to access parameters for that pattern. This is "Pattern Params".


__"White keys" (bottom row)__

Step Keys - These are your sequencer step on/off keys. 

Hold a key (long press) to access parameters for that step. This is "Note Select".


---

## Modes

### MI - MIDI 

MIDI Keyboard. Encoder selects octave or channel.

AUX does nothing here (yet)

### S1 - Sequencer 1

Step sequencer - One pattern active at a time.

 - AUX is Start/Stop 
 - Pattern Key: Selects playing pattern
 - F1 + AUX: Reset sequences to first/last step 
 - F2 + AUX: Reverse pattern direction 
 - F1 + Pattern Key: Enter "Step Record Mode" (transport must be stopped)
 - Long press a Step Key: Enter "Note Select Mode"
 - Long press a Pattern Key: Enter "Pattern Params Mode"

### S2 - Sequencer 2

Step sequencer - All patterns active.

 - AUX is Start/Stop 
 - Pattern Key: Selects active pattern
 - F1 + AUX: Reset sequences to first/last step 
 - F2 + AUX: Reverse pattern direction 
 - F1 + Pattern Key: Enter "Step Record Mode"
 - F2 + Pattern Key: Mute that pattern  
 - Long press a Step Key: Enter "Note Select Mode"
 - Long press a Pattern Key: Enter "Pattern Params Mode"


### OM - Organelle Mother

Pretty much the same as MI, but with the following tweaks for Organelle Mother on norns/fates.

- AUX key sends CC 25 (127 on press, 0 on release)  
- Encoder button sends CC 26 (100 on press, 0 on release)  
- Encoder turn sends CC 28 (127 on CW, 0 on CCW)  


## Sub-Modes

### Note Select

### Pattern Params

Long press Pattern Key to enter pattern params

Encoder press to get parameters for length, rotation and MIDI channel for that pattern (blinking)

Step Keys also set pattern length 
F1 + pattern will copy pattern  
F2 + pattern will paste pattern to other pattern slot  
F1 + F2 + pattern will clear the pattern back to GM drum map default (and clear all plocks)  

(you can paste multiple times - paste buffer should stay the same until you copy again)


### Step Record

Holding F1 + a Pattern Key will enter Step Record Mode.

Enter notes from the keyboard and the sequence step will automatically advance to the next step. 

You can use the encoder button to skip a step (making no change)

Turn the encoder to change octave for the note you enter.


# MIDI Switch for the mini TRS jack connection

A hardware switch on the device will let you swap between Type-A and Type-B for the hardware MIDI TRS jack.  


Products That Use Type-B mini TRS Jack Connections  
- Arturia BeatStep Pro  
- Novation products  
- Polyend products  
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

