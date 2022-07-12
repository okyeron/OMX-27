#include <U8g2_for_Adafruit_GFX.h>

#include "omx_disp.h"
#include "consts.h"
#include "ClearUI.h"
#include "omx_midi_settings.h"

U8G2_FOR_ADAFRUIT_GFX u8g2_display;

// Constructor
OmxDisp::OmxDisp()
{
}

void OmxDisp::setup() {
    initializeDisplay();
	u8g2_display.begin(display);
}

void OmxDisp::clearDisplay(){
    // Clear display
	display.display();
	dirtyDisplay = true;
}


void OmxDisp::drawStartupScreen(){
    display.clearDisplay();
	testdrawrect();
	delay(200);
	display.clearDisplay();
	u8g2_display.setForegroundColor(WHITE);
	u8g2_display.setBackgroundColor(BLACK);
	drawLoading();
}

void OmxDisp::displayMessage(const char* msg) {
    display.fillRect(0, 0, 128, 32, BLACK);
    u8g2_display.setFontMode(1);
    u8g2_display.setFont(FONT_TENFAT);
    u8g2_display.setForegroundColor(WHITE);
    u8g2_display.setBackgroundColor(BLACK);
    u8g2centerText(msg, 0, 10, 128, 32);

    messageTextTimer = MESSAGE_TIMEOUT_US;
    dirtyDisplay = true;
}

void OmxDisp::displayMessagef(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[24];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    displayMessage(buf);
}

void OmxDisp::u8g2centerText(const char* s, int16_t x, int16_t y, uint16_t w, uint16_t h) {
//  int16_t bx, by;
	uint16_t bw, bh;
	bw = u8g2_display.getUTF8Width(s);
	bh = u8g2_display.getFontAscent();
	u8g2_display.setCursor(
		x + (w - bw) / 2,
		y + (h - bh) / 2
	);
	u8g2_display.print(s);
}

void OmxDisp::u8g2centerNumber(int n, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	char buf[8];
	itoa(n, buf, 10);
	u8g2centerText(buf, x, y, w, h);
}

void OmxDisp::testdrawrect() {
	display.clearDisplay();

	for(int16_t i=0; i<display.height()/2; i+=2) {
		display.drawRect(i, i, display.width()-2*i, display.height()-2*i, SSD1306_WHITE);
		display.display(); // Update screen with each newly-drawn rectangle
		delay(1);
	}

	delay(500);
}

void OmxDisp::drawLoading() {
	const char* loader[] = {"\u25f0", "\u25f1", "\u25f2", "\u25f3"};
	display.clearDisplay();
	u8g2_display.setFontMode(0);
	for(int16_t i=0; i<16; i+=1) {
		display.clearDisplay();
		u8g2_display.setCursor(18,18);
		u8g2_display.setFont(FONT_TENFAT);
		u8g2_display.print("OMX-27");
		u8g2_display.setFont(FONT_SYMB_BIG);
		u8g2centerText(loader[i%4], 80, 10, 32, 32); // "\u00BB\u00AB" // // dice: "\u2685"
		display.display();
		delay(100);
	}

	delay(100);
}


void OmxDisp::dispGridBoxes(){
	display.fillRect(0, 0, gridw, 10, WHITE);
	display.drawFastVLine(gridw/4, 0, gridh, INVERSE);
	display.drawFastVLine(gridw/2, 0, gridh, INVERSE);
	display.drawFastVLine(gridw*0.75, 0, gridh, INVERSE);
}
void OmxDisp::invertColor(bool flip){
	if (flip) {
		u8g2_display.setForegroundColor(BLACK);
		u8g2_display.setBackgroundColor(WHITE);
	} else {
		u8g2_display.setForegroundColor(WHITE);
		u8g2_display.setBackgroundColor(BLACK);
	}
}
void OmxDisp::dispValBox(int v, int16_t n, bool inv){			// n is box 0-3
	invertColor(inv);
	u8g2centerNumber(v, n*32, hline*2+3, 32, 22);
}

void OmxDisp::dispSymbBox(const char* v, int16_t n, bool inv){			// n is box 0-3
	invertColor(inv);
	u8g2centerText(v, n*32, hline*2+3, 32, 22);
}

