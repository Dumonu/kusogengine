#include <stdio.h>
#include <stdlib.h>
#include "farbfeld.h"
#include "animage.h"

int main(int argc, char** argv)
{
    if(argc < 4)
    {
        fprintf(stderr, "Usage: %s [farbfeld file] [sprite name] [palette name]", argv[0]);
        return 2;
    }

    struct farbfeld* ff = malloc(sizeof(*ff));
    int ret;
    if((ret = readFarbfeld(argv[1], ff)))
        return ret;

    struct sprite* amg = malloc(sizeof(*amg));
    struct palette* apl = malloc(sizeof(*apl));

    if((ret = ff2anI(ff, amg, apl)))
        return ret;

    if((ret = saveSprite(argv[2], amg)))
        return ret;
    if((ret = savePalette(argv[3], apl)))
        return ret;

    free(ff);
    free(amg);
    free(apl);
}
