# OMX-27 v3 BOM

OMX-27 v3 comes with SMD parts pre-assembled. Some alternate/replacement parts are listed here for reference 

| Mouser  | QTY | Part | Value | Package |
|-----|:--:|-----|-----|-----|
|[688-RK09K1130A5R](http://www.mouser.com/Search/ProductDetail.aspx?R=688-RK09K1130A5R)|5|VR1-VR4,VR6|10K linear |9MM_SNAP-IN_POT*|
|[652-PEC11R-4015F-S24](http://www.mouser.com/Search/ProductDetail.aspx?R=652-PEC11R-4015F-S24)|1|VR5|PEC11 + SWITCH|Encoder with Switch|
|[540-MX3A-L1NA](http://www.mouser.com/Search/ProductDetail.aspx?R=540-MX3A-L1NA)|27|K1-K27|CHERRY-MX|CHERRY-MX RGB Silent Red \***|
| (alt) |27|K1-K27|Kailh Choc V1|Kailh Choc V1 Red \***|
|  |1| |OLED - 128x32 I2C display| \**See below|

\* POTS - I used trimmer type pots because they're a little more low profile. But you can use alpha pots (10K linear) or whatever you have around.

\** OLED - 128x32 I2C display (SSD1306) with pin order ( GND, VCC, SCL, SDA )  
example search terms:  "0.91" 128x32 IIC I2C White OLED LCD Display DIY Module For Arduino"  

---
### Optional battery

A lipo battery can be added. 

[This battery from tinycircuits is recommended](https://tinycircuits.com/products/lithium-ion-polymer-battery-3-7v-290mah). Also available from [Mouser](https://www.mouser.com/ProductDetail/TinyCircuits/ASR00007) or [Digikey](https://www.digikey.com/en/products/detail/tinycircuits/ASR00007/7404517)

The battery plug on the PCB is a JST SH connector (2 pin, 1.0 mm pitch) or you can solder the battery wires directly to the PCB. Please note the PCB markings for polarity.

A 3D printable battery box is also available. [Model link]

---
### \*** Key switches: 

Cherry MX or Kailh Choc V1 switches can be used for the build. Denki-oto kits default to Cherry MX RGB Silent Red or Kailh Choc V1 Red switches and will have a different keyplate and spacer setup depending on the switch choice.

Switches can be best obtained from various online keyboard shops like [mechanicalkeyboards.com](https://mechanicalkeyboards.com/).  


### Keycaps - Choc: 

Choc Kits come with FK MBK "Dot Glow" keycaps. 

Nice blank Choc keycaps with shine-thru are hard to find. A translucent white MBK keycap made with POM material looks great with backlighting.  Search online keyboard shops in your country for "MBK POM"

FK offers a [Custom Service](https://fkcaps.com/custom/) if you wanted to create your own. 

---

### Keycaps - MX: 

Any MX-compatible keycaps will work, but you'll want one designed for backlighting, such as a "backlit two-shot", "translucent", "shine-through", or "windowed".  

I like the DSA profile caps for this application.

I have the DSA windowed keycaps in various colors available for sale in sets [on my shop](https://www.denki-oto.com/store/p62/OMX-27_Keycap_sets.html#/).

Signature Plastics sells the keycap I use in a Light or Dark grey - [DSA "Dolch" Keyset (Two Shot) "Windowed" Keys](https://spkeyboards.com/products/dsa-dolch) (choose the __LED Keys__ option).  
These come in a pack of 4 keycaps. You will need 7 packs (@ ~$60 total).  

[Flashquark Translucent DSA Keycaps](https://flashquark.com/product/translucent-dsa-keycaps/) (in Black, White, Clear, Blue, Red) - __50 per pack__. ($12.99-$15.99 per pack)

[Blue Hat 1U DSA Blank Printed Keycaps PBT Keycaps](https://www.amazon.com/gp/product/B07SJKMNWC) (Gray Translucent) - __37 per pack__. ($15.55 per pack and prime shipping)  
(about the same as the black ones from Flashquark above)

[Maxkeyboard Black Translucent MX Blank](https://www.maxkeyboard.com/black-translucent-cherry-mx-blank-keycap-set-for-esc-w-a-s-d-or-e-s-d-f-and-arrow-keys.html) (pack of 9). ($21 for 3 packs)

---

### Knobs

So many opinions about knobs. 

Micro knobs (Befaco Style) can be found at these places:  
https://www.thonk.co.uk/shop/tall-trimmer-toppers/   
https://www.thonk.co.uk/shop/micro-knobs/  
https://www.thonk.co.uk/shop/tall-trimmer-toppers/

---

### USB Cable: 

I recommend using a right angle extension cable [like this one from Amazon](https://www.amazon.com/gp/product/B015PSU5F6/)  

Be sure you have a good, known working, __USB DATA CABLE__ and not just a charging cable.
