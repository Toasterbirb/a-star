#include "Camera.hpp"
#include "CameraInfoOverlay.hpp"
#include "PerformanceOverlay.hpp"
#include "Profiling.hpp"
#include "Renderer.hpp"
#include "RendererOverlay.hpp"
#include "Timer.hpp"
#include "Timestep.hpp"
#include "Window.hpp"

#include "Game.hpp"

int main(void)
{
	// tell the shader class where to look for custom shaders
	birb::shader::shader_src_search_paths.push_back("assets/shaders");

	// create a new window and initialize Dear ImGui
	birb::window window("Playground", birb::vec2<int>(1280, 720));
	window.init_imgui();

	// match the window background color with the obstacle color
	window.set_background_color(0x16161D);

	// construct misc. things like the renderer, timestep, camera etc.
	birb::timestep timestep;
	birb::renderer renderer;
	birb::camera camera(window.size());
	camera.position.z = 45;

	// debugging overlays
	birb::overlay::performance perf_widget(timestep);
	birb::overlay::renderer_overlay render_widget(renderer);
	birb::overlay::camera_info camera_widget(camera);

	// create the game state
	game game;

	birb::timer pathfinder_tick_timer(0.1);

	birb::timer new_map_timer(1.0);
	bool waiting_for_new_map{false};

	// tell the renderer which scene to render
	renderer.set_scene(game.scene);

	// allocate debug views
	renderer.debug.alloc_world(window);
	renderer.debug.alloc_entity_editor(game.scene);

	bool paused = false;

	// the main loop that runs until the window wants to be closed
	while (!window.should_close())
	{
		// process inputs until the window doesn't have any more
		// inputs queued up
		while (window.inputs_available())
		{
			// get the next input in the queue
			birb::input input = window.next_input();

			// handle cases where the key is pressed down (no need to release)
			if (input.state == birb::input::action::key_down)
			{
				switch (input.key)
				{
					case (birb::input::keycode::escape):
						window.quit();

					case (birb::input::keycode::space):
						paused = !paused;
						break;

					default:
						break;
				}
			}
		}

		// progress timers
		if (!paused)
		{
			pathfinder_tick_timer.tick(timestep.deltatime());
			new_map_timer.tick(timestep.deltatime());
		}

		// update the game state (path finding)
		if (pathfinder_tick_timer.done())
		{
			// progress the path finding
			game.update();

			// reset the pathfinding timer
			pathfinder_tick_timer.reset();

			// if the path was found, start waiting for a map reset
			if (game.is_done() && !waiting_for_new_map)
			{
				new_map_timer.reset();
				waiting_for_new_map = true;
			}
		}

		// if we found a route, wait a little bit before generating a new map
		if (waiting_for_new_map && new_map_timer.done())
		{
			game.reset();
			waiting_for_new_map = false;
		}

		// clear the backbuffer
		window.clear();

		/////////////////////
		// Draw stuff here //
		/////////////////////

		// draw entities that should be rendered into the backbuffer
		renderer.draw_entities(camera, window.size());

		{
			PROFILER_SCOPE_RENDER("Debug widgets");
			perf_widget.draw();
			render_widget.draw();
			camera_widget.draw();
		}

		// flip frame buffers to display the frame that was just rendered
		window.flip();

		// poll for window events and engine reserved keybinds
		window.poll();

		// tell the timestep that we are moving on to the next frame
		timestep.step();
	}
}
