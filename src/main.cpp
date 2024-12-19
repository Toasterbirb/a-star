#include "Camera.hpp"
#include "CameraInfoOverlay.hpp"
#include "PerformanceOverlay.hpp"
#include "Profiling.hpp"
#include "Renderer.hpp"
#include "RendererOverlay.hpp"
#include "Scene.hpp"
#include "Timestep.hpp"
#include "Window.hpp"

int main(void)
{
	birb::window window("Playground", birb::vec2<int>(1280, 720));
	window.init_imgui();

	birb::timestep timestep;
	birb::renderer renderer;
	birb::camera camera(window.size());
	birb::scene scene;

	birb::overlay::performance perf_widget(timestep);
	birb::overlay::renderer_overlay render_widget(renderer);
	birb::overlay::camera_info camera_widget(camera);

	renderer.debug.alloc_world(window);
	renderer.debug.alloc_entity_editor(scene);

	while (!window.should_close())
	{
		while (window.inputs_available())
		{
			birb::input input = window.next_input();

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

		window.clear();

		/////////////////////
		// Draw stuff here //
		/////////////////////

		{
			PROFILER_SCOPE_RENDER("Debug widgets");
			perf_widget.draw();
			render_widget.draw();
			camera_widget.draw();
		}

		window.flip();

		window.poll();
		timestep.step();
	}
}
