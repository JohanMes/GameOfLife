#ifndef MAP_H
#define MAP_H

#include <vector>

#include "block.h"

class Map {

	__int64 newtime;
	__int64 oldtime;
	float secpercount;
	int olditerate;
	float modtime;
	std::vector<std::vector<Block*> > field;

	public:
		Map();
		Map(int xsize,int ysize,int blocksize);
		~Map();
		
		int xsize;
		int ysize;
		int blocksize; // pixels
		int iteration;
		float fiterate;
		float time;
		bool done;
		std::vector<Block*> redrawlist;
	
		Block* Get(int x,int y);
		std::vector<Block*> GetNeighborList(int x,int y);
		int GetCountOf(BlockState state,std::vector<Block*> neighbors);
		void SetSize(int xsize,int ysize);
		void Invalidate();
		bool Apply();
		bool Iterate();
		void AddColony(int x,int y);
};

#endif
