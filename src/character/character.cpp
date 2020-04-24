#include "SDL2/SDL_thread.h"
#include "joyState.h"
#include "settings.h"
#include <global.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <character.h>


void changeState(char_t* character, int state){
	character->state = state;
}
void changeHealth(char_t* character, int health){
	character->hp = health;
}
void damage(char_t* character, int health){
	changeHealth(character, character->hp - health);
}
void Update(char_t* character){
	character->updates[character->state]();
} 

