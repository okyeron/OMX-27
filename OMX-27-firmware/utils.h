/*
 * Utility Functions
 * Adapted from 16n Faderbank (c) 2020 by Tom Armitage
 * MIT License
 */
 
#pragma once
#include <Arduino.h>
#include "storage.h"

#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif


// Utility
inline void readEEheader(int start, uint8_t buffer[], int length) {
  for (int i = 0; i < length; i++) {
    buffer[i] = storage->read(start+i);
  }
}

inline void readEEArray(int start, uint8_t buffer[], int length) {
  for (int i = 0; i < length; i++) {
    buffer[i] = storage->read(start+i);
  }
}

inline void writeEEArray(int start, uint8_t buffer[], int length) {
  for (int i = 0; i < length; i++) {
    storage->write(start+i, buffer[i]);
  }
}

inline void printHex(uint8_t num) {
  char hexCar[2];

  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
}

inline void printHexArray(uint8_t* array, int size) {
  for(int i=0; i<size; i++){
    printHex(array[i]);
    Serial.print(" ");
  }
  Serial.println();
}

inline void printIntArray(int* array, int size) {
  for(int i=0; i<size; i++){
    printHex(array[i]);
    Serial.print(" ");
  }
  Serial.println();
}