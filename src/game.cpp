#include <entt.hpp>

#include "Entity.hpp"
#include "Transform.hpp"
#include "Vector.hpp"

#include "Font.hpp"
#include "FontManager.hpp"
#include "Game.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "ShaderCollection.hpp"
#include "ShaderRef.hpp"
#include "ShaderSprite.hpp"
#include "Text.hpp"
#include "Tile.hpp"
#include "Transformer.hpp"

game::game()
{
	birb::shader_sprite sprite(white);

	// create a grid of tiles that'll be rendered in screenspace
	constexpr u8 grid_size = 16;
	constexpr u8 grid_dimensions = 64;
	const u32 top_pos = 1080;
	for (u8 i = 0; i < grid_size; ++i)
	{
		for (u8 j = 0; j < grid_size; ++j)
		{
			birb::entity tile_entity = scene.create_entity("Tile", birb::component::transform);
			tile_entity.get_component<birb::transform>().position.x = i * 34 + 1024;
			tile_entity.get_component<birb::transform>().position.y = top_pos - (j * 34 + 128);
			tile_entity.get_component<birb::transform>().local_scale.x = 30;
			tile_entity.get_component<birb::transform>().local_scale.y = 30;

			tile_entity.add_component(sprite);
			tiles[j][i] = &tile_entity.get_component<tile>();

			tile t;
			t.state = static_cast<tile_state>(walls[j][i]);

			switch (t.state)
			{
				case tile_state::obstacle:
					tile_entity.get_component<birb::shader_sprite>().set_shader(black);
					break;

				case tile_state::start:
					tile_entity.get_component<birb::shader_sprite>().set_shader(green);
					t.distance_to_start = 0;
					t.distance_to_end = 0;
					start_location = { i, j };
					break;

				case tile_state::end:
					tile_entity.get_component<birb::shader_sprite>().set_shader(red);
					t.distance_to_start = 0;
					t.distance_to_end = 0;
					end_location = { i, j };
					break;

				default:
					break;
			}
			tile_entity.add_component(t);
		}
	}

	// birb::font_manager font_manager;
	// birb::font mononoki_32 = font_manager.load_font("assets/mononoki-Regular.ttf", 32);
	// birb::text numbers("000", mononoki_32, {512, 1024, 0});

	// std::string text = "";
	// for (u16 i = 0; i < 25 * 3; ++i)
	// 	text += '0';

	// for (u16 i = 0; i < 25; ++i)
	// {
	// 	birb::entity text_entity = scene.create_entity("Text");

	// 	for (size_t i = 0; i < text.size(); ++i)
	// 		text[i] = i % 10 + '0';

	// 	numbers.set_text(text);
	// 	numbers.position.y -= 50;
	// 	text_entity.add_component(numbers);
	// }

	// add the tiles around the starting tile as the first tiles in the
	tileset.insert(tiles[start_location.y][start_location.x - 1]);
	tileset.insert(tiles[start_location.y][start_location.x + 1]);
	tileset.insert(tiles[start_location.y - 1][start_location.x]);
	tileset.insert(tiles[start_location.y + 1][start_location.x]);
	tileset.insert(tiles[start_location.y + 1][start_location.x + 1]);
	tileset.insert(tiles[start_location.y + 1][start_location.x - 1]);
	tileset.insert(tiles[start_location.y - 1][start_location.x + 1]);
	tileset.insert(tiles[start_location.y - 1][start_location.x - 1]);
}

void game::update()
{
	// const auto view = scene.registry.view<birb::text>();
	// birb::random rng;
	// rng.seed(counter++);

	// for (const auto entity : view)
	// {
	// 	birb::text& t = view.get<birb::text>(entity);

	// 	std::string str = t.get_text();
	// 	for (size_t i = 0; i < str.size(); ++i)
	// 		str[i] = rng.next() % 10 + '0';

	// 	t.set_text(str);
	// }
}
