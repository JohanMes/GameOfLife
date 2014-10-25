#include <stdlib.h>
#include <omp.h>
#include <windows.h>
#include <math.h>

#include "map.h"

Map::Map(int xsize,int ysize,int blocksize) {
	this->blocksize = blocksize;
	SetSize(xsize,ysize);
	
	// Set timers
	__int64 frequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	secpercount = 1.0f/(float)frequency;
	QueryPerformanceCounter((LARGE_INTEGER*)&oldtime);
	time = 0;
	modtime = 0;
	olditerate = 0;
	iteration = 0;
	done = false;
}
Map::~Map() {
	for(int i = 0;i < xsize;i++) {
		for(int j = 0;j < ysize;j++) {
			delete field[i][j];
		}
	}
}

void Map::SetSize(int xsize,int ysize) {
	
	this->xsize = xsize;
	this->ysize = ysize;
	
	redrawlist.reserve(xsize * ysize); // realloc once

	field.resize(xsize);
	
	#pragma omp parallel for
	for(int i = 0;i < xsize;i++) { // rows
		field[i].resize(ysize); // columns
		for(int j = 0;j < ysize;j++) {
			field[i][j] = new Block(i,j);
		}
	}
	
	#pragma omp parallel for
	for(int i = 0;i < xsize;i++) {
		for(int j = 0;j < ysize;j++) {
			field[i][j]->neighbors = GetNeighborList(i,j); // calculate once
		}
	}
}

Block* Map::Get(int x,int y) {
	if(x < 0) {
		x = xsize + x;
	} else {
		x = x % xsize;
	}
	if(y < 0) {
		y = ysize + y;
	} else {
		y = y % ysize;
	}
	return field[x][y];
}

std::vector<Block*> Map::GetNeighborList(int x,int y) { // slow
	std::vector<Block*> result;
	result.reserve(8);
	
	// below
	result.push_back(Get(x - 1,y - 1));
	result.push_back(Get(x - 1,y));
	result.push_back(Get(x - 1,y + 1));
	
	// left/right
	result.push_back(Get(x,y - 1));
	result.push_back(Get(x,y + 1));
	
	// above
	result.push_back(Get(x + 1,y - 1));
	result.push_back(Get(x + 1,y));
	result.push_back(Get(x + 1,y + 1));
	
	return result;
}

int Map::GetCountOf(BlockState state,std::vector<Block*> neighbors) {
	int result = 0;
	for(unsigned int i = 0;i < neighbors.size();i++) {
		if(neighbors[i]->GetState() == state) {
			result++;
		}
	}
	return result;
}

void Map::Invalidate() {
	redrawlist.clear();
	for(int x = 0;x < xsize;x++) {
		for(int y = 0;y < ysize;y++) {
			redrawlist.push_back(field[x][y]);
		}
	}
}

bool Map::Apply() {
	bool result = false;
	redrawlist.clear();

	// Apply new state
//	#pragma omp parallel for
	for(int x = 0;x < xsize;x++) {
		for(int y = 0;y < ysize;y++) {
			if(field[x][y]->Apply()) {
				result = true;
				redrawlist.push_back(field[x][y]);
			}
		}
	}
	return result;
}

bool Map::Iterate() {
	
	if(done) {
		return false; // stop processing
	}
				
	QueryPerformanceCounter((LARGE_INTEGER*)&newtime);

	// Update FPS once every 5 seconds
	time += (newtime - oldtime) * secpercount;
	modtime += (newtime - oldtime) * secpercount;
	if(modtime > 0.1f) { // seconds
		fiterate = (iteration - olditerate) / (float)modtime;
		olditerate = iteration;
		modtime = 0.0f;
	}
	oldtime = newtime;
	
	// Interact
	#pragma omp parallel for
	for(int x = 0;x < xsize;x++) {
		for(int y = 0;y < ysize;y++) {
			Block* thisblock = field[x][y];

			if(thisblock->GetState() == bsAlive) {
				
				// Any live cell with fewer than two live neighbours dies, as if caused by under-population.
				// Any live cell with two or three live neighbours lives on to the next generation.
				// Any live cell with more than three live neighbours dies, as if by overcrowding.
				
				int alivecount = GetCountOf(bsAlive,thisblock->neighbors);
			//	int foodcount = GetCountOf(bsFood,thisblock->neighbors);
			//	if(foodcount > 0) {
//					if(alivecount < 1) {
//						thisblock->SetState(bsDead);
//					} else if(alivecount > 7) {
//						thisblock->SetState(bsDead);
//					}
//				} else { // no food? stricter rules
					if(alivecount < 2) {
						thisblock->SetState(bsDead);
					} else if(alivecount > 3) {
						thisblock->SetState(bsDead);
					}
			//	}
			} else if(thisblock->GetState() == bsDead) {
				
				// Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
				
				int alivecount = GetCountOf(bsAlive,thisblock->neighbors);
				if(alivecount == 3) {
					thisblock->SetState(bsAlive);
				}
			}
		}
	}
	
	bool changed = Apply();
	if(!changed) {
		done = true;
	} else {
		iteration++; // don't count this iteration otherwise, nothing happened
	}
	
	return true;
}

void Map::AddColony(int x,int y) {
	
	// 101
	// 010
	// 101
	
	Get(x-1,y+1)->SetState(bsAlive);
	Get(x+1,y+1)->SetState(bsAlive);
	
	Get(x,y)->SetState(bsAlive);
	
	Get(x-1,y-1)->SetState(bsAlive);
	Get(x+1,y-1)->SetState(bsAlive);
}
