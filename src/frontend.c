#include <stdbool.h>
#include <string.h>

#include "frontend.h"
#include "logging.h"
#include "debug.h"
#include "gpu.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define VRAM_WIDTH 512
#define VRAM_HEIGHT 1024

#define PSX_RT frontend_state.psx_render_target
#define VRAM_RT frontend_state.vram_render_target

Frontend frontend_state = {
	.window = NULL,
    .solid_shader = 0,
    .current_render_target = &frontend_state.psx_render_target,
    .psx_render_target = {
        .framebuffer = 0,
        .depth_stencil_buffer = 0,
        .render_texture = 0,
        .draw_buffer = 0,
        .size = { 512, 240 },
    },
    .vram_render_target = {
        .framebuffer = 0,
        .depth_stencil_buffer = 0,
        .render_texture = 0,
        .draw_buffer = 0,
        .size = { VRAM_WIDTH, VRAM_HEIGHT },
    },
    .window_size = {
        .x = WINDOW_WIDTH,
        .y = WINDOW_HEIGHT,
    },
};

void draw_pixel(uint16_t x_coord, uint16_t y_coord, uint8_t red, uint8_t green, uint8_t blue)
{
    glBindFramebuffer(GL_FRAMEBUFFER, PSX_RT.framebuffer);
    glEnable(GL_SCISSOR_TEST);

    y_coord = PSX_RT.size.y - y_coord;

    glScissor(x_coord, y_coord, 1, 1);
    glClearColor(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_triangle(Triangle triangle)
{
    // Prepare vertices for OpenGL
    float vertices[] = {
        (triangle.v1.position.x * 2.0f) / PSX_RT.size.x - 1.0f,
        (triangle.v1.position.y * 2.0f) / PSX_RT.size.y - 1.0f,
        0.0f,
        (triangle.v2.position.x * 2.0f) / PSX_RT.size.x - 1.0f,
        (triangle.v2.position.y * 2.0f) / PSX_RT.size.y - 1.0f,
        0.0f,
        (triangle.v3.position.x * 2.0f) / PSX_RT.size.x - 1.0f,
        (triangle.v3.position.y * 2.0f) / PSX_RT.size.y - 1.0f,
        0.0f
    };

    glBindFramebuffer(GL_FRAMEBUFFER, PSX_RT.framebuffer);
    glUseProgram(frontend_state.solid_shader);

    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_quad(Quad quad)
{
    // Prepare vertices for OpenGL
    float vertices[] = {
        (quad.v1.position.x * 2.0f) / PSX_RT.size.x - 1.0f,
        (quad.v1.position.y * 2.0f) / PSX_RT.size.y - 1.0f,
        0.0f,
        (quad.v2.position.x * 2.0f) / PSX_RT.size.x - 1.0f,
        (quad.v2.position.y * 2.0f) / PSX_RT.size.y - 1.0f,
        0.0f,
        (quad.v3.position.x * 2.0f) / PSX_RT.size.x - 1.0f,
        (quad.v3.position.y * 2.0f) / PSX_RT.size.y - 1.0f,
        0.0f,
        (quad.v2.position.x * 2.0f) / PSX_RT.size.x - 1.0f,
        (quad.v2.position.y * 2.0f) / PSX_RT.size.y - 1.0f,
        0.0f,
        (quad.v3.position.x * 2.0f) / PSX_RT.size.x - 1.0f,
        (quad.v3.position.y * 2.0f) / PSX_RT.size.y - 1.0f,
        0.0f,
        (quad.v4.position.x * 2.0f) / PSX_RT.size.x - 1.0f,
        (quad.v4.position.y * 2.0f) / PSX_RT.size.y - 1.0f,
        0.0f
    };

    float colors[18] = {
        quad.v1.color.r / 255.0f,
        quad.v1.color.g / 255.0f,
        quad.v1.color.b / 255.0f,
        quad.v2.color.r / 255.0f,
        quad.v2.color.g / 255.0f,
        quad.v2.color.b / 255.0f,
        quad.v3.color.r / 255.0f,
        quad.v3.color.g / 255.0f,
        quad.v3.color.b / 255.0f,
        quad.v2.color.r / 255.0f,
        quad.v2.color.g / 255.0f,
        quad.v2.color.b / 255.0f,
        quad.v3.color.r / 255.0f,
        quad.v3.color.g / 255.0f,
        quad.v3.color.b / 255.0f,
        quad.v4.color.r / 255.0f,
        quad.v4.color.g / 255.0f,
        quad.v4.color.b / 255.0f,
    };

    glBindFramebuffer(GL_FRAMEBUFFER, PSX_RT.framebuffer);
    glUseProgram(frontend_state.solid_shader);

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint color_bo = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &color_bo);

    glBindVertexArray(vao);

    // Send vertices
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Send colors
    glBindBuffer(GL_ARRAY_BUFFER, color_bo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &color_bo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    frontend_state.window_size.x = width;
    frontend_state.window_size.y = height;

    glfwSetWindowSize(window, width, height);
    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_SPACE)
        debug_state.in_debug = !debug_state.in_debug;

    if (action == GLFW_PRESS && key == GLFW_KEY_V)
    {
        if (frontend_state.current_render_target == &frontend_state.psx_render_target)
            frontend_state.current_render_target = &frontend_state.vram_render_target;
        else
            frontend_state.current_render_target = &frontend_state.psx_render_target;
    }
}

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

    // Set callback functions for window resizing and handling input
    glfwSetKeyCallback(frontend_state.window, key_callback);
    glfwSetFramebufferSizeCallback(frontend_state.window, framebuffer_size_callback);

	// Check if GLAD loaded successfully
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		log_error("Failed to initialize GLAD\n");
		return -1;
	}

	return 0;
}

