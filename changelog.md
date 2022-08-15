# Change log

```
### 1.11.1 alpha
***Warning: Will clear your global save state***
- Adds new Chord Mode
- Chords from chords mode will get saved with global save
- Scale settings now also get saved with global save
- Lower 16 keys play chords. 
- Each Chord can be configured by pressing it and using params or in chord edit mode
- Hold F1 + a chord key to jump into chord edit mode
- Chord Edit Mode:
-  Lower Right 8 keys set degree and play the chord
-  Right 4 keys set the base amount of notes in chord
-  Lower Key 5 toggles Spread Up Down - spreads notes in both directions
-  Lower Key 6 toggles Quartal Voicing
-  Key 1 quickselects Root Note
-  Key 2 quickselects Scale Pattern
-  Key 3 quickselects Chord Octave Offset, hold and use lower keys to change value
-  Key 4 quickselects Chord Transpose, hold and use lower keys to change value
-  Key 5 quickselects Chord Spread, hold and use lower keys to change value
-  Key 6 quickselects Chord Rotation, hold and use lower keys to change value
-  Key 7 quickselects Chord Voicing, hold and use lower keys to change value

- Spread:
   Spreads the notes accross note range by lowering or increasing each note of the chord by an octave

- Rotation:
   Changes the order of the notes in the scale

- Voicings:
   0 = none - based off numNotes
   1 = powerChord
   2 = sus2
   3 = sus4
   4 = sus2 + sus4
   5 = add 6th
   6 = add 6th and add 9th
   7 = Kenny Barron Jazz Minor 11th
```

```
### alpha10
- Adds 5 global midifx shared between euclidean sequencer and MI modes. 
- Quickly switch midifx mode in MI mode using aux plus upper 5 keys. 
- Improved MidiFX UI. 
- MidiFX slots in a midifx group increased from 4 to 8
- MidiFX and Euclidean sequencer states are saved with global save
- Cut/copy/paste MidiFX slots in a MidiFX group using F1 and F2 keys. 
```

```
### alpha9
- Adds Norns macro mode
- Adds second page to m8 mode for remote or headless control
```

```
### alpha8
- Reduces number of euclids to 8. Resource problems with 16, pots not updating correctly
- Adds modes: MIX, EDIT, PATTERN
- Hold F2 + select save slot to save. Press key to load. Same concept as grids. 
- Knobs only work to adjust euclid pattern in edit mode. 
- Future plans for MIX mode, but doesn't do much currently. 
```

```
### alpha7
- Increases number of euclids to 16 and MidiFX for euclidean seq mode to 5 for fun and profit. 
- Select a euclid in lower 16. Assign it to 1 of the 5 midi FX in blue on top row
- Hold down on a midifx to enter submode and adjust parameters. 
```

A light update tonight
```
### alpha6
- Adds new Harmonizer MidiFX
- ORIG plays original note if on, does not play if off. 
- Add up to 7 additional notes
- Play complex chords with ease!
```

```
### alpha5
Warning: Alpha build, UI not really fleshed out, might lock up, LEDs not fully implemented for Euclidean Mode
- Euclid UI improvements
- Can change variables for each euclid now like note number, midi channel, velocity
- LEDS should update correctly. 
```

```
### alpha4
Warning: Alpha build, UI not really fleshed out, might lock up, LEDs not fully implemented for Euclidean Mode
- Adds midiFX to MI mode
- Adds Make Mono MidiFX: stops polyphonic chords in their tracks, only one note can survive. 
```

```
### 1.8.1alpha
Warning: Alpha build, UI not really fleshed out, might lock up, LEDs not fully implemented for Euclidean Mode
- EuclideanMode: Adds Scaler MidiFX
```

```
### 1.8.0alpha
Warning: Alpha build, UI not really fleshed out, might lock up
- New euclidean sequencer mode
- MidiFX capabilities added
   - Chance MidiFX: This sets the probability that a note will play. 
   - Randomizer MidiFX: Randomized note number +-, octave +-, velocity +-, length, and chance that note is randomized. 

knobs change settings for active euclid sequencer
   - Knob 1: Rotation
   - Knob 2: Events
   - Knob 3: Steps
   - Knob 4: Gate
   - Knob 5: Sequence rate

Keys 1-4 change selected euclid sequencer. *NOTE* no LEDs for this ATM, screen will display square next to selected sequencer lane. 

Aux: Start / Stop playback. 

Encoder Push: Change between polymeter and polyrhtym mode, recommend pressing this first. 

## MIDIFX
Key 16 Lower: Enter MidiFX Mode

Aux: Exit MidiFX Mode

Keys 1-4: select FX slot. Params change based on selected midifx. 

Keys 9-16: select FX type for selected slot. 
```


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
