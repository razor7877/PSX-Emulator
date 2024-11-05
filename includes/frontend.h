#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct
{
	GLFWwindow* window;
} frontend;

extern frontend frontend_state;

int start_interface();
int update_interface();
void stop_interface();
