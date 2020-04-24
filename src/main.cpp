#include <stdio.h>
#include <stdlib.h>

#include "SDL2/SDL.h"

#include "global.h"
#include "settings.h"
#include "joyState.h"
#include "parse.h"
#include "display.h"
#include "audio.h"

#include "animage.h"
using namespace audio;
using namespace display;
    Uint64 NOW = 0;
    Uint64 LAST = 0;

    int testScene(void);
    typedef int(*upfunc_t)(void);
    upfunc_t test{testScene};

    upfunc_t* upfuncs[] = { &test };

    SDL_Rect P1, P2;

    //TODO: SHITTY CODE
    drawable_t* back_d;
    drawable_t* ryu_d;
    drawable_t* ryu2_d;
    sound_t* music;
    sound_t* sfx;
    track_t* sfx_track;
    //END SHITTY CODE

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
        int funcs = sizeof(upfuncs) / sizeof(upfuncs[0]);
        for(int i = 0; i < funcs; ++i)
        {
            if((ret = (*upfuncs[i])()))
            {
                errprint("Update Function %d returned %d\n", i, ret);
                return ret;
            }
        }

        // Drawing
        ret = updateViewport(&P1, &P2);
        //ret=0;
        if(ret)
        {
            errprint("Update Viewport Function returned %d\n", ret);
            return ret;
        }

        uint8_t* sp1 = (uint8_t*)malloc((ButtonCount + MacroCount + 1) * sizeof(*sp1));
        sp1 = joyState(0, sp1);

        uint8_t* sp2 = (uint8_t*)malloc((ButtonCount + MacroCount + 1) * sizeof(*sp2));
        sp2 = joyState(1, sp2);

        if(sp1[2]==1||sp1[2]==2){
            if(ryu_d->x >0)
            ryu_d->x-=5;
        }
        if(sp1[3]==1||sp1[3]==2){
            if(ryu_d->x + ryu_d->sprite->surface->w < 2400)
            ryu_d->x+=5;
        }

        if(sp2[2]==1||sp2[2]==2){
            if(ryu2_d->x >0)
            ryu2_d->x-=5;
        }
        if(sp2[3]==1||sp2[3]==2){
            if(ryu2_d->x + ryu2_d->sprite->surface->w < 2400)
            ryu2_d->x+=5;
        }
        drawGame();
        drawUI();
        ret = updateWindow();
        if(ret)
        {
            errprint("Update Window Function returned %d\n", ret);
            return ret;
        }

        // Audio
        audio::updateAudio();
        return 0;
    }

    int initialize(void)
    {
        int ret;
        if((ret = SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_EVENTS)))
        {
            errprint("Failure initializing SDL: %s\n", SDL_GetError());
            return ret;
        }
        joyInit();
        initQueues();
        SDL_Rect bounds;
        if((ret = SDL_GetDisplayUsableBounds(0, &bounds)))
        {
            errprint("Unable to find display bounds: %s\n", SDL_GetError());
            return ret;
        }
        SDL_Window * window = makeWindow(bounds.w, bounds.h, "Fight Game");
        initDisplay();
        P1.x = 300; P1.y = 300;
        audio::initAudio(window);
        return 0;
    }

    int teardown(void)
    {
        destroyWindow();
        joyRip();
        SDL_Quit();
        return 0;
    }

    int main(int argc, char** argv)
    {
        int ret = 0;
        if((ret = initialize()))
            return ret;

        //TODO: REMOVE THIS SHITTY BLOCK:
        struct sprite *back_spr  = new struct sprite;
        struct palette* back_pal = new struct palette;
        struct sprite *ryu_spr   = new struct sprite;
        struct palette* ryu_pal  = new struct palette;

        printf("Filename %s\n",argv[1]);
        readSprite(argv[1], back_spr);
        readPalette(argv[2], back_pal);
        readSprite(argv[3], ryu_spr);
        readPalette(argv[4], ryu_pal);



        sprite_t* back = createSprite(back_spr, back_pal, 1);
        sprite_t* ryu = createSprite(ryu_spr, ryu_pal, 1);

        back_d = display::drawFromSprite(back, 0, 0, 0, 0, NULL, GAME);
        ryu_d = display::drawFromSprite(ryu, 1300, 900, 1, 0,  NULL, GAME);
        ryu2_d = display::drawFromSprite(ryu, 900, 900, 1, 1, NULL, GAME);

        music = audio::loadAudio(argv[5]);
        sfx = audio::loadAudio(argv[6]);
        //END TODO

        double deltaTime = 0.0;
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
        if(ret != -1)
            return ret;

        //TODO: MORE SHITTY CODE:
        free(back);
        free(ryu);

        free(back_spr);
        free(back_pal);
        free(ryu_spr);
        free(ryu_pal);
        //END SHITTY CODE

        if((ret = teardown()))
            return ret;
    }

    int testScene(void)
    {
        static int frame = -1;
        //ryu_d->x++;
        //ryu2_d->y--;
        ++frame;
        if(frame == 0)
        {
            audio::playMusic(music, 10);
            //sfx_track = audio::playSound(sfx, 0, 0);
        }
        P1.x = ryu_d->x;
        P1.y = ryu_d->y;
        P1.w = ryu_d->sprite->surface->w / ryu_d->sprite->frames;
        P1.h = ryu_d->sprite->surface->h;
        P2.x = ryu2_d->x;
        P2.y = ryu2_d->y;
        P2.w = ryu2_d->sprite->surface->w / ryu2_d->sprite->frames;
        P2.h = ryu2_d->sprite->surface->h;
        //sfx_track->x_pos += 5;    
        return 0;
    }