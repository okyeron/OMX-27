# OMX-27 version 3

![Build Status](https://github.com/okyeron/OMX-27/actions/workflows/platformio-ci.yml/badge.svg)

OMX-27 is a compact DIY hardware MIDI controller and sequencer with RGB LED backlit mechanical key switches.

Version 3 of the hardware is based on the RP2040 microprocessor

Kits and specs are [available here](https://www.denki-oto.com/).

Information and code for earlier heardware versions is included in the [Archive](Archive/ReadMe.md) directory.

Dimensions:  313mm x 65mm  

## Firmware 

Kits are shipped with the current firmware already flashed.

The OMX-27 firmware is Open Source and current uses PlatformIO (Arduino)

See [below](<#Firmware-Development>) to compile the firmware yourself.  

## Build

[Build Guide](<build/Build-Kit.md>)

## Docs

[Documentation](<Docs.md>)

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
git checkout https://github.com/okyeron/OMX-27.git

# go to the project directory
cd OMX-27-RP2040

# compile the project (this may take a while the first time)
pio run

# upload to hardware (press reset and boot and release reset before boot)
pio run -t upload


```

(optional) Install PlatformIO IDE VSCode extension. [Instructions](https://platformio.org/platformio-ide)

Install EditorConfig extension for your text editor. [Instructions](https://editorconfig.org/)
 
Note: when making changes using the PlatformIO toolchain, please ensure the sketch still builds on Teensyduino before opening a PR.


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
