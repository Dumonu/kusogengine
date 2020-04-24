#include "SDL2/SDL_thread.h"
#include "joyState.h"
#include "settings.h"
#include <global.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <parse.h>
// Type that holds a queue of length queueLength. Used for storing input arrays.
typedef struct {
    int head;
    int tail;
    uint8_t* inputs[QueueLength];
    SDL_mutex* queueMutex;
} queue_t;

// Mod function that is always positive.
int mod(int x, int m)
{
    int r = x % m;
    if (r < 0) {
        return r + m;
    } else {
        return r;
    }
}
queue_t Queues[2];

// Initialize Queues
void initQueues()
{
    for (int i = 0; i < 2; i++) {
        Queues[i].head = 0;
        Queues[i].tail = 0;
        Queues[i].queueMutex = SDL_CreateMutex();
        if (!Queues[i].queueMutex) {
            fprintf(stderr, "Couldn't create mutex\n");
            return;
        }
    }
}

// Remove Buffers from memory.
void destroyBuffers()
{
    for (int i = 0; i < 2; i++) {
        for (int i = 0; i < QueueLength; i++) {
            if (Queues[i].inputs[i] != NULL)
                free(Queues[i].inputs[i]);
        }
        SDL_DestroyMutex(Queues[i].queueMutex);
    }
}

// Add a new input to the player's queue, deleting the input
void add(uint8_t inputs[], queue_t* player)
{

    for (int i = 3; i > 0; i--) {
        //Fix A using macros
        if (inputs[BUTTON_A] == i || inputs[MACRO_AC] == i || inputs[MACRO_AB] == i || inputs[MACRO_ABCD] == i) {
            inputs[BUTTON_A] = i;
        }
        //Fix B using macros
        if (inputs[BUTTON_B] == i || inputs[MACRO_AB] == i || inputs[MACRO_BD] == i || inputs[MACRO_ABCD] == i) {
            inputs[BUTTON_B] = i;
        }
        //Fix C using macros
        if (inputs[BUTTON_C] == i || inputs[MACRO_AC] == i || inputs[MACRO_ABCD] == i) {
            inputs[BUTTON_C] = i;
        }
        //Fix D using macros
        if (inputs[BUTTON_D] == i ||inputs[MACRO_BD] == i || inputs[MACRO_ABCD] == i) {
            inputs[BUTTON_D] = i;
        }
    }

    if (SDL_LockMutex(player->queueMutex) == 0) {
        if (player->inputs[player->head] != NULL) {
            if (mod((player->head - 1), QueueLength) == player->tail) {
                if (player->inputs[player->tail] != NULL)
                    free(player->inputs[player->tail]);

                player->inputs[player->tail] = inputs;
                player->tail = mod((player->tail - 1), QueueLength);
                player->head = mod((player->head - 1), QueueLength);
            } else {
                if (player->inputs[mod(player->head - 1, QueueLength)] != NULL)
                    free(player->inputs[mod(player->head - 1, QueueLength)]);

                player->inputs[mod(player->head - 1, QueueLength)] = inputs;
                player->head = mod((player->head - 1), QueueLength);
            }
        } else {
            if (player->inputs[player->head] != NULL)
                free(player->inputs[player->head]);

            player->inputs[player->head] = inputs;
            player->tail = player->head;
        }
        SDL_UnlockMutex(player->queueMutex);
    } else {
        fprintf(stderr, "Couldn't lock mutex\n");
    }

}

// Removes the element at the head location.
uint8_t* pop(queue_t* player)
{
    if (SDL_LockMutex(player->queueMutex) == 0) {
        uint8_t* temp = player->inputs[player->head];
        player->inputs[player->head] = NULL;
        player->head = mod((player->head + 1), QueueLength);
        SDL_UnlockMutex(player->queueMutex);
        return temp;
    }
    else
    {
        errprint(__FILE__" - "xstr(__LINE__)": %s", SDL_GetError());
        return NULL;
    }
}

// Returns the element at the head location
uint8_t* peek(queue_t* player)
{
    uint8_t* temp = player->inputs[player->head];
    return temp;
}

