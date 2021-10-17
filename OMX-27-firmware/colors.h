#pragma once

#include <stdint.h>

// COLOR PRESETS
// https://www.rapidtables.com/web/color/color-wheel.html
// hsl(xxx, 100%, 50%)
const auto RED = 0xFF0000;
const auto ORANGE = 0xFF8000;
const auto YELLOW = 0xFFFF00;
const auto LIME = 0x80FF00;
const auto GREEN = 0x00FF00;
const auto MINT = 0x00FF80;
const auto CYAN = 0x00FFFF;
const auto RBLUE = 0x007FFF;
const auto BLUE = 0x0000FF;
const auto PURPLE = 0x7F00FF;
const auto MAGENTA = 0xFF00FF;
const auto ROSE = 0xFF0080;

// hsl(xxx, 50%, 50%)
const auto MEDRED = 0xBF4040;
const auto MEDBLUE = 0x4040BF;
const auto MEDYELLOW = 0xBFBF40;

// hsl(xxx, 100%, 75%)
const auto LTCYAN = 0x80FFFF;
const auto LTPURPLE = 0xBF80FF;
const auto SALMON = 0xFF8080;
const auto PINK = 0xFF80D4;
const auto LTYELLOW = 0xFFFF80;

// hsl(xxx, 100%, 15%)
const auto DKRED = 0x800000;
const auto DKORANGE = 0x4D2600;
const auto DKYELLOW = 0x4C4D00;
const auto DKLIME = 0x408000;
const auto DKGREEN = 0x264D00;
const auto DKCYAN = 0x004C4D;
const auto DKBLUE = 0x00004D;
const auto DKPURPLE = 0x26004D;
const auto DKMAGENTA = 0x4D004C;
const auto INDIGO = 0x4B0082;

// hsl(xxx, 50%, 75%)
const auto LBLUE = 0x9FCFDF;
const auto VIOLET = 0xDF9FDF;

// hsl(xxx, 25%, 50%)
const auto DIMORANGE = 0x9F8060;
const auto DIMYELLOW = 0x9F9F60;
const auto DIMLIME = 0x809F60;
const auto DIMGREEN = 0x609F60;
const auto DIMMAGENTA = 0x9F609F;
const auto DIMCYAN = 0x609F9F;
const auto DIMBLUE = 0x60609F;
const auto DIMPURPLE = 0x7F609F;


// other
const auto AMBER = 0x999900;
const auto BEIGE = 0xFFCC33;

// no color

const auto WHITE = 0xFFFFFF;
const auto HALFWHITE = 0x808080;
const auto LOWWHITE = 0x202020;
const auto LEDOFF = 0x000000;

uint32_t stepColor = 0x000000;
uint32_t muteColor = 0x000000;

// sequencer pattern colors
const uint32_t seqColors[] = {ORANGE,YELLOW,GREEN,MAGENTA,CYAN,BLUE,LIME,LTPURPLE};
const uint32_t muteColors[] = {DKORANGE,DKYELLOW,DKGREEN,DKMAGENTA,DKCYAN,DKBLUE,DKLIME,DKPURPLE};

const auto MIDINOTEON = HALFWHITE;
const auto SEQCHASE = DKRED;
const auto SEQMARKER = LOWWHITE;
const auto SEQSTEP = ORANGE;

const auto NOTESEL = DKCYAN;
const auto PATTSEL = LIME;

const auto FUNKONE = LTCYAN;
const auto FUNKTWO = MINT;

const auto SEQ1C = HALFWHITE;
const auto SEQ2C = DKBLUE;
