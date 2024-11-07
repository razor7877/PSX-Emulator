#pragma once

#include <stdint.h>
#include <stdbool.h>

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

typedef enum
{
	VARIABLE_SIZE = 0,
	SINGLE_PIXEL = 1,
	SPRITE_8x8 = 2,
	SPRITE_16x16 = 3,
} RectangleSize;

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
	
	bool running_gp0_command;

	GP0Command current_gp0_command;

	RectangleSize rect_size;

	uint32_t rect_color;
} GPU;

extern GPU gpu_state;

uint32_t read_gpu(uint32_t address);
void write_gpu(uint32_t address, uint32_t value);
