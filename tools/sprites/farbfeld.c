#include <stdio.h>
#include <string.h>
#include "farbfeld.h"

const int endint = 1;
#define bigendian() ((*(char*)&endint) == 0)

void fixEnd32(uint32_t* var)
{
    char* bytes = (char*)var;
    if(bigendian())
        return;

    char tmp = bytes[0];
    bytes[0] = bytes[3];
    bytes[3] = tmp;
    tmp = bytes[1];
    bytes[1] = bytes[2];
    bytes[2] = tmp;
}

void fixEnd16(uint16_t* var)
{
    char* bytes = (char*)var;
    if(bigendian())
        return;

    char tmp = bytes[0];
    bytes[0] = bytes[1];
    bytes[1] = tmp;
}

int readFarbfeld(char* filename, struct farbfeld* img)
{
    FILE* in = fopen(filename, "rb");
    fread(img->magic, 1, 8, in);
    if(memcmp(img->magic, "farbfeld", 8) != 0) // This is not a farbfeld image
    {
        fprintf(stderr, "%s is not a farbfeld image.\n", filename);
        return 1;
    }

    fread(&(img->width), 4, 1, in);
    fread(&(img->height), 4, 1, in);
    fixEnd32(&(img->width));
    fixEnd32(&(img->height));

    if(img->width > MAX_WIDTH || img->height > MAX_HEIGHT)
    {
        fprintf(stderr, "Dimensions too large.\n");
        return 2;
    }

    for(int r = 0; r < img->height; ++r)
    {
        for(int c = 0; c < img->width; ++c)
        {
            fread(&(img->rgba[r * img->width + c].red), 2, 1, in);
            fread(&(img->rgba[r * img->width + c].green), 2, 1, in);
            fread(&(img->rgba[r * img->width + c].blue), 2, 1, in);
            fread(&(img->rgba[r * img->width + c].alpha), 2, 1, in);
            fixEnd16(&(img->rgba[r * img->width + c].red));
            fixEnd16(&(img->rgba[r * img->width + c].green));
            fixEnd16(&(img->rgba[r * img->width + c].blue));
            fixEnd16(&(img->rgba[r * img->width + c].alpha));
        }
    }

    fclose(in);
    return 0;
}
