#ifndef FIGHTER_H_
#define FIGHTER_H_

typedef struct {
	uint8_t player;		//Player controlling the character
	int hp;			//Health Points
	int ac;			//Armor Class
	int x,y;		//X and Y positions for the character
	int state;		//Character's state(with universal states defined in the enum in char.h)
	int stateFrames;	//How long the character is in this state (-1 if in state indefinitely).
	int nextState;		//State to jump to when stateFrames = 0;
	int numStates;		//How many states the player has total;

	int* hitboxes[2];	//Currently active hitboxes
	int* hurtboxes[2];	//Currently active hurtboxes
	void (*updates[])();	//Update functions for each state
} fighter_t;

void damage(char_t* character, int health);

#endif
