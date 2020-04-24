#ifndef FARBFELD_H_
#define FARBFELD_H_

#include <stdint.h>

#define MAX_WIDTH 8000
#define MAX_HEIGHT 8000

struct farbfeld
{
    uint8_t magic[8];
    uint32_t width;
    uint32_t height;
    struct
    {
        uint16_t red;
        uint16_t green;
        uint16_t blue;
        uint16_t alpha;
    } rgba[MAX_WIDTH * MAX_HEIGHT];
};

int readFarbfeld(char* filename, struct farbfeld*);

#endif
