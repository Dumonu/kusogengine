#include "SDL2/SDL_thread.h"
#include "joyState.h"
#include "settings.h"
#include <global.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fighter.h>


void changeState(fighter_t* fighter, int state){
	fighter->state = state;
}
void changeHealth(fighter_t* fighter, int health){
	fighter->hp = health;
}
void damage(fighter_t* fighter, int health){
	changeHealth(fighter, fighter->hp - health);
}
void Update(fighter_t* fighter){
	fighter->updates[fighter->state]();
} 

