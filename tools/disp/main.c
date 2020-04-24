#include <stdio.h>
#include "SDL2/SDL.h"
#include "animage.h"

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s [animage sprite] [animage palette]\n", argv[0]);
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
    SDL_Window* win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, spr->width, spr->height, 0);
    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, 0);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);

    for(int row = 0; row < spr->height; ++row)
    {
        for(int col = 0; col < spr->width; ++col)
        {
            SDL_SetRenderDrawColor(rend, 100, 149, 237, 255);
            SDL_RenderDrawPoint(rend, col, row);
            int r,g,b,a;
            r = pal->srgb[spr->colors[row * spr->width + col].color].red;
            g = pal->srgb[spr->colors[row * spr->width + col].color].green;
            b = pal->srgb[spr->colors[row * spr->width + col].color].blue;
            a = spr->colors[row * spr->width + col].alpha;
          //printf("%d, %d: (%x, %x, %x, %x)\n", col, row, r, g, b, a);
            SDL_SetRenderDrawColor(rend, r, g, b, a);
            SDL_RenderDrawPoint(rend, col, row);
        }
    }

    SDL_RenderPresent(rend);

    while(1)
    {
        SDL_Event e;
        if(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                break;
        }
    }

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();

    free(spr);
    free(pal);
}
