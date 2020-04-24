#include <stdio.h>
#include <stdint.h>
#include "SDL2/SDL.h"
#include "global.h"
#include "settings.h"

#include "joyState.h"
#define InputCount ButtonCount + MacroCount

keybind keys[PlayerCount][InputCount] = DefaultKeybinds;
bool rebinding = false;
int rebindPlayer = 0;
int rebindButton = 0;

int nextJoyID = 0;
SDL_Joystick *joystick[MaxJoysticks];
uint8_t rawstate[PlayerCount][InputCount];
uint8_t joystate[PlayerCount][InputCount];

SDL_mutex* joylock;

int addJoy(int index)
{
    for(int i = 0; i < MaxJoysticks; ++i)
    {
        if(joystick[i] == NULL)
        {
            joystick[i] = SDL_JoystickOpen(index);
            if(!joystick[i])
            {
                errprint("Couldn't open Joystick: %s\n", SDL_GetError());
                SDL_ClearError();
                return 1; // SDL error
            }
            nextJoyID = SDL_JoystickInstanceID(joystick[i]) + 1; // auto-rebind controller
            return 0;
        }
    }
    return 2; // No more open spots
}

void remJoy(int index)
{
    for(int i = 0; i < MaxJoysticks; ++i)
    {
        if(joystick[i] != NULL && SDL_JoystickInstanceID(joystick[i]) == index)
        {
            int joyID = SDL_JoystickInstanceID(joystick[i]);
            SDL_JoystickClose(joystick[i]);
            joystick[i] = NULL;
            // Code for auto-rebind controller. A joystick instance ID is never reused,
            // so this moves all keybinds that use that ID to the next one to be added.
            for(int p = 0; p < PlayerCount; ++p)
            {
                for(int b = 0; b < InputCount; ++b)
                {
                    // If the next Joystick instance ID already has matching keybinds,
                    // e.g. if only a single stick is added and then removed and there are
                    // two sets of keybinds mapped by default, bump those up one.
                    if(keys[p][b].type != BIND_KEYBOARD && keys[p][b].joy >= nextJoyID)
                        keys[p][b].joy += 1;
                    // Set it to the next Joystick instance ID that will be added.
                    if(keys[p][b].type != BIND_KEYBOARD && keys[p][b].joy == joyID)
                        keys[p][b].joy = nextJoyID;
                }
            }
            return;
        }
    }
}


int updateStateAxi(SDL_JoyAxisEvent* jaxis)
{
  //  dbgprint("%d: Axis %d on Joystick %d: %d\n", jaxis->timestamp,
  //          jaxis->axis, jaxis->which, jaxis->value);
    if(rebinding)
    {
        if(jaxis->value >= 0 && jaxis->value >= (INT16_MAX / 2))
        {
            keys[rebindPlayer][rebindButton].type = BIND_AXIS;
            keys[rebindPlayer][rebindButton].joy = jaxis->which;
            keys[rebindPlayer][rebindButton].ind = jaxis->axis;
            keys[rebindPlayer][rebindButton].threshold = INT16_MAX / 2;
            rebinding = false;
            return 0;
        }
        else if(jaxis->value < 0 && jaxis->value <= (INT16_MIN / 2))
        {
            keys[rebindPlayer][rebindButton].type = BIND_AXIS;
            keys[rebindPlayer][rebindButton].joy = jaxis->which;
            keys[rebindPlayer][rebindButton].ind = jaxis->axis;
            keys[rebindPlayer][rebindButton].threshold = INT16_MIN / 2;
            rebinding = false;
            return 0;
        }
    }

    for(int p = 0; p < PlayerCount; ++p)
    {
        for(int i = 0; i < InputCount; ++i)
        {
            if(keys[p][i].type != BIND_AXIS) continue;
            if(keys[p][i].joy != jaxis->which) continue;
            if(keys[p][i].ind != jaxis->axis) continue;
            if(keys[p][i].threshold >= 0)
            {
                if(jaxis->value >= keys[p][i].threshold)
                    rawstate[p][i] = 1;
                else
                    rawstate[p][i] = 0;
            }
            else
            {
                if(jaxis->value <= keys[p][i].threshold)
                    rawstate[p][i] = 1;
                else
                    rawstate[p][i] = 0;
            }
        }
    }
    return 0;
}

