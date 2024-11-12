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

// Flat/Gouraud polygon shader
const char* color_v_shader =
    "#version 410 core\n"
    "layout(location = 0) in vec3 aPos;"
    "layout(location = 1) in vec3 aColor;"
    "out vec3 color;"
    "void main()"
    "{"
    "   gl_Position = vec4(aPos, 1.0);"
    "   color = aColor;"
    "}";

const char* color_f_shader =
    "#version 410 core\n"
    "in vec3 color;"
    "out vec4 FragColor;"
    "void main()"
    "{"
    "   FragColor = vec4(color, 1.0);"
    "}";

// Textured polygon shader
const char* texture_v_shader =
    "#version 410 core\n"
    "layout(location = 0) in vec3 aPos;"
    "layout(location = 1) in vec2 aTexCoord;"
    "out vec2 texCoord;"
    "void main()"
    "{"
    "   gl_Position = vec4(aPos, 1.0);"
    "   texCoord = aTexCoord;"
    "}";

const char* texture_f_shader =
    "#version 410 core\n"
    "in vec2 texCoord;"
    "out vec4 FragColor;"
    "uniform sampler2D textureSampler;"
    "void main()"
    "{"
    "   FragColor = vec4(texCoord, 1.0, 1.0);"
    "}";

// Blit to quad shader
const char* blit_v_shader =
    "#version 410 core\n"
    "layout(location = 0) in vec3 aPos;"
    "layout(location = 1) in vec2 aTexCoord;"
    "out vec2 texCoord;"
    "void main()"
    "{"
    "   gl_Position = vec4(aPos, 1.0);"
    "   texCoord = aTexCoord;"
    "}";

const char* blit_f_shader =
    "#version 410 core\n"
    "in vec2 texCoord;"
    "out vec4 FragColor;"
    "uniform sampler2D textureSampler;"
    "void main()"
    "{"
    "   FragColor = texture(textureSampler, texCoord);"
    "}";

