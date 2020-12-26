# OMX-27

<img src="buildpix/OMX-27-top.png" alt="Top" width="1080" height="244" />  
<img src="buildpix/OMX-27-bottom.png" alt="Bottom" width="1080" height="244" />  

### Before your start

The key-switches are going to be the VERY LAST thing you solder. After you solder the switches in, everything on the inside is going to be inaccessible.

Ideally you want to be able to test all the LEDs and OLED before putting the switches on.

I'd also suggest testing each switch connection with a piece of wire so you can confirm the diodes are soldered correctly.

### 100n and 10u capacitors  

Everything is 100n caps except for the two next to the encoder which are 10u

<img src="buildpix/OMX-27-build-caps.png" alt="Capacitors" width="1080" height="237" />

### Diodes

The stripes (cathode pin) on each diode are all facing towards the top of the PCB. The one oddball (top left key) has its stripe facing to the outside of the board.
<img src="buildpix/OMX-27-build-diodes.png" alt="Diodes" width="1080" height="237" />

### Resistors
<img src="buildpix/OMX-27-build-resistors.png" alt="Resistors" width="1080" height="237" />

### IC

U1 - the TLV9062 op-amp here is oriented with pin 1 at the bottom right side. Extra large white dot added here in picture for emphasis.  
<img src="buildpix/OMX-27-ic.png" alt="IC" width="1080" height="237" />

### LEDs

The LEDs are __Reverse Mount__ and are soldered to the back-side of the PCB with the LED facing towards the top of the PCB. When looking at the back of the PCB as in the picture, the GND leg is the top right pad for each one (marked with a red triangle in the picture below). The LED itself has a "notched" leg for GND.

<img src="buildpix/OMX-27-build-leds.png" alt="LEDs" width="1080" height="237" />


### TEENSY

For the keyplate to fit properly, the Teensy MUST be flush-mounted to the top of the main PCB.

Before soldering - Use a piece of insulating tape (Electrical, Kapton if you have it) to cover all the contacts on the bottom-middle of the Teensy (to prevent accidental shorts). (PIC NEEDED) 

(NEED PICS OF ATTCHMENT/SOLDERING TECHNIQUE HERE)


### JACKS, OLED, POTS, ENCODER, ETC.

Figure it out. :)


### KEY SWITCHES

Snap all the key-switches into the keyplate (from the top). Then solder.