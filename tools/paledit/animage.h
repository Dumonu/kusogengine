#ifndef ANIMAGE_H_
#define ANIMAGE_H_

#include <stdint.h>

#define SPRITE_MAGIC "AnImAgE2" 
#define PALETTE_MAGIC "AnPaLTt2"

#define MAX_WIDTH 8000
#define MAX_HEIGHT 8000

struct sprite
{
    uint8_t magic[8];
    uint32_t width;
    uint32_t height;
    struct {
        uint8_t color;
        uint8_t alpha;
    } colors[MAX_WIDTH * MAX_HEIGHT];
};

struct palette
{
    uint8_t magic[8];
    struct {
        uint8_t section;
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } srgb[256];
};

int readSprite(char* filename, struct sprite* out);
int readPalette(char* filename, struct palette* out);
int savePalette(char* filename, struct palette* pal);

#endif
