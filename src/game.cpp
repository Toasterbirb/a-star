#include <format>

#include "Entity.hpp"
#include "Font.hpp"
#include "FontManager.hpp"
#include "Game.hpp"
#include "Math.hpp"
#include "Shader.hpp"
#include "ShaderCollection.hpp"
#include "ShaderRef.hpp"
#include "ShaderSprite.hpp"
#include "Text.hpp"
#include "Tile.hpp"
#include "Transform.hpp"
#include "Transformer.hpp"
#include "Vector.hpp"

// randomness source
static birb::random rng;

game::game()
{
	// generate a random map
	generate_map();

	// shader sprite used for rendering the 2D tiles
	birb::shader_sprite sprite(s_unexplored);

	// create a grid of tiles that'll be rendered in screenspace
	constexpr u8 grid_dimensions = 64;
	constexpr u32 top_pos = 1080;

	constexpr u8 tile_size = 64;
	constexpr u8 tile_pos_offset = 64;

	for (u8 i = 0; i < map_size; ++i)
	{
		for (u8 j = 0; j < map_size; ++j)
		{
			// create a new entity for each tile with a transform component
			birb::entity tile_entity = scene.create_entity(birb::component::transform);

			// set the position and scale
			tile_entity.get_component<birb::transform>().position.x = i * tile_pos_offset + 512;
			tile_entity.get_component<birb::transform>().position.y = top_pos - (j * tile_pos_offset + tile_pos_offset);
			tile_entity.get_component<birb::transform>().local_scale = { tile_size, tile_size, 0 };

			// add the shader sprite as a component to the tile entity
			tile_entity.add_component(sprite);

			// create a new tile and set its position (scaled coords), coordinates and state
			tile t;
			t.position = birb::vec2<i16>(i * world_scale, j * world_scale);
			t.coordinates = birb::vec2<i16>(i, j);
			t.state = static_cast<tile_state>(walls[j][i]);

			// add the tile to the entity as a component
			tile_entity.add_component(t);

			// get a pointer to the tile component and store it to the tile array
			tiles[j][i] = &tile_entity.get_component<tile>();
		}
	}

	// construct a font manager and load a font in two sizes with it
	birb::font_manager font_manager;
	birb::font mononoki_28 = font_manager.load_font("assets/mononoki-Regular.ttf", 28);
	birb::font mononoki_14 = font_manager.load_font("assets/mononoki-Regular.ttf", 14);

	// height offset for text
	// this makes setting positions a bit easier, since the (0,0) point is
	// at the bottom left corner
	size_t text_height_offset{0};

	// create the g_cost texts
	for (i16 i = 0; i < walls.size(); ++i)
	{
		// create the text entity
		birb::entity text_entity = scene.create_entity();

		// construct a text component and add it to the text entity as a component
		birb::text row("000", mononoki_28, birb::vec3<f32>(489, 1004 - text_height_offset, 0), 0x1F1F28);
		text_entity.add_component(row);
		text_height_offset += tile_pos_offset;

		// get a pointer to the text component and store it
		weight_text_rows.at(i) = &text_entity.get_component<birb::text>();
	}

	// create the f_cost/h_cost texts
	text_height_offset = 0;
	for (i16 i = 0; i < walls.size(); ++i)
	{
		// create the text entity
		birb::entity text_entity = scene.create_entity();

		// construct a text component and add it to the text entity as a component
		birb::text row("000 000 ", mononoki_14, birb::vec3<f32>(484, 1030 - text_height_offset, 0), 0x16161D);
		text_entity.add_component(row);
		text_height_offset += tile_pos_offset;

		// get a pointer to the text component and store it
		f_and_h_cost_text_rows.at(i) = &text_entity.get_component<birb::text>();
	}

	// reset the map and game_state to generate a new level
	reset();
}

void game::generate_map()
{
	// generate a randomize map with the drunk man algorithm

	// first fill the map with obstacles
	for (auto& map_row : walls)
		map_row.fill(1);

	// choose a random starting point and change the tile state accordingly
	start_location = birb::vec2<i16>(rng.range(1, map_size - 1), rng.range(1, map_size - 1));
	walls[start_location.y][start_location.x] = static_cast<u8>(tile_state::start);

	// wander around randomly for a set amount of tiles depending on the map size
	const size_t wandering_count = (map_size * map_size);

	// start wandering from the start location
	birb::vec2<i16> current_location = start_location;

	for (size_t i = 0; i < wandering_count; ++i)
	{
		const birb::vec2<i16> direction(rng.range(-1, 1), rng.range(-1, 1));

		// make sure that the new location would be a legal one
		const birb::vec2<i16> new_location = current_location + direction;

		if (new_location.x < 0 || new_location.x >= map_size)
			continue;

		if (new_location.y < 0 || new_location.y >= map_size)
			continue;

		// make sure that we don't end up at the starting location
		// the search for the path would be a bit boring in that case
		if (new_location == start_location)
			continue;

		// set the new location and update the walls
		current_location = new_location;
		walls[new_location.y][new_location.x] = static_cast<u8>(tile_state::unexplored);
	}

	// set the last tile we end up to be the goal
	end_location = current_location;
	walls[current_location.y][current_location.x] = static_cast<u8>(tile_state::end);
}

