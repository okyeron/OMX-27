# OMX Sysex spec

The OMX interfaces with its editor via MIDI Sysex. This document describes the supported messages.

_Work in progress, porting from 16n faderbank editor_

## `0x1F` - "1nFo"

Request for OMX to transmit current state via sysex. No other payload.

## `0x0F` - "c0nFig"

"Here is my current config." Only sent by OMX as an outbound message, in response to `0x1F`. Payload of 80 bytes, describing current EEPROM state.

## `0x0E` - "c0nfig Edit"

"Here is a new complete configuration for you". Payload (other than mfg header, top/tail, etc) of 80 bytes to go straight into EEPROM, according to the memory map described in `README.md`.

## `0x0D` - "c0nfig edit (Device options)"

"Here is a new set of device options for you". Payload (other than mfg header, top/tail, etc) of 16 bytes to go straight into appropriate locations of EEPROM, according to the memory map described in `README.md`.

## `0x0C` - "c0nfig edit (usb options)"

"Here is a new set of USB options for you". Payload (other than mfg header, top/tail, etc) of 32 bytes to go straight into appropriate locations of EEPROM, according to the memory map described in `README.md`.

## `0x0B` - "c0nfig edit (trs options)"

"Here is a new set of TRS options for you". Payload (other than mfg header, top/tail, etc) of 32 bytes to go straight into appropriate locations of EEPROM, according to the memory map described in `README.md`.
