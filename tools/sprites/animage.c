#include <stdio.h>
#include <string.h>
#include "animage.h"

uint8_t color16to8(uint16_t value)
{
    return value >> 8;
}

uint32_t distSq(int r1, int g1, int b1, int r2, int g2, int b2)
{
    return (r1 - r2) * (r1 - r2) + (g1 - g2) * (g1 - g2) + (b1 - b2) * (b1 - b2);
}

uint32_t addColor(uint8_t r, uint8_t g, uint8_t b, struct sprite* spr, struct palette* pal, int row, int col)
{
    uint32_t min = UINT32_MAX;
    uint8_t minI1 = 0;
    uint8_t minI2 = 0;

    for(int i1 = 0; i1 < 256; ++i1)
    {
        for(int i2 = i1 + 1; i2 < 256; ++i2)
        {
            uint32_t d = distSq(pal->srgb[i1].red, pal->srgb[i1].green, pal->srgb[i1].blue,
                    pal->srgb[i2].red, pal->srgb[i2].green, pal->srgb[i2].blue);
            if(d < min)
            {
                min = d;
                minI1 = i1;
                minI2 = i2;
            }
        }
    }

    // set minI1 = to the average of the two colors
    pal->srgb[minI1].red = (pal->srgb[minI1].red + pal->srgb[minI2].red) / 2;
    pal->srgb[minI1].green = (pal->srgb[minI1].green + pal->srgb[minI2].green) / 2;
    pal->srgb[minI1].blue = (pal->srgb[minI1].blue + pal->srgb[minI2].blue) / 2;
    // set minI2 = to the new color
    pal->srgb[minI2].red = r;
    pal->srgb[minI2].green = g;
    pal->srgb[minI2].blue = b;

    // update all instances of i2 in the spr to instances of i1
    for(int r = 0; r <= row; ++r)
    {
        for(int c = 0; c < spr->width && (!(r == row) || (c < col)); ++c)
        {
            if(spr->colors[r * spr->width + c].color == minI2)
            {
                spr->colors[r * spr->width + c].color = minI1;
            }
        }
    }
    // set the (row, col) pixel of spr to minI2
    spr->colors[row * spr->width + col].color = minI2;
    // return the min distance squared
    return min;
}

int ff2anI(struct farbfeld* in, struct sprite* sout, struct palette* pout)
{
    if(memcmp(in->magic, "farbfeld", 8) != 0)
    {
        fprintf(stderr, "This is not a farbfeld image.\n");
        return 1;
    }

    memset(sout, 0, sizeof(*sout));
    memset(pout, 0, sizeof(*pout));

    // initialize the sprite and palette with magic numbers 
    memcpy(sout->magic, SPRITE_MAGIC, 8);
    memcpy(pout->magic, PALETTE_MAGIC, 8);
    sout->width = in->width;
    sout->height = in->height;

    int cnt = 0;
    uint32_t minDistSq = 0;
    // loop through pixels and do shit
    for(int r = 0; r < sout->height; ++r)
    {
        for(int c = 0; c < sout->width; ++c)
        {
            int done = 0;
            for(int i = 0; i < cnt; ++i)
            {
                uint32_t d = distSq(color16to8(in->rgba[r * in->width + c].red),
                        color16to8(in->rgba[r * in->width + c].green),
                        color16to8(in->rgba[r * in->width + c].blue),
                        pout->srgb[i].red, pout->srgb[i].green, pout->srgb[i].blue); 

                if(d <= minDistSq)
                {
                    sout->colors[r * in->width + c].color = i;
                    done = 1;
                    break;
                }
            }
            if(!done)
            {
                if(cnt < 256)
                {
                    pout->srgb[cnt].red = color16to8(in->rgba[r * in->width + c].red);
                    pout->srgb[cnt].green = color16to8(in->rgba[r * in->width + c].green);
                    pout->srgb[cnt].blue = color16to8(in->rgba[r * in->width + c].blue);
                    sout->colors[r * in->width + c].color = cnt++;
                }
                else
                {
                    minDistSq = addColor(color16to8(in->rgba[r * in->width + c].red),
                            color16to8(in->rgba[r * in->width + c].green),
                            color16to8(in->rgba[r * in->width + c].blue),
                            sout, pout, r, c);
                }
            }
            sout->colors[r * in->width + c].alpha = color16to8(in->rgba[r * in->width + c].alpha);
        }
    }
    return 0;
}

int saveSprite(char* filename, struct sprite* spr)
{
    FILE* out = fopen(filename, "w");
    if(!out)
    {
        fprintf(stderr, "Can't open %s for write.\n", filename);
        return 1;
    }
    
    //TODO: deal with endianness
    fwrite(spr->magic, 1, 8, out);
    fwrite(&(spr->width), 4, 1, out);
    fwrite(&(spr->height), 4, 1, out);
    for(int i = 0; i < spr->width * spr->height; ++i)
    {
        fwrite(&(spr->colors[i].color), 1, 1, out);
        fwrite(&(spr->colors[i].alpha), 1, 1, out);
    }
    fclose(out);
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
