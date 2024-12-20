#include "Camera.hpp"
#include "CameraInfoOverlay.hpp"
#include "PerformanceOverlay.hpp"
#include "Profiling.hpp"
#include "Renderer.hpp"
#include "RendererOverlay.hpp"
#include "Scene.hpp"
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

	// tell the renderer which scene to render
	renderer.set_scene(game.scene);

	// allocate debug views
	renderer.debug.alloc_world(window);
	renderer.debug.alloc_entity_editor(game.scene);

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

					default:
						break;
				}
			}
		}

		game.update();

		// clear the backbuffer
		window.clear();

		/////////////////////
		// Draw stuff here //
		/////////////////////

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
