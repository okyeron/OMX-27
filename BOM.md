# OMX-27 BOM


| Mouser  | QTY | Part | Value | Package |
|-----|:--:|-----|-----|-----|
|[RC0805FR-1047RL](http://www.mouser.com/Search/ProductDetail.aspx?R=RC0805FR-1047RL)|2|R7 R8|47R|0805|
|[603-RC0805FR-0710KL](http://www.mouser.com/Search/ProductDetail.aspx?R=603-RC0805FR-0710KL)|2|R1 R2|10K|0805|
|[652-CR0805-FX2202ELF](http://www.mouser.com/Search/ProductDetail.aspx?R=652-CR0805-FX2202ELF)|2|R4 R6|22K|0805|
|[RC0805FR-1056KL](http://www.mouser.com/Search/ProductDetail.aspx?R=RC0805FR-1056KL)|2|R3 R5|56K|0805|
|[80-C0805C104J5RACLR](http://www.mouser.com/Search/ProductDetail.aspx?R=80-C0805C104J5RACLR)|28|C1-C29|100nF|0805|
|[710-885382207006](http://www.mouser.com/Search/ProductDetail.aspx?R=710-885382207006)|2|C30, C31|10nF|0805|
|[621-1N4148W-F](http://www.mouser.com/Search/ProductDetail.aspx?R=621-1N4148W-F)|27|D1-D27|1N4148 Diode|SOD-123|
|[SJ-3523-SMT-TR](http://www.mouser.com/Search/ProductDetail.aspx?R=SJ-3523-SMT-TR)|1|J1|SJ-3523-SMT-TR|3.5 mm jack stereo|
|[490-MJ-3523-SMT-TR](http://www.mouser.com/Search/ProductDetail.aspx?R=490-MJ-3523-SMT-TR)|2|J2,J3|MJ-3523-SMT|3.5 mm jack mono|
|[540-MX3A-L1NA](http://www.mouser.com/Search/ProductDetail.aspx?R=540-MX3A-L1NA)|27|K1-K27|CHERRY-MX|CHERRY-MX RGB Silent Red \*|
|[595-TLV9062IDR](http://www.mouser.com/Search/ProductDetail.aspx?R=595-TLV9062IDR)|1|U1|SOIC127P600X175-8N|TLV9062IDR|
|[AYZ0202AGRLC](http://www.mouser.com/Search/ProductDetail.aspx?R=AYZ0202AGRLC)|1|S1|DPDT Switch|SWITCH-DPDT-SMD-AYZ0202|
|[688-RK09K1130A5R](http://www.mouser.com/Search/ProductDetail.aspx?R=688-RK09K1130A5R)|5|VR1-VR4,VR6|10K|9MM_SNAP-IN_POT*|
|[652-PEC11R-4015F-S24](http://www.mouser.com/Search/ProductDetail.aspx?R=652-PEC11R-4015F-S24)|1|VR5|PEC11+SWITCH|Encoder with Switch|
| [aliexpress](https://www.aliexpress.com/item/4000475685852.html?spm=a2g0s.9042311.0.0.601b4c4dcyhOZn) / [ebay](https://www.ebay.com/itm/100-2000pcs-SK6812-MINI-E-LED-CHIP-SK6812-3228-4pin-dream-color-LEDS-DC5V/224140435419?hash=item342fcf9fdb:g:XbAAAOSwzkRd8g96)|27|LED1-LED27|SK6812MINIE|SK6812-MINI-E|
| [PJRC Store](https://www.pjrc.com/store/teensy32.html) |1| |TEENSY 3.2||
|  |1| |OLED - 128x32 I2C display| \**See below|
|  | | |header pins| \***See below|

\* POTS - I used trimmer type pots because they're a little more low profile. But you can use alpha pots or whatever you have around.

\** OLED - 128x32 I2C display (SSD1306) with pin order ( GND, VCC, SCL, SDA )  
example from eBay:  
"0.91" 128x32 IIC I2C White OLED LCD Display DIY Module For Arduino"  
https://www.ebay.com/itm/293660021494  

\*** Headers:
1X04 (oled) 
1x14 x 2 (teensy) 
1x01 (teensy dac pin) 

TIP: Get 1x40 breakaway headers and cut what you need.  

[Mouser Cart (work in progress)](https://www.mouser.com/ProjectManager/ProjectDetail.aspx?AccessID=13c0107d30) - __DOES NOT include Teensy, OLED, LEDs or headers__

Mounting Hardware is NOT LISTED

Knobs are up to you.

---
### \* Key switches: 

Any of the Cherry MX RGB switches will work. Red/SilentRed/Blue/Brown/Black/SpeedSilver.  
Red is linear (45g). Blue is clicky & tactile (50g). Brown is tactile (45g). Black is similar to Red but 60g actuation force. SpeedSilver (shorter key travel for gamers) are linear and __Expensive__( 45g)

Cherry MX RGB part numbers:  

| name  | part num | type | actuation |
|-----|----|-----|----|
|SilentRed |[MX3A-L1NA](https://www.mouser.com/ProductDetail/CHERRY/MX3A-L1NA/?qs=F5EMLAvA7IA6PAS7ry3I9w%3D%3D)| linear | (45g) |
|Red	|MX1A-L1NA| linear | (45g) |
|SilentBlack |[MX3A-11NA](https://www.mouser.com/ProductDetail/CHERRY/MX3A-11NA/?qs=F5EMLAvA7ICizK1XKjfN9w%3D%3D)| linear | (60g) |
|Black	|MX1A-11NA| linear | (60g) |
|Brown	|[MX1A-G1NA](https://www.mouser.com/ProductDetail/540-MX1A-G1NA/)| tactile | (45g) |
|Blue	|MX1A-E1NA| clicky & tactile | (50g) |
|Silver	|[MX1A-51NA](https://www.mouser.com/ProductDetail/CHERRY/MX1A-51NA/?qs=F5EMLAvA7IB4ByA0zXdBkg%3D%3D)| linear | (45g) |

Reference: https://www.mouser.com/pdfDocs/cherrykeyswitches.pdf


### Keycaps: 

Any MX-compatible keycaps will work, but you'll want one designed for backlighting, such as a "backlit two-shot", "translucent", "shine-through", or "windowed".  

I like the DSA profile caps for this application.

[DSA "Dolch" Keyset (Two Shot) "Windowed" Keys](https://pimpmykeyboard.com/dsa-dolch-keyset-two-shot/) (choose the __LED Kit__ option).  
These come in a pack of 4 keycaps. You will need 7 packs (@ ~$42 total).  

[Flashquark Translucent DSA Keycaps](https://flashquark.com/product/translucent-dsa-keycaps/) (in Black, White, Clear, Blue, Red) - __50 per pack__. ($12.99-$15.99 per pack)

[Blue Hat 1U DSA Blank Printed Keycaps PBT Keycaps](https://www.amazon.com/gp/product/B07SJKMNWC) (Gray Translucent) - __37 per pack__. ($15.55 per pack and prime shipping)  
(about the same as the black ones from Flashquark above)

[Maxkeyboard Black Translucent MX Blank](https://www.maxkeyboard.com/black-translucent-cherry-mx-blank-keycap-set-for-esc-w-a-s-d-or-e-s-d-f-and-arrow-keys.html) (pack of 9). ($21 for 3 packs)

### Knobs

So many opinions about knobs. 

Micro knobs (Befaco Style) can be found at these places:  
https://www.thonk.co.uk/shop/tall-trimmer-toppers/   
https://www.thonk.co.uk/shop/micro-knobs/  
https://www.thonk.co.uk/shop/tall-trimmer-toppers/


### USB Cable: 

I recommend using a right angle extension cable [like this one from Amazon](https://www.amazon.com/gp/product/B015PSU5F6/)  

Be sure you have a good, known working, USB DATA CABLE and not just a charging cable.
