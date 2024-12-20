#pragma once

#include "Vector.hpp"

enum class tile_state : u8
{
	unexplored = 0,
	obstacle = 1,
	start = 2,
	end = 3,
	explored = 4,
	seen = 5,
	route = 6
};

struct tile
{
	tile() {};

	u16 distance_to_start{0};
	u16 distance_to_end{0};

	__attribute__((const))
	u16 weight() { return distance_to_start + distance_to_end; }

	// start all tiles as "unexplored"
	// the state of the tile will also change the color of the tile
	tile_state state = tile_state::unexplored;
};
