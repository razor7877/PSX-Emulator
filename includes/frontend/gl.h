#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gpu.h>

#define PSX_RT frontend_state.psx_render_target
#define VRAM_RT frontend_state.vram_render_target

/// <summary>
/// Functions and state for implementing the GPU operations using the OpenGL graphics API
/// </summary>

/// <summary>
/// Represents the UV data for a primitive
/// </summary>
typedef struct
{
	/// <summary>
	/// The position of the CLUT in VRAM
	/// </summary>
	Vec2 clut_position;

	/// <summary>
	/// The x coordinate of the texture page, in 64 halfword steps
	/// </summary>
	uint8_t texture_page_x_base;

	/// <summary>
	/// The y coordinate of the texture page, in 256 line steps
	/// </summary>
	uint8_t texture_page_y_base;

	/// <summary>
	/// The state of the semi transparency flag
	/// </summary>
	uint8_t semi_transparency;

	/// <summary>
	/// The texture page color mode
	/// </summary>
	TexturePageColors texture_page_colors;
} UVData;

/// <summary>
/// Represents a single vertex (position, color, UV coordinates)
/// </summary>
typedef struct
{
	Vec2 position;
	Vec3 color;
	Vec2 uv;
} Vertex;

/// <summary>
/// Represents a triangle made of three vertices
/// </summary>
typedef struct
{
	Vertex v1;
	Vertex v2;
	Vertex v3;
	UVData uv_data;
} Triangle;

/// <summary>
/// Represents a quad made of four vertices
/// </summary>
typedef struct
{
	Vertex v1;
	Vertex v2;
	Vertex v3;
	Vertex v4;
	UVData uv_data;
} Quad;

/// <summary>
/// A render target which can be drawn to
/// </summary>
typedef struct
{
	/// <summary>
	/// The OpenGL handle to the framebuffer
	/// </summary>
	GLuint framebuffer;

	/// <summary>
	/// The OpenGL handle to the renderbuffer
	/// </summary>
	GLuint depth_stencil_buffer;

	/// <summary>
	/// The OpenGL handle to the framebuffer texture
	/// </summary>
	GLuint render_texture;

	/// <summary>
	/// The draw buffer that specifies the output color attachment
	/// </summary>
	GLuint draw_buffer;

	/// <summary>
	/// The size of the framebuffer, in pixels
	/// </summary>
	Vec2 size;
} RenderTarget;

typedef struct
{
	/// <summary>
	/// A pointer to the GLFW window handle
	/// </summary>
	GLFWwindow* window;

	bool fullscreen_mode;

	/// <summary>
	/// The OpenGL handle for the gouraud/flat shading shader
	/// </summary>
	GLuint color_shader;

	/// <summary>
	/// The OpenGL handle for the textured shader
	/// </summary>
	GLuint texture_shader;

	/// <summary>
	/// The OpenGL handle for the blit to quad shader
	/// </summary>
	GLuint blit_shader;

	/// <summary>
	/// The current render target that should be output to the screen
	/// </summary>
	RenderTarget* current_render_target;

	/// <summary>
	/// The render target for the PSX internal framebuffer
	/// </summary>
	RenderTarget psx_render_target;

	/// <summary>
	/// The render target for the VRAM view
	/// </summary>
	RenderTarget vram_render_target;

	/// <summary>
	/// The current size of the window, in pixels
	/// </summary>
	Vec2 window_size;

	GLuint vram_tex;
	GLuint blit_quad_vao;
	GLuint blit_quad_vbo;
	GLuint blit_quad_texture_bo;
} Frontend;

extern Frontend frontend_state;

void start_gl_state();
void reset_gl_state();

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
void draw_textured_triangle(Triangle triangle);

void draw_quad(Quad quad);
void draw_textured_quad(Quad quad);

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static int setup_glfw();

/// <summary>
/// Compiles a shader and returns the OpenGL handle
/// </summary>
/// <param name="v_shader">The code of the vertex shader</param>
/// <param name="f_shader">The code of the fragment shader</param>
/// <returns>An OpenGL handle to the shader, or 0 if the compilation failed</returns>
static GLuint compile_shader(const char* v_shader, const char* f_shader);

/// <summary>
/// Creates a new framebuffer
/// </summary>
/// <param name="render_target">The render target in which the state should be stored</param>
static void create_framebuffer(RenderTarget* render_target);

static void delete_framebuffer(RenderTarget* render_target);

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
