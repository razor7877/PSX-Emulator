#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "frontend/gl.h"
#include "memory.h"

#define GPU_FREQ 53222400 // GPU Frequency in Hz
#define DOT_CLK_256 5322240
#define DOT_CLK_320 6652800
#define DOT_CLK_368 7603200
#define DOT_CLK_512 10644480
#define DOT_CLK_640 13305600

#define NTSC_SCANLINE_CYCLES 3413 // In GPU cycles
#define GPU_TO_CPU_CYCLES (CPU_FREQ / GPU_FREQ)

/// <summary>
/// Functions and state for emulating the GPU state, but not the graphics API implementation
/// </summary>

/// <summary>
/// The possible GP0 commands
/// </summary>
typedef enum
{
	GP0_MISC = 0,
	GP0_POLYGON = 1,
	GP0_LINE = 2,
	GP0_RECTANGLE = 3,
	GP0_VRAM_TO_VRAM_BLIT = 4,
	GP0_CPU_TO_VRAM_BLIT = 5,
	GP0_VRAM_TO_CPU_BLIT = 6,
	GP0_ENVIRONMENT = 7,
} GP0Command;

/// <summary>
/// The possible GP1 commands
/// </summary>
typedef enum
{
	GP1_RESET = 0,
	GP1_RESET_CMD_BUFFER = 1,
	GP1_ACK_IRQ = 2,
	GP1_DISPLAY_ENABLE = 3,
	GP1_DMA_DIRECTION = 4,
	GP1_DISPLAY_START_VRAM = 5,
	GP1_DISPLAY_RANGE_H = 6,
	GP1_DISPLAY_RANGE_V = 7,
	GP1_DISPLAY_MODE = 8,
	GP1_VRAM_SIZE_V2 = 9,
	GP1_READ_INTERNAL_REG = 0x10,
	GP1_VRAM_SIZE_V1 = 0x20,
} GP1Command;

/// <summary>
/// Rectangle rendering
/// </summary>
typedef enum
{
	VARIABLE_SIZE = 0,
	SINGLE_PIXEL = 1,
	SPRITE_8x8 = 2,
	SPRITE_16x16 = 3,
} RectangleSize;

/// <summary>
/// Display mode horizontal resolution
/// </summary>
typedef enum
{
	H_RES_256 = 0,
	H_RES_320 = 1,
	H_RES_512 = 2,
	H_RES_640 = 3,
} HorizontalRes1;

/// <summary>
/// Display mode horizontal resolution 2
/// </summary>
typedef enum
{
	USE_H_RES_1 = 0,
	H_RES_368 = 1,
} HorizontalRes2;

/// <summary>
/// Display mode vertical resolution
/// </summary>
typedef enum
{
	V_RES_240 = 0,
	V_RES_480 = 1,
} VerticalRes;

/// <summary>
/// Display mode video mode
/// </summary>
typedef enum
{
	NTSC = 0,
	PAL = 1,
} VideoMode;

/// <summary>
/// Displ
/// </summary>
typedef enum
{
	COLOR_15_BITS = 0,
	COLOR_24_BITS = 1,
} DisplayColorDepth;

typedef enum
{
	PAGE_4_BIT,
	PAGE_8_BIT,
	PAGE_15_BIT,
	PAGE_RESERVED
} TexturePageColors;

typedef enum
{
	DMA_DIR_OFF = 0,
	DMA_DIR_FIFO = 1,
	DMA_DIR_CPU_TO_GP0 = 2,
	DMA_DIR_GPUREAD_TO_CPU = 3,
} DMADirection;

/// <summary>
/// Display mode state - GP1(0x08)
/// </summary>
typedef struct
{
	HorizontalRes1 h_res_1;
	HorizontalRes2 h_res_2;
	VerticalRes v_res;
	VideoMode video_mode;
	DisplayColorDepth color_depth;
	bool use_vertical_interlace;
	bool flip_screen_horizontal;
} DisplayMode;

/// <summary>
/// All the parameters of the GPUSTAT registers
/// </summary>
typedef struct
{
	uint8_t texture_page_x_base;
	bool texture_page_y_base_1;
	uint8_t semi_transparency;
	TexturePageColors texture_page_colors;
	bool dither_24_to_15;
	bool draw_to_display;
	bool set_mask_on_draw;
	bool draw_pixels;
	bool interlace_field;
	bool flip_screen_horizontal;
	bool texture_page_y_base_2;
	HorizontalRes1 h_res_1;
	HorizontalRes2 h_res_2;
	VerticalRes v_res;
	VideoMode video_mode;
	DisplayColorDepth color_depth;
	bool use_vertical_interlace;
	bool display_enable;
	bool irq_1_on;
	bool dma_request;
	bool can_receive_cmd;
	bool can_send_vram_to_cpu;
	bool can_receive_dma_block;
	DMADirection dma_direction;
	bool drawing_odd_lines;
} GPUStatus;