Frontend frontend_state = {
	.window = NULL,
    .color_shader = 0,
    .blit_shader = 0,
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
    float vertices[9] = {
        (triangle.v1.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (triangle.v1.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (triangle.v2.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (triangle.v2.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (triangle.v3.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (triangle.v3.position.y / PSX_RT.size.y) * 2.0f,
        0.0f
    };

    float colors[9] = {
        triangle.v1.color.r / 255.0f,
        triangle.v1.color.g / 255.0f,
        triangle.v1.color.b / 255.0f,
        triangle.v2.color.r / 255.0f,
        triangle.v2.color.g / 255.0f,
        triangle.v2.color.b / 255.0f,
        triangle.v3.color.r / 255.0f,
        triangle.v3.color.g / 255.0f,
        triangle.v3.color.b / 255.0f,
    };

    glBindFramebuffer(GL_FRAMEBUFFER, PSX_RT.framebuffer);
    glUseProgram(frontend_state.color_shader);

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

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &color_bo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_textured_triangle(Triangle triangle)
{
    log_warning("Unhandled OpenGL function -- draw_textured_triangle !\n");
}

void draw_quad(Quad quad)
{
    // Prepare vertices for OpenGL
    float vertices[18] = {
        (quad.v1.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v1.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (quad.v2.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v2.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (quad.v3.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v3.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (quad.v2.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v2.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (quad.v3.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v3.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (quad.v4.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v4.position.y / PSX_RT.size.y) * 2.0f,
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
    glUseProgram(frontend_state.color_shader);

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

void draw_textured_quad(Quad quad)
{
    // Prepare vertices for OpenGL
    float vertices[18] = {
        (quad.v1.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v1.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (quad.v2.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v2.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (quad.v3.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v3.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (quad.v2.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v2.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (quad.v3.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v3.position.y / PSX_RT.size.y) * 2.0f,
        0.0f,
        (quad.v4.position.x / PSX_RT.size.x) * 2.0f - 1.0f,
        1.0f - (quad.v4.position.y / PSX_RT.size.y) * 2.0f,
        0.0f
    };

    float uv[12] = {
        quad.v1.uv.x / 255.0f,
        (255.0f - quad.v1.uv.y) / 255.0f,
        quad.v2.uv.x / 255.0f,
        (255.0f - quad.v2.uv.y) / 255.0f,
        quad.v3.uv.x / 255.0f,
        (255.0f - quad.v3.uv.y) / 255.0f,
        quad.v2.uv.x / 255.0f,
        (255.0f - quad.v2.uv.y) / 255.0f,
        quad.v3.uv.x / 255.0f,
        (255.0f - quad.v3.uv.y) / 255.0f,
        quad.v4.uv.x / 255.0f,
        (255.0f - quad.v4.uv.y) / 255.0f,
    };

    uint32_t texture_page_start = quad.uv_data.texture_page_y_base * 1024 + quad.uv_data.texture_page_x_base;
    uint32_t clut_start;

    glBindFramebuffer(GL_FRAMEBUFFER, PSX_RT.framebuffer);
    glUseProgram(frontend_state.color_shader);

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint texture_bo = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &texture_bo);

    glBindVertexArray(vao);

    // Send vertices
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Send colors
    glBindBuffer(GL_ARRAY_BUFFER, texture_bo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uv), uv, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &texture_bo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    frontend_state.window_size.x = width;
    frontend_state.window_size.y = height;

    glfwSetWindowSize(window, width, height);
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

static GLuint compile_shader(const char* v_shader, const char* f_shader)
{
    GLuint program = 0;

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &v_shader, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        log_error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n %s\n", infoLog);
        return 0;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &f_shader, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        log_error("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
        return 0;
    }

    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        log_error("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        return 0;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, render_target->size.x, render_target->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

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

GLuint vram_tex = 0;
GLuint blit_quad_vao = 0;
GLuint blit_quad_vbo = 0;
GLuint blit_quad_texture_bo = 0;

const float quad_verts[] = {
    -1.0f, 1.0f, 0.0f, // Top left
    1.0f, 1.0f, 0.0f, // Top right
    -1.0f, -1.0f, 0.0f, // Bottom left

    1.0f, 1.0f, 0.0f, // Top right
    1.0f, -1.0f, 0.0f, // Bottom right
    -1.0f, -1.0f, 0.0f, // Bottom left
};

const float quad_tex_coords[] = {
    0.0f, 0.0f, // Top left
    1.0f, 0.0f, // Top right
    0.0f, 1.0f, // Bottom left

    1.0f, 0.0f, // Top right
    1.0f, 1.0f, // Bottom right
    0.0f, 1.0f, // Bottom left
};

static void setup_blit_quad()
{
    // Generate VAO and VBO and bind them
    glGenVertexArrays(1, &blit_quad_vao);
    glGenBuffers(1, &blit_quad_vbo);

    glBindVertexArray(blit_quad_vao);

    // Send vertices
    glBindBuffer(GL_ARRAY_BUFFER, blit_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, GL_STATIC_DRAW);

    // Enable layout 0 input in shader
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Send tex coords
    glGenBuffers(1, &blit_quad_texture_bo);

    glBindBuffer(GL_ARRAY_BUFFER, blit_quad_texture_bo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_tex_coords), quad_tex_coords, GL_STATIC_DRAW);

    // Enable layout 1 input in shader
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
}

int start_interface()
{
	if (setup_glfw() != 0)
		return -1;

    frontend_state.color_shader = compile_shader(color_v_shader, color_f_shader);
    frontend_state.texture_shader = compile_shader(texture_v_shader, texture_f_shader);
    frontend_state.blit_shader = compile_shader(blit_v_shader, blit_f_shader);

    create_framebuffer(&PSX_RT);
    create_framebuffer(&VRAM_RT);

    setup_blit_quad();

    // Render texture for the VRAM
    glGenTextures(1, &vram_tex);
    glBindTexture(GL_TEXTURE_2D, vram_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VRAM_WIDTH, VRAM_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	return 0;
}

int update_interface()
{
    // Send VRAM data to the texture
    glBindTexture(GL_TEXTURE_2D, vram_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VRAM_WIDTH, VRAM_HEIGHT, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, gpu_state.vram);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Render to the framebuffer with a quad
    glBindFramebuffer(GL_FRAMEBUFFER, VRAM_RT.framebuffer);
    glViewport(0, 0, VRAM_WIDTH, VRAM_HEIGHT);

    glBindVertexArray(blit_quad_vao);

    glUseProgram(frontend_state.blit_shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, vram_tex);
    glUniform1i(glGetUniformLocation(frontend_state.blit_shader, "textureSampler"), 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Blit from PSX framebuffer to window framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frontend_state.current_render_target->framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    Vec2 src_size = frontend_state.current_render_target->size;
    Vec2 tgt_size = frontend_state.window_size;

    glViewport(0, 0, tgt_size.x, tgt_size.y);
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

	return 0;
}

void stop_interface()
{
	glfwDestroyWindow(frontend_state.window);
	glfwTerminate();
}
