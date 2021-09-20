#ifndef DISPLAY_H
#define DISPLAY_H
#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void initDisplay();
void clearScreen();
void welcomeScreen();
void startScreen();
void showBeforeStartScreen();
void showFlightScreen(String arr[]);
#endif