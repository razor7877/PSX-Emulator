#pragma once

#include <stdint.h>
#include <stdbool.h>

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
	GP0_VRAM_TO_CPU_BLIT = 5,
	GP0_CPU_TO_VRAM_BLIT = 6,
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
	USE_H_RES_1,
	H_RES_368,
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
	/// Whether the GPU is currently running a GP0 command
	/// </summary>
	bool running_gp0_command;

	/// <summary>
	/// An enum identifying the currently running GP0 command
	/// </summary>
	GP0Command current_gp0_command;

	/// <summary>
	/// When rendering rectangles, the current rectangle size
	/// </summary>
	RectangleSize rect_size;

	/// <summary>
	/// When rendering rectangles, the current rectangle color
	/// </summary>
	uint32_t rect_color;

	/// <summary>
	/// Stores the state associated with the display mode GP1 command
	/// </summary>
	DisplayMode display_mode;
} GPU;

extern GPU gpu_state;

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