std::vector<tile*> game::get_tile_neighbors(const birb::vec2<i16> tile_to_check)
{
	// a lambda that checks if a tile is an explorable neighbor or not
	// a not-explorable tile would be a tile out-of-bounds or an obstacle
	const auto is_explorable_tile = [&](const birb::vec2<i16> tile_coords) -> bool
	{
		// check if the tile is within the bounds of the area we are exploring
		if (tile_coords.x < 0 || tile_coords.y < 0)
			return false;

		if (tile_coords.x >= walls.size() || tile_coords.y >= walls.size())
			return false;

		// check if the tile is an obstacle
		if (tiles[tile_coords.y][tile_coords.x]->state == tile_state::obstacle)
			return false;

		return true;
	};

	std::vector<tile*> neighbors;

	// loop through a 3x3 grid around the tile we are checking
	for (i8 i = -1; i < 2; ++i)
	{
		for (i8 j = -1; j < 2; ++j)
		{
			// skip the current tile (that we are checking)
			if (!i && !j) continue;

			// check if the tile is explorable (i.e. in the bounds and not an obstacle)
			// if it is, add it to the neighbor list
			if (is_explorable_tile(birb::vec2<i16>(tile_to_check.x - i, tile_to_check.y - j)))
				neighbors.emplace_back(tiles[tile_to_check.y - j][tile_to_check.x - i]);
		}
	}

	return neighbors;
}

void game::update_weight_texts()
{
	// update the weight texts
	for (size_t i = 0; i < weight_text_rows.size(); ++i)
		weight_text_rows.at(i)->set_text(weight_text_row_strings.at(i));

	for (size_t i = 0; i < f_and_h_cost_text_rows.size(); ++i)
		f_and_h_cost_text_rows.at(i)->set_text(f_and_h_cost_text_row_strings.at(i));
}

