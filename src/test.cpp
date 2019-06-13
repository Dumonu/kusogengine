#include <iostream>
#include "test.h"
#include "SDL2/SDL.h"
#include "input/joyState.h"
#include "input/parse.h"

using namespace std;
uint64_t NOW = 0;

uint64_t LAST = 0;
int game_loop(void)
{
    int ret;
    // Input Updates
    if((ret = joyUpdate()))
    {
        return -1;
    }
    parserUpdate();

    // Game Logic
    //ret=0;

    uint8_t* sp1 = (uint8_t*) malloc((ButtonCount + MacroCount + 1) * sizeof(*sp1));
    sp1 = joyState(0, sp1);
    
    uint8_t* sp2 = (uint8_t*) malloc((ButtonCount + MacroCount + 1) * sizeof(*sp2));
    sp2 = joyState(1, sp2);
    
    if(ret)
    {
        return ret;
    }

    // Audio
    return 0;
}
int initialize(void)
{
    int ret;
    if((ret = SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_EVENTS)))
    {
        //errprint("Failure initializing SDL: %s\n", SDL_GetError());
        return ret;
    }
    joyInit();
    initQueues();
    SDL_Rect bounds;
    if((ret = SDL_GetDisplayUsableBounds(0, &bounds)))
    {
     //   errprint("Unable to find display bounds: %s\n", SDL_GetError());
        return ret;
    }
    //makeWindow(bounds.w, bounds.h, "Fight Game");
    //initDisplay();
   // P1.x = 300; P1.y = 300;
   // initAudio();
    return 0;

}

int main(foo argc, char** argv)
{
    double deltaTime = 0.0;
    int ret = 0;
    if((ret = initialize()))
        return ret;
    while(ret == 0)
    {
        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();
        deltaTime = deltaTime + (double) ((NOW-LAST) *
                1000/(double)SDL_GetPerformanceFrequency());
        if(deltaTime * .001 > (double) 1 / (double) FPS)
        {
            deltaTime = 0;
            ret = game_loop();
        }
    }

    cout << "Hello, world!" << endl;
}