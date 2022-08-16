# Change log

### 1.7.7beta
- MI: Fixes normal midi notes being sent while in M8 midi macro mode. 

### 1.7.5beta
- Entering a new mode will set param and page to a default state and put param mode into param select. 
- GR: Using quickkeys in grids mode will now automatically put param mode into param edit. 
- GR: Grids LEDs will not render other leds when holding F1 to change the instrument. 
- S1/S2: Some cleaning of the sequencer mode code to make it easier to check current sequencer mode. 
- S1/S2: Fixed being able to enter patten params mode in S1 & S2 when in note select mode. 
- S1/S2: Adds display message when holding encoder to reset steps in step edit mode.
- S1/S2: Clearing a pattern (F1 + F2 + Pattern Key) in Pattern Params mode(hold pat key) now fully clears pattern. 

### 1.7.4beta
Fixes S1 S2 Bugs:
 - Step LEDs being off by one. Effected step record mode and sequencer reset
 - Note number of current step in step record being off
 - Being able to enter the pattern settings in step record mode if holding a pattern select key
 - Density not being able to be changed in other pages

 Adds Features:
 - Rendering of the step note in step record and note edit modes for S1 and S2
 - New interaction mode. Pressing encoder will toggle between selecting and editing parameters. 
    - This has been applied to all modes. 
    - Param selection vs editing is differentiated by a box highlighting the param in selection mode. 
 - New ParamManager class makes it very easy to manager pages and params. 
    - Big code improvement, however increased possibility that some things might not work right. 

### 1.7.x
Code refactor
Adds Grids sequencer

### 1.6.0
Add MIDI Macro sub-mode for M8 Tracker
Add screensaver and inactivity led animation for MI Mode
Documentation updates

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