void OmxDisp::setSubmode(int submode) {
    switch(submode){
		case SUBMODE_MIDI:
//			if (midiRoundRobin) {
//				displaychan = rrChannel;
//			}
			legends[0] = "OCT";
			legends[1] = "CH";
			legends[2] = "CC";
			legends[3] = "NOTE";
			legendVals[0] = (int)midiSettings.octave+4;
			legendVals[1] = sysSettings.midiChannel;
			legendVals[2] = potSettings.potVal;
			legendVals[3] = midiSettings.midiLastNote;
			dispPage = 1;
			break;
		case SUBMODE_MIDI2:
			legends[0] = "RR";
			legends[1] = "RROF";
			legends[2] = "PGM";
			legends[3] = "BNK";
			legendVals[0] = midiSettings.midiRRChannelCount;
			legendVals[1] = midiSettings.midiRRChannelOffset;
			legendVals[2] = midiSettings.currpgm + 1;
			legendVals[3] = midiSettings.currbank;
			dispPage = 2;
			break;
		case SUBMODE_MIDI3:
			legends[0] = "PBNK";	// Potentiometer Banks
			legends[1] = "THRU";	// MIDI thru (usb to hardware)
			legends[2] = "MCRO";	// Macro mode
			legends[3] = "M-CH";
			legendVals[0] = potSettings.potbank + 1;
			legendVals[1] = -127;
			if (midiSettings.midiSoftThru) {
				legendText[1] = "On";
			} else {
				legendText[1] = "Off";
			}			
			legendVals[2] = -127;
			legendText[2] = macromodes[midiMacroConfig.midiMacro];
			legendVals[3] = midiMacroConfig.midiMacroChan;
			dispPage = 3;
			break;
		case SUBMODE_MIDI4:
			legends[0] = " BG ";	
			legends[1] = "---";
			legends[2] = "---";
			legends[3] = "---";
			legendVals[0] = 0;
			legendVals[1] = 0;
			legendVals[2] = 0;
			legendVals[3] = 0;
			dispPage = 4;
			break;
		case SUBMODE_SEQ:
			// legends[0] = "PTN";
			// legends[1] = "TRSP";
			// legends[2] = "SWNG"; //"TRSP";
			// legends[3] = "BPM";
			// legendVals[0] = sequencer.playingPattern + 1;
			// legendVals[1] = (int)transpose;
			// legendVals[2] = (int)sequencer.getCurrentPattern()->swing; //(int)swing;
			// // legendVals[2] =  swing_values[sequencer.getCurrentPattern()->swing];
			// legendVals[3] = (int)clockbpm;
			// dispPage = 1;
			break;
		case SUBMODE_SEQ2:
			// legends[0] = "SOLO";
			// legends[1] = "LEN";
			// legends[2] = "RATE";
			// legends[3] = "CV"; //cvPattern
			// legendVals[0] = sequencer.getCurrentPattern()->solo; // playingPattern+1;
			// legendVals[1] = sequencer.getPatternLength(sequencer.playingPattern);
			// legendVals[2] = -127;
			// legendText[2] = mdivs[sequencer.getCurrentPattern()->clockDivMultP];

			// legendVals[3] = -127;
			// if (sequencer.getCurrentPattern()->sendCV) {
			// 	legendText[3] = "On";
			// } else {
			// 	legendText[3] = "Off";
			// }
			// dispPage = 2;
			break;
		case SUBMODE_PATTPARAMS:
			// legends[0] = "PTN";
			// legends[1] = "LEN";
			// legends[2] = "ROT";
			// legends[3] = "CHAN";
			// legendVals[0] = sequencer.playingPattern + 1;
			// legendVals[1] = sequencer.getPatternLength(sequencer.playingPattern);
			// legendVals[2] = rotationAmt; //(int)transpose;
			// legendVals[3] = sequencer.getPatternChannel(sequencer.playingPattern);
			// dispPage = 1;
			break;
		case SUBMODE_PATTPARAMS2:
			// legends[0] = "START";
			// legends[1] = "END";
			// legends[2] = "FREQ";
			// legends[3] = "PROB";
			// legendVals[0] = sequencer.getCurrentPattern()->startstep + 1;			// STRT step to autoreset on
			// legendVals[1] = sequencer.getCurrentPattern()->autoresetstep;			// STP step to autoreset on - 0 = no auto reset
			// legendVals[2] = sequencer.getCurrentPattern()->autoresetfreq; 			// FRQ to autoreset on -- every x cycles
			// legendVals[3] = sequencer.getCurrentPattern()->autoresetprob;			// PRO probability of resetting 0=NEVER 1=Always 2=50%
			// dispPage = 2;
			break;
		case SUBMODE_PATTPARAMS3:
			// legends[0] = "RATE";
			// legends[1] = "SOLO";
			// legends[2] = "---";
			// legends[3] = "---";

			// // RATE FOR CURR PATTERN
			// legendVals[0] = -127;
			// legendText[0] = mdivs[sequencer.getCurrentPattern()->clockDivMultP];

			// legendVals[1] = sequencer.getCurrentPattern()->solo;
			// legendVals[2] = 0; 			// TBD
			// legendVals[3] = 0;			// TBD
			// dispPage = 3;
			break;
		case SUBMODE_STEPREC:
			// legends[0] = "OCT";
			// legends[1] = "STEP";
			// legends[2] = "NOTE";
			// legends[3] = "PTN";
			// legendVals[0] = (int)octave+4;
			// legendVals[1] = sequencer.seqPos[sequencer.playingPattern]+1;
			// legendVals[2] = getSelectedStep()->note; //(int)transpose;
			// legendVals[3] = sequencer.playingPattern+1;
			// dispPage = 1;
			break;
		case SUBMODE_NOTESEL:
			// legends[0] = "NOTE";
			// legends[1] = "OCT";
			// legends[2] = "VEL";
			// legends[3] = "LEN";
			// legendVals[0] = getSelectedStep()->note;
			// legendVals[1] = (int)octave+4;
			// legendVals[2] = getSelectedStep()->vel;
			// legendVals[3] = getSelectedStep()->len + 1;
			// dispPage = 1;
			break;
		case SUBMODE_NOTESEL2:
// 			legends[0] = "TYPE";
// 			legends[1] = "PROB";
// 			legends[2] = "COND";
// 			legends[3] = "";
// 			legendVals[0] = -127;
// 			legendText[0] = stepTypes[getSelectedStep()->stepType];
// 			legendVals[1] = getSelectedStep()->prob;
// //				String ac = String(trigConditionsAB[][0]);
// //				String bc = String(trigConditionsAB[getSelectedStep()->condition][1]);

// 			legendVals[2] = -127;
// 			legendText[2] = trigConditions[getSelectedStep()->condition]; //ac + bc; // trigConditions

// 			legendVals[3] = 0;
// 			dispPage = 2;
			break;
		case SUBMODE_NOTESEL3:
			// legends[0] = "L-1";
			// legends[1] = "L-2";
			// legends[2] = "L-3";
			// legends[3] = "L-4";
			// for (int j=0; j<4; j++){
			// 	int stepNoteParam = getSelectedStep()->params[j];
			// 	if (stepNoteParam > -1){
			// 		legendVals[j] = stepNoteParam;
			// 	} else {
			// 		legendVals[j] = -127;
			// 		legendText[j] = "---";
			// 	}
			// }
			// dispPage = 3;
			break;

		default:
			break;
	}
}

