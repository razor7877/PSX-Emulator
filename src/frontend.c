#include <stdbool.h>

#include "frontend.h"
#include "logging.h"

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

frontend frontend_state = {
	.window = NULL
};

static int setup_glfw()
{
	// Initialize GLFW context
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	frontend_state.window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PSX Emulator", NULL, NULL);
	if (frontend_state.window == NULL)
	{
		log_error("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(frontend_state.window);

	// Check if GLAD loaded successfully
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		log_error("Failed to initialize GLAD\n");
		return -1;
	}

	return 0;
}

int start_interface()
{
	if (setup_glfw() != 0)
		return -1;

	return 0;
}

int update_interface()
{
	// Swap new frame and poll GLFW for inputs
	glfwSwapBuffers(frontend_state.window);
	glfwPollEvents();

	if (glfwWindowShouldClose(frontend_state.window))
		return 1;

	return 0;
}

void stop_interface()
{
	glfwDestroyWindow(frontend_state.window);
	glfwTerminate();
}
