# OMX Sysex spec

The OMX-27 interfaces with its editor via MIDI Sysex. This document describes the supported messages.

_Work in progress, porting from 16n faderbank editor_

## `0x1F` - "1nFo"

Request for OMX-27 to transmit current state via sysex. No other payload.

## `0x0F` - "c0nFig"

"Here is my current config." Only sent by OMX-27 as an outbound message, in response to `0x1F`. Payload of 32 bytes, describing current EEPROM state.

## `0x0E` - "c0nfig Edit"

~~"Here is a new complete configuration for you". Payload (other than mfg header, top/tail, etc) of 80 bytes to go straight into EEPROM, according to the memory map described in `README.md`.~~ not implemented

## `0x0D` - "c0nfig edit (Device options)"

"Here is a new set of device options for you". Payload (other than mfg header, top/tail, etc) of 32 bytes to go straight into appropriate locations of EEPROM, according to the following map:
```
## SYSEX Map of config data - 90 bytes

	Status Byte `F0` (SYSEX Start) (1 byte)  
	Manufacturer ID `7D 00 00` (3 bytes)  

	Command (1 byte)  
	Modelnum (1 byte)  
	Version (3 bytes)   

	0 - EEPROM VERSION (1 byte)  
	1 - Current MODE (1 byte)  
	2 - Sequencer PlayingPattern (1 byte)  
	3 - MIDI mode MidiChannel (1 byte)  
	4 - 28 - Pots (x25 - 5 banks of 5 pots) (25 bytes)  

	29 - MIDI Macro Channel (1 byte)
	30 - MIDI Macro Type (1 byte)
	31 - Scale Root (1 byte)
	32 - Scale Pattern, -1 for chromatic (1 byte)
	33 - Lock Scale - Bool (1 byte)
	34 - Scale Group 16 - Bool (1 byte)
	35 - 63 - Not yet used
```
Example: 
`F0 7D 00 00 0D 09 00 00 00 15 16 17 18 07 1D 1E 1F 20 21 22 23 24 25 26 27 28 29 2A 2B 5B 5D 67 68 69 00 00 00 F7`

## `0x0C` - "c0nfig edit (usb options)"

~~"Here is a new set of USB options for you". Payload (other than mfg header, top/tail, etc) of 32 bytes to go straight into appropriate locations of EEPROM, according to the memory map described in `README.md`.~~ not implemented

## `0x0B` - "c0nfig edit (trs options)"

~~"Here is a new set of TRS options for you". Payload (other than mfg header, top/tail, etc) of 32 bytes to go straight into appropriate locations of EEPROM, according to the memory map described in `README.md`.~~ not implemented
