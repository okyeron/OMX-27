# OMX-27 version 3

![Build Status](https://github.com/okyeron/OMX-27/actions/workflows/platformio-ci.yml/badge.svg)

OMX-27 is a compact DIY hardware MIDI controller and sequencer with RGB LED backlit mechanical key switches.

Version 3 of the hardware is based on the RP2040 microprocessor (Raspberry Pi Pico).

The firmware also supports earlier hardware versions:
- **OMX-27 v2**: Teensy 4.0
- **OMX-27 v1**: Teensy 3.2/3.1

All three platforms share a unified codebase with platform-specific optimizations.

Kits and specs are [available here](https://www.denki-oto.com/).

Dimensions:  313mm x 65mm  

## Docs

[New Documentation](https://okyeron.github.io/OMX-27/docs/)

[Old Documentation](<Docs.md>)

## Firmware 

Kits are shipped with the current firmware already flashed.

The OMX-27 firmware is Open Source and uses PlatformIO (Arduino framework)

The firmware supports multiple hardware platforms (RP2040/Pico, Teensy 4.0, Teensy 3.2/3.1).

See [below](<#Firmware-Development>) to compile the firmware yourself.  

## Build

[Build Guide](<build/Build-Kit.md>)


## Web Configurator

[Online Configurator](https://okyeron.github.io/OMX-27/webconfig/index.html)

## BOM

OMX-27 v3 comes with SMD parts pre-assembled. Some alternate thru-hole parts are listed in the [Bill of Materials](<build/BOM.md>) for reference 



## Firmware Development  

Currently the firmware is best worked on with PlatformIO and VSCode. This may change in future.

### PlatformIO / VSCode

Ensure Homebrew in installed. [Instructions](https://brew.sh/)  
Install PlatformIO CLI tools. [Detailed Instructions](https://platformio.org/install/cli)

```sh
# Mac OSX
brew install platformio

# check out the project
git clone https://github.com/okyeron/OMX-27.git

# go to the project directory
cd OMX-27

# compile for default platform (RP2040/Pico)
pio run

# compile for specific platform
pio run -e pico      # RP2040/Pico (v3)
pio run -e teensy40  # Teensy 4.0 (v2)
pio run -e teensy31  # Teensy 3.2/3.1 (v1)

# compile for all platforms
pio run -e pico -e teensy40 -e teensy31

# upload to hardware
## For RP2040/Pico: Press RESET and BOOT buttons, release RESET before releasing BOOT
pio run -e pico -t upload

## For Teensy: Press the button on the Teensy board
pio run -e teensy40 -t upload
pio run -e teensy31 -t upload
```

(optional) Install PlatformIO IDE VSCode extension. [Instructions](https://platformio.org/platformio-ide)

Install EditorConfig extension for your text editor. [Instructions](https://editorconfig.org/)
 
To open the project in VSCode :
- open a new window
- select the PlatformIO icon from the Primary Side Bar (left toolbar)
- use the "Pick a folder" button to select the OMX-27 folder you created above

## Clear Storage Utility

The `clear_storage` directory contains a utility sketch to reset the device's FRAM or EEPROM storage to factory defaults. This is useful when:
- Upgrading between firmware versions with incompatible storage formats
- Troubleshooting configuration issues
- Starting fresh with default settings

Pre-compiled hex files are available for Teensy platforms:
- `clear_storage.T32.hex` - For Teensy 3.2/3.1
- `clear_storage.T4.hex` - For Teensy 4.0

To use: Flash the appropriate hex file using TyUpdater (same process as main firmware). The device will automatically detect whether you have FRAM or EEPROM and clear it accordingly. Progress is shown on the OLED display.

After clearing storage, re-flash the main OMX-27 firmware.

## FAQ

Q: What key switches are recommended?  
A: The board uses a dual-footprint for either Cherry MX or Kailh Choc V1 switches. 

Q: Can I use other key switches?  
A: Yes - as long as they have the same footprint as Cherry MX or Kailh Choc V1 switches and a window/opening for the LED to shine through. NOTE - __Cherry Low Profile__ or__ Kailh Choc V2__ switches have a different footprint and will not work.

Q: What about recommended Keycaps if I want to customize?  
A: Also listed in the [BOM](<build/BOM.md>). It depends on which switches you use. You want a "shine thru" cap with a window for the LED.

Q: Does this project require soldering?  
A: Yes. Thru-hole soldering is required (Pots and switches).  

Q: Can I get the Gerbers or order the pcbs myself?  
A: No. Not open source at this time.

Q: Can I get some of those windowed keycaps you're using?  
A: Yes (send me an email).
