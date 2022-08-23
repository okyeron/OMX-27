"Was messing around with the Grids mode, and thought it would be nice if the four buttons that blink showing the different 'hits' could also act as mutes."

- Move MidiFX easily rather than cut/paste

- Quickkeys for arps. 

- Way to adjust midifx and play keyboard at same time. 

"feedback dialogs when selecting FXgroup or FX-off might be nice"







- S1/S2 - Active step mode
    This would add an additional view in which steps can be turned on or off. 
    Steps that are off get ignored as if they did not exist. 
    Allows for fun performance manipulation of sequence without changing it. 

- S1/S2 pattern gate feature

- Arpegiator

- Chord Split Mode

- Add MidiFX to Seq, Chord, and Grids Modes

- Chord LED feedback

- Auto strum

- Earthsea sequencer, add into midi and chord modes, simple records what you play then plays back in natural time or quantized. 

- Pot CC Presets
    These would be templates for quickly setting cc's to work with a specific synth like a mm2 or digitone. 

    Might be easier to just have multiple banks of pot presets. 

    A key shortcut for changing banks would be nice. 


- Try and make UI more consistent between modes

- Show something on screen to indicate you're in a submode and can press aux to exit. 

- Save single mode to sysex and load from sysex

- Add midi channels to chords

- Default chords to something playable. 

- Full manual chord note input






Waldorf Arp notes


The arpeggiator uses a so-called note list that can store up to 16 notes. 

Sort Order is set to Num Lo>Hi, the list is rearranged so that the lowest note is placed at the first position, the second lowest note at the next


Mode
	off
	on
	one shot
	hold
	
Step Len
	if Length is set to legato, all arpeggio notes are played without pauses between each step and Arp Steplen therefore has no effect.
	
	
Range
	octaves
	
Patterns


x-xxx-xxx-xxx-xx
x-x-x--xx-x-x--x
x-x-x-xxx-x-x-xx
x-xxx-x-x-xxx-x-
x-x-xx-xx-x-xx-x
xx-x-xx-xx-x-xx-
x-x-x-x-xx-x-x-x
x-x-x-xx-x-xx-x-
xxx-xxx-xxx-xxx-
xx-xx-xx-xx-xxx-
xx-xx-xx-xx-x-x-
xx-xx-x-xx-xx-x-
x-x-x-x-xx-x-xxx
x--x--x--x--x--x
x-x-x-x-x--xx-x-

Max Notes

Step length

Direction

	up
	down
	alt up
	alt down
	
Sort Order
	as played 
	reversed 
	Num Lo>Hi 
	Num Hi>Lo 
	Vel Lo>Hi 
	Vel Hi>Lo
	
Velocity
	randomize like grids?
	
Swing?

Same Note Overlap

Pattern Reset
	With Pattern Reset, you can decide if the note list is also restarted from the beginning when the rhythm pattern is reset.
	
	If Off is selected, the note list is not restarted, so that there is no synchronization between rhythm and note list. E.g., when you have a pattern where four steps are set and you play three notes, the pattern and the note list are repeated differently.
	
	If On is selected, the note list will be restarted as soon as the rhythm pattern is restarted.
	

Arpeggiator Edit Menu Step Data
	
Arp Accent 

Arp Glide

Arp Step

	• If * is selected (asterisk symbol), the Arpeggiator plays the step unaltered. The note list is advanced beforehand, except when you press a new chord.

	• If `off` is selected (empty space), the Arpeggiator plays nothing at this step position. When Length or Steplen is set to legato, the previous step that isn’t set to Off is still held to create the legato effect. The note list is not advanced.

	• If - is selected, the Arpeggiator plays the same note as it had to play in the previous step that was set to * or ˆ. With this setting, you can repeat a particular note of the note list several times. The note list is not advanced.

	• If < is selected, the Arpeggiator plays the very first note of the note list. This might be interesting if you want to only play the "root note" of a chord in a bass sound. The note list is not advanced.

	• If > is selected, the Arpeggiator plays the very last note of the note list. The note list is not advanced.

	• If <> is selected, the Arpeggiator plays a chord with two notes, the first and the last one of the note list. This means that you have to play at least two notes to hear the effect. Otherwise, you would hear only one note anyway. The note list is not advanced.

	• If (notes) is selected (notes symbol), the Arpeggiator plays a chord with all notes from the note list. This means that you have to play at least two notes to hear the effect. The note list is not advanced.

	• If ? is selected, the Arpeggiator plays a random note from the note list. This doesn’t mean that it creates any random note, it only uses one note of the note list at will. The note list is not advanced.




