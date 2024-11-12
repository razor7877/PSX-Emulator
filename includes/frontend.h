#pragma once

#include <stdint.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct
{
	float x;
	float y;
} Vec2;

typedef struct
{
	int x;
	int y;
} iVec2;

typedef struct
{
	union
	{
		float x;
		float r;
	};
	union
	{
		float y;
		float g;
	};
	union
	{
		float z;
		float b;
	};
} Vec3;

typedef struct
{
	union
	{
		int x;
		int r;
	};
	union
	{
		int y;
		int g;
	};
	union
	{
		int z;
		int b;
	};
} iVec3;

typedef struct
{
	Vec2 position;
	Vec3 color;
} Vertex;

typedef struct
{
	Vertex v1;
	Vertex v2;
	Vertex v3;
} Triangle;

typedef struct
{
	Vertex v1;
	Vertex v2;
	Vertex v3;
	Vertex v4;
} Quad;

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
	GLuint blit_shader;
	RenderTarget* current_render_target;
	RenderTarget psx_render_target;
	RenderTarget vram_render_target;
	Vec2 window_size;
} Frontend;

extern Frontend frontend_state;

/// <summary>
/// Draws a pixel into the PSX internal framebuffer
/// </summary>
/// <param name="x_coord">The x position of the pixel</param>
/// <param name="y_coord">The y position of the pixel</param>
/// <param name="red">The red color value</param>
/// <param name="green">The green color value</param>
/// <param name="blue">The blue color value</param>
void draw_pixel(uint16_t x_coord, uint16_t y_coord, uint8_t red, uint8_t green, uint8_t blue);
void draw_triangle(Triangle triangle);
void draw_quad(Quad quad);

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static int setup_glfw();
static GLuint compile_shader(const char* v_shader, const char* f_shader);
static void create_framebuffer(RenderTarget* render_target);

/// <summary>
/// Resizes the PSX framebuffer
/// </summary>
/// <param name="new_size">The new size for the PSX framebuffer</param>
void resize_psx_framebuffer(Vec2 new_size);
static void resize_framebuffer(RenderTarget* render_target, Vec2 new_size);

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
