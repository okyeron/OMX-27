# Change log

### 1.5.x

Move sequencer into it's own header/source files 

Add SysEx functionality and webmidi configuration

Fix for sequencer pattern save

### 1.4.x

Pattern Length can now be up to 64 steps  

SEQ Mode Quick Keys - Hold F1 + F2  
- first 4 "white keys" select which "page" of the current pattern 

MIDI Mode Quick Keys - Hold AUX  
- first 2 "white keys" are octave up/down  
- first 2 "black keys" are move param selection on the display  

Add MIDI mode params for bank select and program change  

Add RoundRobin MIDI Channel distribution and RR offset value  

Add potentiometer bank select  (CC assignments are still hard coded)


### 1.3.x

Step probability (percentage)

Step Type (play, mute, forward, reverse, random step, random event)

Conditional trigs (elektron style)

Added led status for step events (pulsing)

Bug fix - disable key long press in pattern params

Bug fix - correct wrong velocity sent in playnote on pattern switching

Pot 5 will change/set step velocity in Step Record


### 1.2.x

MIDI-solo

Swing

Auto-reset in patterns

New display layout

Plocks in steprecord

### 1.1.x

Save pattern data
