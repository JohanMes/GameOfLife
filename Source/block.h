#ifndef BLOCK_H
#define BLOCK_H

#include <vector>

enum BlockState {
	bsAlive,
	bsDead,
	bsWall,
	bsFood,
};

class Block {
	BlockState newstate;
	BlockState oldstate;
	
	public:
		Block(int x,int y);
		
		int x;
		int y;
		std::vector<Block*> neighbors; // doesn't change, so cache
	
		BlockState GetState();
		void SetState(BlockState value);
		bool Apply(); // apply new state
};

#endif