static void compile_shaders()
{
    const char* vShaderCode =
        "#version 410 core\n"
        "layout(location = 0) in vec3 aPos;"
        "layout(location = 1) in vec3 aColor;"
        "out vec3 color;"
        "void main()"
        "{"
        "   gl_Position = vec4(aPos, 1.0);"
        "   color = aColor;"
        "}";

    const char* fShaderCode =
        "#version 410 core\n"
        "in vec3 color;"
        "out vec4 FragColor;"
        "void main()"
        "{"
        "   FragColor = vec4(color, 0.0);"
        "}";

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        log_error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n %s\n", infoLog);
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        log_error("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    frontend_state.solid_shader = glCreateProgram();
    glAttachShader(frontend_state.solid_shader, vertex);
    glAttachShader(frontend_state.solid_shader, fragment);
    glLinkProgram(frontend_state.solid_shader);

    glGetProgramiv(frontend_state.solid_shader, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(frontend_state.solid_shader, 512, NULL, infoLog);
        log_error("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

static void create_framebuffer(RenderTarget* render_target)
{
    glGenFramebuffers(1, &render_target->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, render_target->framebuffer);

    // Depth buffer for the FBO
    glGenRenderbuffers(1, &render_target->depth_stencil_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, render_target->depth_stencil_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, render_target->size.x, render_target->size.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_target->depth_stencil_buffer);

    // Render texture for the framebuffer
    glGenTextures(1, &render_target->render_texture);
    glBindTexture(GL_TEXTURE_2D, render_target->render_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, render_target->size.x, render_target->size.y, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Set texture as color attachment 0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, render_target->render_texture, 0);

    render_target->draw_buffer = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &render_target->draw_buffer);

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void resize_psx_framebuffer(Vec2 new_size)
{
    resize_framebuffer(&PSX_RT, new_size);
}

static void resize_framebuffer(RenderTarget* render_target, Vec2 new_size)
{
    render_target->size = new_size;
    
    glDeleteFramebuffers(1, &render_target->framebuffer);
    glDeleteRenderbuffers(1, &render_target->depth_stencil_buffer);
    glDeleteTextures(1, &render_target->render_texture);

    render_target->framebuffer = 0;
    render_target->depth_stencil_buffer = 0;
    render_target->render_texture = 0;

    create_framebuffer(render_target);
}

GLuint vram_texture = 0;
uint32_t test[VRAM_WIDTH * VRAM_HEIGHT] = {0};

int start_interface()
{
	if (setup_glfw() != 0)
		return -1;

    compile_shaders();
    create_framebuffer(&PSX_RT);
    create_framebuffer(&VRAM_RT);

	return 0;
}

int update_interface()
{
    glBindTexture(GL_TEXTURE_2D, VRAM_RT.render_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VRAM_WIDTH, VRAM_HEIGHT, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, gpu_state.vram);

    // Blit from PSX framebuffer to window framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frontend_state.current_render_target->framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    Vec2 src_size = frontend_state.current_render_target->size;
    Vec2 tgt_size = frontend_state.window_size;

    glScissor(0, 0, src_size.x, src_size.y);
    glBlitFramebuffer(0, 0, src_size.x, src_size.y, 0, 0, tgt_size.x, tgt_size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    int glError = glGetError();
    if (glError != 0)
        log_error("OpenGL error %d\n", glError);

	// Swap new frame and poll GLFW for inputs
	glfwSwapBuffers(frontend_state.window);
	glfwPollEvents();

	if (glfwWindowShouldClose(frontend_state.window))
		return 1;

    glDeleteTextures(1, &vram_texture);

	return 0;
}

void stop_interface()
{
	glfwDestroyWindow(frontend_state.window);
	glfwTerminate();
}
