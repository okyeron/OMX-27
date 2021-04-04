# OMX-27

<img src="buildpix/OMX-27-top.png" alt="Top" width="1080" height="244" />  
<img src="buildpix/OMX-27-bottom.png" alt="Bottom" width="1080" height="244" />  

# Before you start

## READ THIS ENTIRE GUIDE FIRST

Also - see these __Build Videos:__

[Part 1 - LEDs](https://youtu.be/UFm8Dfpjoz4)   
[Part 2 - Teensy](https://youtu.be/W-rJqxFzsLw):   
[Part 3- Pots and Testing](https://youtu.be/rtUBW4xm9us):   
[Part 4 - Switches and Assembly](https://youtu.be/jUWWuaacoz4):   

The key-switches are going to be the VERY LAST thing you solder. __After you solder the switches in, everything on the inside is going to be inaccessible.__

Ideally you want to be able to test all the LEDs, the OLED, and the pots/encoder before putting the switches on.

I'd also suggest testing each switch connection with a piece of wire or tweezers so you can confirm the diodes/LEDs/caps are all soldered correctly.

Follow the order of operations here to make your life easier. __NOTE - the keyswitches are  absolutely the last thing you solder.__ Make sure everything else looks good before you do the switches.

Also important - Keyswitches are snapped into the keyplate first (before soldering them). 

Don't forget to put the spacer layer in-between the main PCB and the keyplate before you solder all the switches.


### Soldering Tips

I work with a fine point tip on my iron at 400C. With this setup I typically hold the iron on a pad for about 2 seconds and then apply a bit of solder and then hold the iron there for anything 2-3 seconds. You want to watch for the solder to flow around the joint, but not to hold the iron there forever.

See [Adafruit's guide to excellent soldering](https://learn.adafruit.com/adafruit-guide-excellent-soldering) for lots of good tips and tricks.

Nice to have tools:
 - flush diagonal cutters  
 - tweezers  
 

---

# Build from Kit

### LEDs

The LEDs are __Reverse Mount__ and are soldered to the back-side of the PCB with the LED facing towards the top of the PCB. When looking at the back of the PCB as in the picture, the GND leg is the top right pad for each one (marked with a red triangle in the picture below). The LED itself has a "notched" leg for GND.

<img src="buildpix/OMX-27-build-leds.png" alt="LEDs" width="1080" height="237" />

Set each LED into position (tweezers are handy for this) and __then double check the ground pin is in the right position__.  

<img src="buildpix/leds3.png" alt="LEDs" width="360" height="275" />
<img src="buildpix/leds1.png" alt="LEDs" width="360" height="275" />
<img src="buildpix/leds2.png" alt="LEDs" width="360" height="275" />


Solder/tack the bottom right corner pad of each LED to hold each one in place. Then check the orientation of each LED to be sure they're nice and square in the hole. If not, warm up the solder there and reposition as needed.

After you're happy with the LEDs being in the proper positions - solder the rest of the pads.


### TEENSY

For the keyplate to fit properly, the Teensy MUST be flush-mounted to the top of the main PCB.

An insulating kapton spacer is included with your kit. Use this between the bottom of the teensy and the main PCB to reduce the chances of unintended shorts.

__Teensy jig__

Use the included acrylic jig to set up your teensy like the following for soldering.  

Short side of the headers goes down to the jig and the long side up.  

<img src="buildpix/teensy_jig_1.jpg" alt="LEDs" width="720" height="416" />

Add a 1x3 and 1x1 in the appropriate places. The 1x1 directly next to the 1x3 is not connected to anything so you can solder that or not (your choice).  

<img src="buildpix/teensy_jig_2.jpg" alt="LEDs" width="720" height="416" />
<img src="buildpix/teensy_jig_3.jpg" alt="LEDs" width="720" height="416" />

Add the two spacers  (maybe even tape those two together so they don't wiggle around.

<img src="buildpix/teensy_jig_4.jpg" alt="LEDs" width="720" height="416" />

Drop the Teensy into place. There should just be a small amount of header sticking up from the Teensy at this point.  

<img src="buildpix/teensy_jig_5.jpg" alt="LEDs" width="720" height="416" />

DON'T SOLDER A HEADER TO THE VUSB PIN - it's not used. This is the 1x1 pin/hole right next to the USB jack on the Teensy (on the inside row).

(PIC NEEDED) 

Solder the pins to the Teensy first.

Then remove the jig and carefully remove the black plastic from the headers. __Hold onto the black spacers for the next step.__  

After you've removed the plastic, slide the thin yellow kapton spacer thingy onto the bottom of the teensy - this should end up between the teensy and the main board as an insulator. Then drop the Teensy onto the main board so it sits nice and flat. 

To keep the pins from wiggling around while soldering the bottom, either 

 * Put a big piece of tape over the whole teensy to keep it in place and to keep the pins from getting pushed out
 
 * Or push the black pastic bits from the headers onto the pins to hold them in place while soldering
 
 * Or both

Flip the board over and solder the pins to the bottom. Try to tack/solder one pin on either side in place while pushing your finger against the teensy to make sure it's absolutely flat against the main pcb.

Once you're happy with the flatness - solder the rest of the pins. Be careful not to push down on the pins while soldering.

Using flush cutters, trim the pins away. Be carefull not to nick/scratch the pcb.


### OLED

The OLED display sits on a regular header (not flush like the Teensy)> the display should be close to level with the keyplate (the OLED glass will be about 0.5-1mm higher than the keyplate).

__TIP:__ I suggest using a section of the header plastic you removed from the Teensy headers as a spacer to hold up the other side of the OLED PCB. Glue or tape a 1x4 chunk of the header plastic to the back of the OLED pcb and this will keep it level and support it while you solder (and after).

Trim the headers on the top side of the OLED if you're worried about something shorting there.


### JACKS, POTS, ENCODER, ETC.

Snap pots and encoders into place and solder.

You may need to gently squeeze the snap-in mounting pins together a tiny bit to get the pots to snap into place.


# STOP HERE AND TEST THINGS

At this point you can flash the firmware and do some testing. The OLED should display something as soon as you plug into USB power.

### LED test

On startup all the LEDs should show a rainbow pattern.

If your LEDs work up to a certain point (e.g. LEDs 1-7 work, LED 8-27 don't):

- The problem is most likely a bad soldering joint on the erroneous LED itself, or on the LED that is RIGHT BEFORE this LED in the chain (in the above example, check LED 7 and 8). Carefully re-solder all connections again to fix the problem (melt the existing solder again, maybe apply some more, make sure it flows nicely between LED and PCB pad)

- Check that the orientation of the LED is correct (see pictures above)


### Switch contact test

You will want to test the pads for each keyswitch on the PCB using tweezers or a piece of wire. This is also a second check that the LED for that switch is working correctly.

If not working here, check the LEDs again first. If all the LEDs are working OK look to be sure the diode adjacent to that switch position is OK.

(PIC NEEDED) 


### MIDI test

Use the [browser_test](browser_test/index.html) script to show USB-MIDI input to your computer. Then you can check to be sure the pots are sending CCs and that you get MIDI note-ons/note-offs when you test each keyswitch's pads. Be sure you have the `oct` (octave) set to 4 on the display (change with encoder knob).

Also test the Hardware MIDI 1/8" jack with an appropriate adapter and synth. Check the A/B switch position for your particular setup (try both to be sure you have the right one).

---

# Continue building

### Acrylic Case Parts

__Carefully__ remove the paper backing from the acrylic parts - the spacer and the back plate. Then set these aside for the next step. 

The spacer layer is pretty fragile - try not to break it. However, even if it does break, it might be fine since this sits in-between the other layers.


### KEY SWITCHES

Check the orientation of the switches. The pins go towards the bottom-half and the LED window at the top.  

Snap all the key-switches into the keyplate (from the top). 

(PIC NEEDED)  

The switches may be a tight fit. Be sure they are snapped all the way into place.

(PIC NEEDED)  

Set the black acrylic spacer layer on the main PCB and align it around the various components. Then set the keyplate with switches into place to be sure all the pins line up and everything is nice and flat. You may need to gently bend key-switch pins into place if they got slightly bent in transport.

Use the included case screws/nuts to fix everything together for soldering. I suggest using the holes down the middle of the case. This will ensure the key switches are held in place for soldering and that everything will remain flat.

Solder all the switches.

### Bottom Plate

Then remove the screws/nuts and then reassemble with the back plate.

The nuts fit into the captive cutouts on the bottom plate.

(PIC NEEDED)  
