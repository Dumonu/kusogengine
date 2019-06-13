#ifndef JOYSTATE_H_
#define JOYSTATE_H_

#include "SDL2/SDL.h"
#include "settings.h"

enum bindType
{
    BIND_KEYBOARD, BIND_AXIS, BIND_BUTTON, BIND_HAT, BIND_NONE
};

typedef struct
{
    enum bindType type; //keyboard, axis, button, etc.
    int joy;  //if not keyboard, the joystick
    int ind;  //index of the axis, button, etc. or keycode
    int threshold; //if an axis, the threshold to count as pushed, takes sign
} keybind;

void rebind(int player, enum ButtonName keybind);

void joyInit(void);
int joyUpdate(void);
void joyRip(void);

uint8_t* joyState(int player, uint8_t* out);

#endif