typedef struct
{
	/// <summary>
	/// GPU_READ register
	/// </summary>
	uint32_t gpu_read;

	/// <summary>
	/// GPU_STAT register
	/// </summary>
	uint32_t gpu_stat;

	/// <summary>
	/// All the parameters of the GPUSTAT register
	/// </summary>
	GPUStatus gpu_status;

	/// <summary>
	/// Stores the state associated with the display mode GP1 command
	/// </summary>
	DisplayMode display_mode;

	/// <summary>
	/// Whether the GPU is currently running a GP0 command
	/// </summary>
	bool running_gp0_command;

	/// <summary>
	/// An enum identifying the currently running GP0 command
	/// </summary>
	GP0Command current_gp0_command;

	/// <summary>
	/// When doing CPU to VRAM blit, how many words we still need to consume
	/// </summary>
	int blit_words_remaining;

	/// <summary>
	/// The size of the rectangle for the current CPU to VRAM blit transfer
	/// </summary>
	iVec2 blit_size;

	/// <summary>
	/// The position of the rectangle (top left corner) for the current CPU to VRAM blit transfer
	/// </summary>
	iVec2 blit_position;

	/// <summary>
	/// The current x position in the rectangle for the current CPU to VRAM blit transfer
	/// </summary>
	int blit_x_count;

	/// <summary>
	/// The current y position in the rectangle for the current CPU to VRAM blit transfer
	/// </summary>
	int blit_y_count;

	/// <summary>
	/// The index of the last index in the command buffer
	/// </summary>
	int command_buffer_index;

	/// <summary>
	/// Stores the word for the command being executed
	/// </summary>
	uint32_t command_buffer[13];

	/// <summary>
	/// GP0(0xE2)
	/// </summary>
	Vec2 texture_window_mask;

	/// <summary>
	/// GP0(0xE2)
	/// </summary>
	Vec2 texture_window_offset;

	/// <summary>
	/// GP0(0xE3)
	/// </summary>
	Vec2 drawing_area_top_left;

	/// <summary>
	/// GP0(0xE4)
	/// </summary>
	Vec2 drawing_area_bottom_right;

	/// <summary>
	/// GP0(0xE5)
	/// </summary>
	Vec2 drawing_area_offset;

	/// <summary>
	/// GP0(0xE6)
	/// </summary>
	bool set_mask_while_drawing;

	/// <summary>
	/// GP0(0xE6)
	/// </summary>
	bool check_mask_before_draw;

	/// <summary>
	/// Raw data - GP1(0x05)
	/// </summary>
	uint32_t display_area_start;

	/// <summary>
	/// Raw data - GP1(0x06)
	/// </summary>
	uint32_t display_range_horizontal;

	/// <summary>
	/// Raw data - GP1(0x07)
	/// </summary>
	uint32_t display_range_vertical;

	/// <summary>
	/// The VRAM contains 1024 KiB of VRAM as 16 bit words/pixels
	/// </summary>
	uint16_t vram[1024 * KIB_SIZE / HALF_WORD_SIZE];
} GPU;

extern GPU gpu_state;

void reset_gpu_state();

/// <summary>
/// Reads from a memory address located in the GPU
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The value at the address</returns>
uint32_t read_gpu(uint32_t address);

/// <summary>
/// Writes to a memory address located in the GPU
/// </summary>
/// <param name="address">The address to be written to</param>
/// <param name="value">The value to write to the address</param>
void write_gpu(uint32_t address, uint32_t value);

static void gp0_env(uint32_t value);
static void finish_gp0_command();
static void gp0_misc(uint32_t value);
static void start_gp0_command(uint32_t value, GP0Command command);
static void handle_gp0_command(uint32_t value);

static inline Vec2 get_screen_resolution(DisplayMode display_mode);

void update_gpustat();
static void update_gpustat_display_mode(DisplayMode display_mode);
static void gp1_display_mode(uint32_t value);
static void handle_gp1_command(uint32_t value);
