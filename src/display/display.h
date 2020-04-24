#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "animage.h"

namespace display{
extern uint64_t id_acc;
enum layer{ 
    UI,
    GAME
};

typedef struct{
    uint8_t frames;
    int w;
    int h;
    SDL_Surface* surface;
    SDL_Texture* texture;
}sprite_t;

typedef struct drawable_t{
    uint16_t x;
    uint16_t y;
    uint8_t flipped;
    uint8_t z_index;
    uint8_t current_frame;
    uint16_t cached_x;
    uint16_t cached_y;
    uint_fast8_t calculated;    
    sprite_t* sprite;
    struct drawable_t* parent;
    uint16_t layer;
}drawable_t;

typedef struct disp_node_t{
    uint64_t id;
    drawable_t* drawable;
    struct disp_node_t* next;
}disp_node_t;

extern disp_node_t* game_displayables;
extern disp_node_t* ui_displayables;

int insertDispNode(drawable_t* node, disp_node_t** list);
drawable_t* removeDispNode(uint64_t id, disp_node_t** list);

int initDisplay();
SDL_Window *makeWindow(uint16_t x, uint16_t y, char* name);
sprite_t* createSprite(struct sprite* spr, struct palette* pal, int num_frames);
drawable_t* drawFromSprite(sprite_t* spr, int x, int y, int z_ind, uint8_t flipped, drawable_t* parent, int layer);
void freeSprite(sprite_t* sprite);
int blitSprite(sprite_t* src, uint16_t x, uint16_t y, uint8_t frame, uint8_t flag);

//int fillRect(uint32_t color);

int updateWindow(void);
int updateViewport(SDL_Rect* p1, SDL_Rect* p2);
void drawGame(void);
void drawUI(void);

void destroyWindow(void);
}
#endif

