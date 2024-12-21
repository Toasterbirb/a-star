#pragma once

#include <array>
#include <cmath>
#include <memory>
#include <unordered_set>

#include "Random.hpp"
#include "Scene.hpp"
#include "ShaderCollection.hpp"
#include "ShaderRef.hpp"
#include "Text.hpp"

#include "Tile.hpp"

// class for holding the game state
class game
{
public:
	game();

	// the scene that'll hold all of the game objects
	birb::scene scene;

	void update();
	void reset();
	bool is_done() const;

private:
	void generate_map();
	std::vector<tile*> get_tile_neighbors(const birb::vec2<i16> tile_to_check);
	void update_weight_texts();

	static constexpr u8 map_size = 16;
	std::array<std::array<u8, map_size>, map_size> walls = {
		std::array<u8,16>	{0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,1,0,0,0,3,0,0,0,0,0},
							{0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0},
							{0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,1},
							{0,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1},
							{0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
							{0,0,0,0,1,1,1,1,1,1,0,0,1,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,1,1,1,1,0,0,1,0,0,0,0,0,0,0,0},
							{0,1,0,2,0,0,0,1,0,0,0,0,0,0,0,0},
							{0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
							{0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
	};

	// the multiplier for tile positions
	// used when calculating g and h costs
	static constexpr i16 world_scale = 10;

	birb::shader_ref white = birb::shader_collection::register_shader("texture", "white");
	birb::shader_ref black = birb::shader_collection::register_shader("texture", "black");
	birb::shader_ref green = birb::shader_collection::register_shader("texture", "green");
	birb::shader_ref red = birb::shader_collection::register_shader("texture", "red");
	birb::shader_ref blue = birb::shader_collection::register_shader("texture", "blue");

	std::array<std::array<tile*, map_size>, map_size> tiles;
	std::array<birb::text*, map_size> weight_text_rows;
	std::array<std::string, map_size> weight_text_row_strings;
	std::unordered_set<tile*> open_set;
	std::unordered_set<tile*> closed_set;

	// these need to be found at the initialization phase
	birb::vec2<i16> start_location;
	birb::vec2<i16> end_location;

	bool road_found{false};
};