int updateStateBut(SDL_JoyButtonEvent* jbutton)
{
    //  dbgprint("%d: Button %d down on Joystick %d: %d\n", jbutton->timestamp,
    //          jbutton->button, jbutton->which, jbutton->state);

    if(rebinding)
    {
        if(jbutton->type == SDL_JOYBUTTONDOWN)
        {
            keys[rebindPlayer][rebindButton].type = BIND_BUTTON;
            keys[rebindPlayer][rebindButton].joy = jbutton->which;
            keys[rebindPlayer][rebindButton].ind = jbutton->button;
            keys[rebindPlayer][rebindButton].threshold = 0;
            rebinding = false;
            return 0;
        }
    }

    for(int p = 0; p < PlayerCount; ++p)
    {
        for(int i = 0; i < InputCount; ++i)
        {
            if(keys[p][i].type != BIND_BUTTON) continue;
            if(keys[p][i].joy != jbutton->which) continue;
            if(keys[p][i].ind != jbutton->button) continue;
            if(jbutton->type == SDL_JOYBUTTONDOWN)
                rawstate[p][i] = 1;
            else
                rawstate[p][i] = 0;
        }
    }
    return 0;
}

int updateStateDev(SDL_JoyDeviceEvent* jdevice)
{
    if(jdevice->type == SDL_JOYDEVICEADDED)
    {
        dbgprint("%d: Joystick %d Added\n", jdevice->timestamp, jdevice->which);
        addJoy(jdevice->which);
    }
    else
    {
        dbgprint("%d: Joystick %d Removed\n", jdevice->timestamp, jdevice->which);
        remJoy(jdevice->which);
    }
    return 0;
}

int updateStateHat(SDL_JoyHatEvent* jhat)
{
 //   dbgprint("%d: POV Hat %d on Joystick %d: %d\n", jhat->timestamp,
   //         jhat->hat, jhat->which, jhat->value);
    if(rebinding)
    {
        switch(jhat->value)
        {
        case 1:
        case 2:
        case 4:
        case 8:
            keys[rebindPlayer][rebindButton].type = BIND_HAT;
            keys[rebindPlayer][rebindButton].joy = jhat->which;
            keys[rebindPlayer][rebindButton].ind = jhat->hat;
            keys[rebindPlayer][rebindButton].threshold = jhat->value;
            rebinding = false;
            return 0;
        }
    }

    for(int p = 0; p < PlayerCount; ++p)
    {
        for(int i = 0; i < InputCount; ++i)
        {
            if(keys[p][i].type != BIND_HAT) continue;
            if(keys[p][i].joy != jhat->which) continue;
            if(keys[p][i].ind != jhat->hat) continue;
            if((keys[p][i].threshold & jhat->value) > 0)
                rawstate[p][i] = 1;
            else
                rawstate[p][i] = 0;
        }
    }
    return 0;
}

int updateStateKey(SDL_KeyboardEvent* key)
{
  //  dbgprint("%d: %s key pressed\n", key->timestamp, SDL_GetKeyName(key->keysym.sym));
    if(rebinding)
    {
        if(key->type == SDL_KEYDOWN)
        {
            keys[rebindPlayer][rebindButton].type = BIND_KEYBOARD;
            keys[rebindPlayer][rebindButton].joy = -1;
            keys[rebindPlayer][rebindButton].ind = key->keysym.sym;
            keys[rebindPlayer][rebindButton].threshold = 0;
            rebinding = false;
            return 0;
        }
    }
    for(int p = 0; p < PlayerCount; ++p)
    {
        for(int i = 0; i < InputCount; ++i)
        {
            if(keys[p][i].type != BIND_KEYBOARD) continue;
            if(keys[p][i].ind != key->keysym.sym) continue;
            if(key->type == SDL_KEYDOWN)
                rawstate[p][i] = 1;
            else
                rawstate[p][i] = 0;
        }
    }
    return 0;
}

void rebind(int player, enum ButtonName keybind)
{
    dbgprint("Rebinding Player %d's Button %d\n", player, keybind);
    rebinding = true;
    rebindPlayer = player;
    rebindButton = keybind;
}