// Removes the element at the tail location.
void removeTail(queue_t* player)
{
    if (SDL_LockMutex(player->queueMutex) == 0) {
        if (player->inputs[player->tail] != NULL) {
            free(player->inputs[player->tail]);
            player->inputs[player->tail] = NULL;
            player->tail = mod((player->tail - 1), QueueLength);
        }
        SDL_UnlockMutex(player->queueMutex);
    }
    else
    {
        errprint(__FILE__" - "xstr(__LINE__)": %s", SDL_GetError());
    }
}
// Input: uint8_t array of an input in anime notation with the first element
// being the length of the input, player input buffer
uint8_t searchInput(uint8_t* input, queue_t* player, uint8_t flag)
{
    if (SDL_LockMutex(player->queueMutex) == 0) {
        int length = input[0];
        int currentIndex = length;
        int yPos;
        int xPos;
        int fiveCount;
        int savedI;
        // Every possible joystick position in the game defined by a 2D array.
        int possibleInputs[3][3] = { { 7, 8, 9 }, { 4, 5, 6 }, { 1, 2, 3 } };

        // Scan the entire buffer
        for (int i = player->head; i != player->tail;
                i = mod((i + 1), QueueLength)) {
            if (player->inputs[i][ButtonCount + 1] == 1 && !flag) {
                return 0;
            } else {
                // Use inputs to calculate what direction is being held
                yPos = 1;
                xPos = 1;
                if (player->inputs[i][0] == 1 || player->inputs[i][0] == 2) {
                    yPos--;
                }
                if (player->inputs[i][1] == 1 || player->inputs[i][1] == 2) {
                    yPos++;
                }
                if (player->inputs[i][2] == 1 || player->inputs[i][2] == 2) {
                    xPos--;
                }
                if (player->inputs[i][3] == 1 || player->inputs[i][3] == 2) {
                    xPos++;
                }
                // Give up on the input if more than fiveLimit fives are consecutive
                // *unless* that five is required by the input.

                //dbgprint("[%d %d %d]", possibleInputs[yPos][xPos], input[currentIndex],
                //    currentIndex);

                if (possibleInputs[yPos][xPos] == 5 && input[currentIndex] != possibleInputs[yPos][xPos]) {
                    fiveCount++;
                    if (fiveCount >= FiveLimit) {
                        SDL_UnlockMutex(player->queueMutex);
                        return 0;
                    }
                    // Wasn't a five or five was needed.
                } else {
                    fiveCount = 0;
                    if (input[currentIndex] == possibleInputs[yPos][xPos]) {
                        currentIndex--;
                        if (currentIndex == length - 1) {
                            savedI = i;
                        } else if (currentIndex == 0) {
                            player->inputs[savedI][ButtonCount + 1] = 1;
                            SDL_UnlockMutex(player->queueMutex);
                            return 1;
                        }
                    }
                }
            }
        }
        SDL_UnlockMutex(player->queueMutex);
    }
    else
    {
        errprint(__FILE__" - "xstr(__LINE__)": %s", SDL_GetError());
    }
    // Input wasn't found until now. Just return 0.
    return 0;
}

uint8_t searchInputHelper(uint8_t* input, uint8_t index, uint8_t flag){
        return searchInput(input,&Queues[index],flag);
}

// My update function.
void parserUpdate()
{
    uint8_t* sp1 = (uint8_t*) malloc((ButtonCount + MacroCount + 1) * sizeof(*sp1));
    sp1 = joyState(0, sp1);
 // dbgprint("[%d %d %d %d %d %d %d %d %d %d %d %d]\t", sp1[0], sp1[1],
 //         sp1[2], sp1[3], sp1[4], sp1[5],
 //         sp1[6], sp1[7], sp1[8], sp1[9],
 //         sp1[10], sp1[11]);
    uint8_t* sp2 = (uint8_t*) malloc((ButtonCount + MacroCount + 1) * sizeof(*sp2));
    sp2 = joyState(1, sp2);
    add(joyState(0, sp1), &Queues[0]);
    add(joyState(1, sp2), &Queues[1]);

   uint8_t testInput[] = { 3, 2, 3, 6 };
 dbgprint("[%d %d %d %d %d %d %d %d]\t", peek(Queues)[0], peek(Queues)[1],
        peek(Queues)[2], peek(Queues)[3], peek(Queues)[4], peek(Queues)[5],
         peek(Queues)[6], peek(Queues)[7]);
 dbgprint("[%d %d %d %d %d %d %d %d]\n", peek(Queues + 1)[0],
         peek(Queues + 1)[1], peek(Queues + 1)[2], peek(Queues + 1)[3],
         peek(Queues + 1)[4], peek(Queues + 1)[5], peek(Queues + 1)[6],
         peek(Queues + 1)[7]);

 if (searchInput(testInput, &Queues[0], 0)) {
      dbgprint("%s", "true\n");
 } else
      dbgprint("%s", "false\n");
  }
