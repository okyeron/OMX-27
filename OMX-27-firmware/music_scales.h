#pragma once

const uint8_t rainbowSaturation = 127;
const uint8_t scaleBrightness = 200;

extern int scaleDegrees[12];
extern int scaleOffsets[12];
extern int scaleColors[12];
extern const char* scaleNames[];
extern const char* noteNames[];
extern const int scalePatterns[][7];

void setScale(int scaleRoot, int scalePattern);
int getNumScales();
extern int scaleLength;
