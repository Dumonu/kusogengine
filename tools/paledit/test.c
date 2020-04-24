#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "nuklear_backend.h"

#include "animage.h"

#define min(a, b) a < b ? a : b

int redraw = 1;
struct sprite spr;
struct palette pal;
uint8_t *pixels;

char* saveas;

SDL_Window *sprWin;

struct nk_color pl2nk_color(int i)
{
    return (struct nk_color){ .r = pal.srgb[i].red, .g = pal.srgb[i].green, .b = pal.srgb[i].blue, .a = 255 };
}

void sprite(struct nk_context *ctx)
{
    int win_w, win_h;
    SDL_GetWindowSize(sprWin, &win_w, &win_h);

    if(redraw)
    {
        SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
        int depth = 32, pitch = 4 * spr.width;
        int i =0;

        //Fill pixels with the actual sprite data
        for(int row = 0; row < spr.height; ++row)
        {	
            for(int col = 0; col < spr.width; ++col)
            {
                uint8_t r,g,b,a;
                r = pal.srgb[spr.colors[row * spr.width + col].color].red;
                g = pal.srgb[spr.colors[row * spr.width + col].color].green;
                b = pal.srgb[spr.colors[row * spr.width + col].color].blue;
                a = spr.colors[row * spr.width + col].alpha;
                pixels[i] =r;
                pixels[i+1] =g;
                pixels[i+2] =b;
                pixels[i+3] =a;
                //      printf("%i %i %i %i%s", pixels[i], pixels[i+1], pixels[i+2], pixels[i+3], col < spr.width - 1 ? "\t" : "\n");
                i+=4;
            }
        }
        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels, spr.width, spr.height,depth, pitch, format->Rmask, format->Gmask, format->Bmask, format->Amask);


        SDL_Rect dest;
        dest.x =0;
        dest.y =0;
        dest.w = min(spr.width, win_w);
        dest.h = min(spr.height, win_h);

        SDL_Rect src;
        src.x =0;
        src.y =0;
        src.w = min(spr.width, win_w);
        src.h = min(spr.height, win_h);

        SDL_FillRect(SDL_GetWindowSurface(sprWin), NULL, 0xED9564FF);
        SDL_BlitSurface(surface, &src, SDL_GetWindowSurface(sprWin), &dest);

        //  SDL_RenderPresent(rend);
        SDL_UpdateWindowSurface(sprWin);

        redraw = 0;
    }
}

void palette(struct nk_context *ctx)
{
    if(nk_begin(ctx, "Palette", nk_rect(0, 0, 750, 528), NK_WINDOW_BACKGROUND))
    {
        static int index = 0;
        static int labelTime = 0;
        nk_layout_row(ctx, NK_DYNAMIC, 480, 2, (float[]){.70, .30});
        if(nk_group_begin(ctx, "Color Palette", 0))
        {
            nk_layout_row_dynamic(ctx, 26, 16);
            for(int i = 0; i < 256; ++i)
            {
                if(nk_button_color(ctx, pl2nk_color(i)))
                    index = i;
            }
            nk_group_end(ctx);
        }
        if(nk_group_begin(ctx, "Color Editor", 0))
        {
            struct nk_color col = pl2nk_color(index);
            nk_layout_row_static(ctx, 200, 200, 1);
            col = nk_rgba_cf(nk_color_picker(ctx, nk_color_cf(col), NK_RGBA));
            nk_layout_row_dynamic(ctx, 0, 1);
            col.r = nk_propertyi(ctx, "#R:", 0, col.r, 255, 1, 1);
            col.g = nk_propertyi(ctx, "#G:", 0, col.g, 255, 1, 1);
            col.b = nk_propertyi(ctx, "#B:", 0, col.b, 255, 1, 1);

            if(col.r != pal.srgb[index].red || col.g != pal.srgb[index].green || col.g != pal.srgb[index].blue)
            {
                redraw = 1;
                pal.srgb[index].red = col.r;
                pal.srgb[index].green = col.g;
                pal.srgb[index].blue = col.b;
            }

            if(labelTime)
            {
                labelTime = (labelTime + 1) % 100;
                nk_label_wrap(ctx, "Saved...");
            }
            else
                nk_label_wrap(ctx, "");

            if(nk_button_label(ctx, "Save Palette"))
            {
                savePalette(saveas, &pal);
                labelTime = 1;
            }

            nk_group_end(ctx);
        }

    }
    nk_end(ctx);
}

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s <sprite> <palette> [<saveas>]\n", argv[0]);
        return 1;
    }

    saveas = argv[argc - 1];

    int ret;
    if((ret = readSprite(argv[1], &spr)))
        return ret;
    if((ret = readPalette(argv[2], &pal)))
        return ret;

    init_SDL();

    SDL_Rect display;
    if(SDL_GetDisplayBounds(0, &display) < 0)
    {
        fprintf(stderr, "Line %d: %s\n", __LINE__, SDL_GetError());
    }
    
    sprWin = SDL_CreateWindow("Sprite", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            min(spr.width, display.w), min(spr.height, display.h), 0);
    if(sprWin == NULL)
    {
        fprintf(stderr, "Line %d: %s\n", __LINE__, SDL_GetError());
    }
            
    pixels = calloc(4 * spr.height * spr.width, sizeof(*pixels));

    void (*scripts[])(struct nk_context*) = {
        palette, 
        sprite
    };
    run_nuklear(scripts, 2);

    SDL_DestroyWindow(sprWin);
    quit_SDL();

    free(pixels);
}
