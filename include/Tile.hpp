#pragma once

#include "ShaderRef.hpp"
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

	// distance from starting node
	u16 g_cost = std::numeric_limits<u16>::max();

	// distance from end node
	u16 h_cost{0};

	u16 f_cost() const { return g_cost + h_cost; } // the total cost of the node

	// start all tiles as "unexplored"
	// the state of the tile will also change the color of the tile
	tile_state state = tile_state::unexplored;

	// which tile became before this tile
	tile* predecessor{nullptr};

	// world space position of the tile (can be more than its coordinates)
	birb::vec2<i16> position;

	// the raw coordinates of the tile that map to the tile array in the game state
	birb::vec2<i16> coordinates;
};
