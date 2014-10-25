#include "block.h"

Block::Block(int x,int y) {
	oldstate = bsDead;
	newstate = bsDead;

	this->x = x;
	this->y = y;
}

BlockState Block::GetState() { // returns old state
	return oldstate;
}
void Block::SetState(BlockState value) {
	newstate = value; // set new state, don't apply yet
}

bool Block::Apply() {
	if(oldstate != newstate) {
		oldstate = newstate;
		return true;
	} else {
		return false;
	}
}
