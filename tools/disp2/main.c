#include <stdio.h>
#include "SDL2/SDL.h"
#include "animage.h"
#include <unistd.h>
int main(int argc, char** argv)
{
    if(argc < 4)
    {
        fprintf(stderr, "Usage: %s [animage sprite] [animage palette] [Num Frames]\n", argv[0]);
        return 1;
    }

    struct sprite* spr = malloc(sizeof(*spr));
    struct palette* pal = malloc(sizeof(*pal));

    int ret;
    if((ret = readSprite(argv[1], spr)))
        return ret;

    if((ret = readPalette(argv[2], pal)))
        return ret;

    SDL_Init(SDL_INIT_VIDEO);

    char title[100];
    sprintf(title, "%s - %s", argv[1], argv[2]);

    //Set pixel format to RGBA32 this should hopefully work on both big and little endian machines.
    SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
    uint8_t* pixels = malloc(4* spr->height*spr->width * sizeof(*pixels));
    int depth, pitch;
    depth = 32;
    pitch = 4*spr->width;
    int i =0;
    
    //Fill pixels with the actual sprite data
    for(int row = 0; row < spr->height; ++row)
    {	
        for(int col = 0; col < spr->width; ++col)
        {
            uint8_t r,g,b,a;
            r = pal->srgb[spr->colors[row * spr->width + col].color].red;
            g = pal->srgb[spr->colors[row * spr->width + col].color].green;
            b = pal->srgb[spr->colors[row * spr->width + col].color].blue;
            a = spr->colors[row * spr->width + col].alpha;
            pixels[i] =r;
            pixels[i+1] =g;
            pixels[i+2] =b;
            pixels[i+3] =a;
//          printf("%i %i %i %i\n", pixels[i], pixels[i+1], pixels[i+2], pixels[i+3]);
	    i+=4;
        }
    }
    int num = atoi(argv[3]);
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels, spr->width, spr->height,depth, pitch, format->Rmask, format->Gmask, format->Bmask, format->Amask);
    
    SDL_Window* win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, spr->width/num, spr->height, 0);
//  SDL_Renderer* rend = SDL_CreateRenderer(win, -1, 0);
//  SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
	SDL_Rect dest;
        dest.x =0;
        dest.y =0;
        dest.w = spr->width;
        dest.h = spr->height;

    for(int i =0; i < num; i++){

        SDL_Rect src;
        src.x =(spr->width/num)*i;
        src.y =0;
        src.w = spr->width/num;
        src.h = spr->height;
    
	SDL_FillRect(SDL_GetWindowSurface(win), NULL, 0xED9564FF);
        SDL_BlitSurface(surface, &src, SDL_GetWindowSurface(win), &dest);
    
        //  SDL_RenderPresent(rend);
        SDL_UpdateWindowSurface(win);
    	SDL_Delay(17);
    }
    

    while(1)
    {
        SDL_Event e;
        if(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                break;
        }
    }

//    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();

    free(spr);
    free(pal);
}
