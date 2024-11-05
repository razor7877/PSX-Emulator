#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct
{
	float x;
	float y;
} Vec2;

typedef struct
{
	GLuint framebuffer;
	GLuint depth_stencil_buffer;
	GLuint render_texture;
	GLuint draw_buffer;
	Vec2 size;
} RenderTarget;

typedef struct
{
	GLFWwindow* window;
	GLuint solid_shader;
	RenderTarget psx_render_target;
	Vec2 window_size;
} frontend;

extern frontend frontend_state;

/// <summary>
/// Draws a pixel into the PSX internal framebuffer
/// </summary>
/// <param name="x_coord">The x position of the pixel</param>
/// <param name="y_coord">The y position of the pixel</param>
/// <param name="red">The red color value</param>
/// <param name="green">The green color value</param>
/// <param name="blue">The blue color value</param>
void draw_pixel(uint16_t x_coord, uint16_t y_coord, uint8_t red, uint8_t green, uint8_t blue);

/// <summary>
/// Starts the frontend
/// </summary>
/// <returns>0 if it started successfully, or another value on failure</returns>
int start_interface();

/// <summary>
/// Updates the frontend (query user input, refresh etc.)
/// </summary>
/// <returns>0 if the frontend should stay open, 1 if it should be closed</returns>
int update_interface();

/// <summary>
/// Stops the frontend
/// </summary>
void stop_interface();