void game::update()
{
	// treat the update as a while loop
	// its called on every frame from the main loop at main.cpp

	// pick a tile from the open_set that has the lowest f_cost
	// and set it to be the new current tile

	// the the current_tile_ptr to the first tile in the open_set
	birb::ensure(!open_set.empty());
	tile* current_tile_ptr = *open_set.begin();

	// loop through the open_set
	for (tile* const t : open_set)
	{
		// check if the current tile has a lower f_cost than the currently
		// lowest f_cost found so far
		if (t->f_cost() < current_tile_ptr->f_cost())
			current_tile_ptr = t;
	}

	// get the coordinates of the new current tile
	const birb::vec2<i16> current_tile = current_tile_ptr->coordinates;

	// if we have reached the goal, update the tile states to mark the route
	// from start to finish
	if (current_tile == end_location)
	{
		if (road_found)
			return;

		road_found = true;

		// start from the end tile and work towards the start tile
		tile* t = tiles[end_location.y][end_location.x];
		while (t != tiles[start_location.y][start_location.x])
		{
			t->state = tile_state::route;
			t = t->predecessor;
			birb::ensure(t != nullptr, "tile doesn't have a predecessor");
		}

		// also mark the start tile as part of the route
		tiles[start_location.y][start_location.x]->state = tile_state::route;

		// find all entities that have a tile and shader_sprite component on them
		const auto tile_view = scene.registry.view<tile, birb::shader_sprite>();

		// loop through the entities
		for (const auto tile_entity : tile_view)
		{
			// get the tile and shader_sprite components from the entity
			const tile& t = tile_view.get<tile>(tile_entity);
			birb::shader_sprite& s = tile_view.get<birb::shader_sprite>(tile_entity);

			// if the tile is not part of the route, skip it
			if (t.state != tile_state::route)
				continue;

			// set the shader of the tile to the route shader
			s.set_shader(s_route);
		}

		// don't do any further processing since we have found
		// and visualized the route to the goal
		return;
	}

	// add the current tile to the closed set and remove it from the open set
	open_set.erase(current_tile_ptr);
	closed_set.insert(current_tile_ptr);

	// get neighbors of the current tile
	const std::vector<tile*> current_tile_neighbors = get_tile_neighbors(current_tile);
	birb::ensure(!current_tile_neighbors.empty(), "coudln't find any neighbors for the current tile");

	// update the g_costs and h_costs of the neighbors and set
	// their predecessor to the current tile if needed
	//
	// also add the neighbors to the open set if they aren't there already
	// (unless they are in the closed set)
	for (tile* const t : current_tile_neighbors)
	{
		// skip the neighbor if its in the closed_set
		if (closed_set.contains(t))
			continue;

		// calculate an h_cost for the tile
		t->h_cost = birb::vec_distance(t->position, tiles[end_location.y][end_location.x]->position);

		const tile* const current_tile_ptr = tiles[current_tile.y][current_tile.x];
		const f32 new_g_cost = birb::vec_distance(t->position, current_tile_ptr->position) + current_tile_ptr->g_cost;

		// update the neighbor predecessors in cases where this path would be
		// better than the old one
		if (new_g_cost < t->g_cost || !open_set.contains(t))
		{
			t->predecessor = tiles[current_tile.y][current_tile.x];
			t->g_cost = new_g_cost;

			// insert the tile into the open set
			// it shouldn't matter if its already there since it can be in the set only once
			open_set.insert(t);
		}
	}

	// set tile colors and weight texts based on if they are in the sets or not
	// this could probably be optimized further to avoid unnecessary
	// shader switching
	const auto tile_view = scene.registry.view<tile, birb::shader_sprite>();

	for (const auto tile_entity : tile_view)
	{
		birb::shader_sprite& s = tile_view.get<birb::shader_sprite>(tile_entity);
		tile* const t = &tile_view.get<tile>(tile_entity);

		// check what sets the tile is in (it shouldn't be in all of them)
		const bool is_in_open = open_set.contains(t);
		const bool is_in_closed = closed_set.contains(t);

		// if the tile is not yet in either set, skip it
		if (!is_in_open && !is_in_closed)
			continue;

		// set the color of the tile according to its set
		is_in_open ? s.set_shader(s_open) : s.set_shader(s_closed);

		// update the weight text //

		// g_cost
		{
			// find the correct row
			std::string& text_row = weight_text_row_strings.at(t->coordinates.y);

			// calculate the number position in the row
			const size_t num_pos = t->coordinates.x * 4;

			// update the value
			const std::string weight_str = std::format("{:03}", t->f_cost());

			for (u8 i = 0; i < weight_str.size(); ++i)
				text_row.at(num_pos + i) = weight_str.at(i);
		}

		// h_cost and f_cost
		{
			// find the correct row
			std::string& text_row = f_and_h_cost_text_row_strings.at(t->coordinates.y);

			// calculate the number position in the row
			const size_t num_pos = t->coordinates.x * 8;

			// update the value
			const std::string weight_str = std::format("{:03} {:03} ", t->g_cost, t->h_cost);

			for (u8 i = 0; i < weight_str.size(); ++i)
				text_row.at(num_pos + i) = weight_str.at(i);
		}
	}

	// update the text entities
	update_weight_texts();
}

void game::reset()
{
	road_found = false;

	// generate a new map
	generate_map();

	// clear the sets
	closed_set.clear();
	open_set.clear();

	// add the starting tile to the open_set
	open_set.insert(tiles[start_location.y][start_location.x]);

	// update tile states
	for (u8 i = 0; i < map_size; ++i)
		for (u8 j = 0; j < map_size; ++j)
			tiles[j][i]->state = static_cast<tile_state>(walls[j][i]);

	// give the tiles the correct initial shaders and weight values
	const auto tile_view = scene.registry.view<tile, birb::shader_sprite>();
	for (const auto tile_entity : tile_view)
	{
		tile& t = tile_view.get<tile>(tile_entity);
		birb::shader_sprite& s = tile_view.get<birb::shader_sprite>(tile_entity);

		switch (t.state)
		{
			case tile_state::obstacle:
				s.set_shader(s_obstacle);
				break;

			case tile_state::start:
				s.set_shader(s_route);
				t.g_cost = 0;
				t.h_cost = 0;
				break;

			case tile_state::end:
				s.set_shader(s_route);
				t.g_cost = 0;
				t.h_cost = 0;
				break;

			default:
				s.set_shader(s_unexplored);
				break;
		}
	}

	// reset weight texts

	// generate a default text strings full of whitespace
	std::string g_text = "";
	std::string fh_text = "";

	for (i16 i = 0; i < walls.size(); ++i)
	{
		g_text += "    ";
		fh_text += "        ";
	}

	for (std::string& row : weight_text_row_strings)
		row = g_text;

	for (std::string& row : f_and_h_cost_text_row_strings)
		row = fh_text;

	// update text entities
	update_weight_texts();
}

bool game::is_done() const
{
	return road_found;
}
