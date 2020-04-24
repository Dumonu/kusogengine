#include <stdio.h>
#include <string.h>
#include "animage.h"

int readSprite(char* filename, struct sprite* img)
{
    FILE* in = fopen(filename, "rb");
    fread(img->magic, 1, 8, in);
    if(memcmp(img->magic, SPRITE_MAGIC, 8) != 0) // This is not an animage sprite
    {
        fprintf(stderr, "%s is not an animage sprite.\n", filename);
        return 1;
    }

    fread(&(img->width), 4, 1, in);
    fread(&(img->height), 4, 1, in);

    if(img->width > MAX_WIDTH || img->height > MAX_HEIGHT)
    {
        fprintf(stderr, "Dimensions too large.\n");
        return 2;
    }

    for(int i = 0; i < img->height * img->width; ++i)
    {
        fread(&(img->colors[i].color), 1, 1, in);
        fread(&(img->colors[i].alpha), 1, 1, in);
    }

    fclose(in);
    return 0;
}

int readPalette(char* filename, struct palette* pal)
{
    FILE* in = fopen(filename, "rb");
    fread(pal->magic, 1, 8, in);
    if(memcmp(pal->magic, PALETTE_MAGIC, 8) != 0) // This is not an animage palette
    {
        fprintf(stderr, "%s is not an animage palette.\n", filename);
        return 1;
    }

    for(int i = 0; i < 256; ++i)
    {
        fread(&(pal->srgb[i].section), 1, 1, in);
        fread(&(pal->srgb[i].red), 1, 1, in);
        fread(&(pal->srgb[i].green), 1, 1, in);
        fread(&(pal->srgb[i].blue), 1, 1, in);
    }

    fclose(in);
    return 0;
}

int savePalette(char* filename, struct palette* pal)
{
    FILE* out = fopen(filename, "w");
    if(!out)
    {
        fprintf(stderr, "Can't open %s for write.\n", filename);
        return 1;
    }
    
    fwrite(pal->magic, 1, 8, out);
    for(int i = 0; i < 256; ++i)
    {
        fwrite(&(pal->srgb[i].section), 1, 1, out);
        fwrite(&(pal->srgb[i].red), 1, 1, out);
        fwrite(&(pal->srgb[i].green), 1, 1, out);
        fwrite(&(pal->srgb[i].blue), 1, 1, out);
    }
    fclose(out);
    return 0;
}