void OmxDisp::clearLegends()
{
    legends[0] = "";
    legends[1] = "";
    legends[2] = "";
    legends[3] = "";
    legendVals[0] = 0;
    legendVals[1] = 0;
    legendVals[2] = 0;
    legendVals[3] = 0;
    dispPage = 0;
    legendText[0] = "";
    legendText[1] = "";
    legendText[2] = "";
    legendText[3] = "";
}

void OmxDisp::dispGenericMode(int submode, int selected){
	// const char* legends[4] = {"","","",""};
	// int legendVals[4] = {0,0,0,0};
	// int dispPage = 0;
	// const char* legendText[4] = {"","","",""};
//	int displaychan = sysSettings.midiChannel;

	u8g2_display.setFontMode(1);
	u8g2_display.setFont(FONT_LABELS);
	u8g2_display.setCursor(0, 0);
	dispGridBoxes();

	// labels
	u8g2_display.setForegroundColor(BLACK);
	u8g2_display.setBackgroundColor(WHITE);

	for (int j= 0; j<4; j++){
		u8g2centerText(legends[j], (j*32) + 1, hline-2, 32, 10);
	}

	// value text formatting
	u8g2_display.setFontMode(1);
	u8g2_display.setFont(FONT_VALUES);
	u8g2_display.setForegroundColor(WHITE);
	u8g2_display.setBackgroundColor(BLACK);


	switch(selected){
		case 1:
			display.fillRect(0*32+2, 9, 29, 21, WHITE);
			break;
		case 2: 	//
			display.fillRect(1*32+2, 9, 29, 21, WHITE);
			break;
		case 3: 	//
			display.fillRect(2*32+2, 9, 29, 21, WHITE);
			break;
		case 4: 	//
			display.fillRect(3*32+2, 9, 29, 21, WHITE);
			break;
		case 0:
		default:
			break;
	}
	// ValueBoxes
	int highlight = false;
	for (int j = 1; j < NUM_DISP_PARAMS; j++){ // start at 1 to only highlight values 1-4

		if (j == selected) {
			highlight = true;
		}else{
			highlight = false;
		}
		if (legendVals[j-1] == -127){
			dispSymbBox(legendText[j-1], j-1, highlight);
		} else {
			dispValBox(legendVals[j-1], j-1, highlight);
		}
	}
	if (dispPage != 0) {
		for (int k=0; k<4; k++){
			if (dispPage == k+1){
				dispPageIndicators(k, true);
			} else {
				dispPageIndicators(k, false);
			}
		}
	}

}

void OmxDisp::dispPageIndicators(int page, bool selected){
	if (selected){
		display.fillRect(43 + (page * 12), 30, 6, 2, WHITE);
	} else {
		display.fillRect(43 + (page * 12), 31, 6, 1, WHITE);
	}
}

void OmxDisp::dispMode(){
	// labels formatting
	u8g2_display.setFontMode(1);
	u8g2_display.setFont(FONT_BIG);
	u8g2_display.setCursor(0, 0);

	u8g2_display.setForegroundColor(WHITE);
	u8g2_display.setBackgroundColor(BLACK);

	const char* displaymode = "";
	if (sysSettings.newmode != sysSettings.omxMode && enc_edit) {
		displaymode = modes[sysSettings.newmode]; // display.print(modes[sysSettings.newmode]);
	} else if (enc_edit) {
		displaymode = modes[sysSettings.omxMode]; // display.print(modes[mode]);
	}
	u8g2centerText(displaymode, 86, 20, 44, 32);
}

void OmxDisp::setDispDirty(){
    dirtyDisplay = true;
}

extern OmxDisp omxidsp;
