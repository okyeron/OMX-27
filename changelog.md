# Change log

```
### 1.12.7 alpha
### EEPROM VERSION 25
*** Warning!!!: This will clear your global save state if EEPROM version is different from your firmware***
- Pots now send CC's in chord mode. 
```

```
### 1.12.6 alpha
*** Warning!!!: This will clear your global save state***
- Improves chord mode
- Split UI Mode: Enabled by default
- Basic Chord Types
- - Each chord can be either basic or interval
- - Interval is like previous mode
- - Basic is a root note, chord type, octave, and balance
- - - Balance will change inversion of chord by either incrementing notes by octave, decrementing by an octave or playing all. 
- - - Also changes the velocity of notes that are not the root note. 
-
- Split UI:
- - Play normal keys on right side of keyboard
- - Play chords on the left
- - If the mode is in edit mode, you can quickly hold a basic chord and set the root note on the left. 
- - - Only happens while holding the chord. 
- - If in play mode, the right side does not modify the chord
-
- Each chord can have it's own Midi Channel and be set to a specific midi fx. 
- Holding aux changes the MidiFX slot for the split keyboard. 
- If you are wondering why the arpeggiator is not starting and stopping, it's because
- the chord you are playing is set to a different midifx index than what is selected in aux
- hold aux and select the midifx slot the chord is set to, then the arp hotkeys will work for
- that chord. 
-
- The split keyboard can also have it's own midi channel. 
-
- Cute animations
- 
- Hold F1 + Chord to enter chord edit mode. 
- This mode is different based on whether the chord is basic or an interval
- In basic mode you can play root chord notes and will see the notes of the chord light up. 
- 
- Default chord config is set to basic chords that are mapped with the root notes in C Major. 
-
- While holding aux or a chord, the selected parameter can be quickly edited to avoid having to click encoder. 
- - holding aux does the same in MI mode. 
```

```
### 1.12.5 alpha
*** Warning!!!: This will clear your global save state if FW < 1.12.2***
- Improves the UI of the MidiFX
- Jumps straight into midifx param view, gets rid of the confusion of two views
- Prevents accidentally changing the type of the selected midifx slot
- You now need to hold down the key for the slot in order to change the type
- New UI when holding down a midi fx slot.
- Holding down a midi fx slot and turning the encoder will move it around. 
- Just hold down a midifx slot key
- - You know you want to. 
- LED Colors for the midifx slots now better represent which midifx are in the slots. 
```

```
### 1.12.4 arp preview
*** Warning!!!: This will clear your global save state if FW < 1.12.2***
- Makes grids pots less grainy
- Grids pots now have high resolution. 
- Girds Pots will stick around mid-point 127
- Threshold needs to be met after loading grids settings to modify density values
- - This is to prevent the values from randomly jumping to value of current pot position
```

```
### 1.12.3 arp preview
*** Warning!!!: This will clear your global save state if FW < 1.12.2***
- Adds Aux hotkeys for controlling the arp
- Key 26: Toggles Arp On and Off
- Key 25: Toggles Hold On and Off
- Key 24: Changes the Arp Octave Range
- Key 23: Changes the Arp Pattern
- Key 22: Enters arp param view
- Arp Param view lets you change the arp parameters and also play the keys
- - Aux to go back to the normal params
- Same hotkeys for Chords Mode and MI modes
- If there is no arp in your current MidiFX slot and arp will be auto-added for you. 
```

```
### 1.12.2 arp preview
*** Warning!!!: This will clear your global save state***
- Saving and loading arpeggiator configs
- Copy pasting arpeggiators
- Arps have chance param now
- MidiFX and arps work in Chord Mode now
- - Use in same way as MI by holding aux key
- Arps work in euclidean mode
- Midifx like harmonizer and randomize in front of the arp can cause arp to get stuck on when used in MI and Chord modes.
- - Turn arp mode to off then back on if this happens. 
- - Recommend trying this with the euclidean sequencer. 
- - MidiFX after the arp are fun and won't cause arp to be stuck on
```

```
### 1.12.1 arp preview
*** Warning!!!: Might clear your global save state***
- Adds the arpeggiator MidiFX
- Arpeggiator currently only works in MI mode. 
- Enter MidiFX mode by holding AUX + MidiFX Mode Key
- Arps for MidiFX groups are currently always running, so if you have multiple arps with hold on
- Things can get weird. 
- Try adding MidiFX before and after arp for fun and profit!
```

```
### 1.11.7 alpha
*** Warning!!!: Will clear your global save state***
- Adds note lengths below 1 to the sequencer. 
- Noteoffs improvement to avoid stuck notes at fast bpms and small note lengths. 
- Reduced struct size of grids saves and euclid saves
```

```
### 1.11.6 alpha
*** Warning!!!: Will clear your global save state***
- Adds swing to grids sequencer
-
- Adds cut/paste step feature to S1 and S2
- - Selecting a step by holding it or F1 plus a step will copy it to a buffer
- - 
- - Hold F2:
- - - Select an unlit step to paste the buffer to that step
- - - Select a lit(unmuted) step to "Cut"(Copies and mutes)
- - - - Simply paste back to that step or unmute if you want to undo
- - 
- - This was inspired by my cut paste feature in GridStep for norns and makes it very quick to shuffle notes around! 
- -
- Fixes bugs by using F1 + step to enter step
- - Notes won't change
- - Works with multiple pages
```

```
### 1.11.5 alpha
*** Warning!!!: Will clear your global save state***
- Adds note lengths to grids sequencer
- Change via GATE param in Instrument view, configurable per instrument
- Fixes labels not always displaying correctly in grids mode
- Adds quickkeys for note length, BPM, and note number to Instrument view
```

```
### 1.11.4 alpha
*** Warning!!!: Will clear your global save state***
- Adds note length control
- Changes octave increment to a value
```

```
### 1.11.3 alpha
*** Warning!!!: Will clear your global save state***
- Adds encoder strum mode to Chords Mode
- Press C#2 to enter mode
- Hold chord and turn encoder
- Pots change Sensitivity, Wrap, and Increment
-
- Changed header format to not interfere with sysex
```

```
### 1.11.2 alpha
***Warning: Will clear your global save state***
- Adds ability to paste chords by holding F2 and selecting a destination
- Adds 3 modes to switch between using F#1,G#1,A#1
- Modes are: Play, Edit, Preset
- Adds 8 presets which save all 16 chord configurations to a slot. 
- Page one now displays the notes of a chord. 
- Press A#2 in Chord edit mode to quickjump to chord notes page. 
- Rotate parameter was not working correctly and did nothing. Now it will rotate and increase octave of rotated notes. 
```

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
