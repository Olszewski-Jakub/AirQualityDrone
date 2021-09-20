#include "led_functions.h"

void setOutput(int pins[])
{
    for (int i = 0; i < sizeof(pins); i++)
    {
        pinMode(pins[i], OUTPUT);
    }
}

void setRgbColor(String color, int pins[])
{
    if (color == "red")
    {
        digitalWrite(pins[0], LOW);
        digitalWrite(pins[1], HIGH);
        digitalWrite(pins[2], HIGH);
    }

    if (color == "green")
    {
        digitalWrite(pins[0], HIGH);
        digitalWrite(pins[1], LOW);
        digitalWrite(pins[2], HIGH);
    }

    if (color == "off")
    {
        digitalWrite(pins[0], HIGH);
        digitalWrite(pins[1], HIGH);
        digitalWrite(pins[2], HIGH);
    }
}