int joyEvent(SDL_Event* e)
{
    int ret = 0;
    uint32_t et = e->type;
    if(et == SDL_KEYDOWN || et == SDL_KEYUP || et == SDL_JOYAXISMOTION || et == SDL_JOYHATMOTION ||
            et == SDL_JOYBUTTONDOWN || et == SDL_JOYBUTTONUP || et == SDL_JOYDEVICEADDED ||
            et == SDL_JOYDEVICEREMOVED)
    {
        if(SDL_LockMutex(joylock) == 0)
        {
            if(e->type == SDL_KEYDOWN || e->type == SDL_KEYUP)
                ret = updateStateKey(&e->key);
            if(e->type == SDL_JOYAXISMOTION)
                ret = updateStateAxi(&e->jaxis);
            if(e->type == SDL_JOYHATMOTION)
                ret = updateStateHat(&e->jhat);
            if(e->type == SDL_JOYBUTTONDOWN || e->type == SDL_JOYBUTTONUP)
                ret = updateStateBut(&e->jbutton);
            if(e->type == SDL_JOYDEVICEADDED || e->type == SDL_JOYDEVICEREMOVED)
                ret = updateStateDev(&e->jdevice);
            SDL_UnlockMutex(joylock);
        }
        else
        {
            errprint(__FILE__" - Line "xstr(__LINE__)": %s", SDL_GetError());
            return 1;
        }
    }

    return ret;
}

void joyInit(void)
{
    joylock = SDL_CreateMutex();
    if(!joylock)
    {
        errprint(__FILE__" - Line "xstr(__LINE__)": %s", SDL_GetError());
        exit(1);
    }

    for(int i = 0; i < SDL_NumJoysticks() && i < MaxJoysticks; ++i)
    {
        dbgprint("Joystick %d Added\n", i);
        addJoy(i);
    }

    memset(joystick, (size_t)NULL, MaxJoysticks * sizeof(*joystick));
    memset(rawstate, 0, PlayerCount * InputCount * sizeof(**rawstate));
    memset(joystate, 0, PlayerCount * InputCount * sizeof(**joystate));
}

int joyUpdate(void)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_QUIT) return 1;
        joyEvent(&event);
    }

    // Remove the one-frame button states
    if(SDL_LockMutex(joylock) == 0)
    {
        for(int p = 0; p < PlayerCount; ++p)
        {
            for(int i = 0; i < InputCount; ++i)
            {
                if(rawstate[p][i] == 1)
                    joystate[p][i] = joystate[p][i] == 0 ? 1 : 2;
                else
                    joystate[p][i] = joystate[p][i] == 2 ? 3 : 0;
            }
        }
        SDL_UnlockMutex(joylock);
    }
    else
    {
        errprint(__FILE__" - Line "xstr(__LINE__)": %s", SDL_GetError());
    }

    //dbgprint("[%d %d %d %d %d %d %d %d]\t", joystatep1[0], joystatep1[1], joystatep1[2],
    //        joystatep1[3], joystatep1[4], joystatep1[5], joystatep1[6], joystatep1[7]); 
    //dbgprint("[%d %d %d %d %d %d %d %d]\n", joystatep2[0], joystatep2[1], joystatep2[2],
    //        joystatep2[3], joystatep2[4], joystatep2[5], joystatep2[6], joystatep2[7]); 
    return 0;
}

void joyRip(void)
{
    for(int i = 0; i < MaxJoysticks; ++i)
    {
        if(joystick[i] != NULL && SDL_JoystickGetAttached(joystick[i]))
        {
            dbgprint("Joystick %d Removed\n", SDL_JoystickInstanceID(joystick[i]));
            SDL_JoystickClose(joystick[i]);
        }
    }
}

uint8_t* joyState(int player, uint8_t* out)
{
    if(SDL_LockMutex(joylock) == 0)
    {
        memcpy(out, joystate[player], InputCount * sizeof(*out));
        SDL_UnlockMutex(joylock);
        return out;
    }
    else
    {
        errprint(__FILE__" - Line "xstr(__LINE__)": %s", SDL_GetError());
        return NULL;
    }
}
