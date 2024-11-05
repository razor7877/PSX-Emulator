#include "gpu.h"
#include "logging.h"
#include "frontend.h"

gpu gpu_state = {
	.gpu_read = 0,
	.gpu_stat = 0xFFFFFFFF,
	.running_gp0_command = false,
	.current_gp0_command = 0,
	.rect_size = 0,
	.rect_color = 0,
};

uint32_t read_gpu(uint32_t address)
{
	if (address == 0x1F801810)
		return gpu_state.gpu_read;

	if (address == 0x1F801814)
		return gpu_state.gpu_stat;

	log_warning("Unhandled GPU register read at address %x\n", address);

	return 0xFFFFFFFF;
}

static void handle_gp0_command(uint32_t value)
{
	uint32_t command_mask = (0b111 << 29);
	uint32_t command = (value & command_mask) >> 29;

	switch (command)
	{
		case GP0_MISC:
			if (gpu_state.running_gp0_command && gpu_state.current_gp0_command == GP0_RECTANGLE)
			{
				uint8_t red = gpu_state.rect_color & 0x0000FF;
				uint8_t green = (gpu_state.rect_color & 0x00FF00) >> 8;
				uint8_t blue = (gpu_state.rect_color & 0xFF0000) >> 16;

				uint16_t x_coord = value & 0xFFFF;
				uint16_t y_coord = (value & 0xFFFF0000) >> 16;

				if (gpu_state.rect_size == SINGLE_PIXEL)
					draw_pixel(x_coord, y_coord, red, green, blue);
				else
					log_warning("Unhandled rectangle draw with size %x\n", gpu_state.rect_size);
				//log_warning("Received rectangle vertex data - x %x y %x - RGB %x %x %x\n", x_coord, y_coord, red, green, blue);
			}
			else
				log_warning("Received GPU misc command\n");
			break;

		case GP0_POLYGON:
			log_warning("Received GPU polygon primitive command\n");
			break;

		case GP0_LINE:
			log_warning("Received GPU line primitive command\n");
			break;

		case GP0_RECTANGLE:
			gpu_state.running_gp0_command = true;
			gpu_state.current_gp0_command = GP0_RECTANGLE;

			uint32_t size_mask = 0b11 << 27;
			gpu_state.rect_size = (value & size_mask) >> 27;
			gpu_state.rect_color = value & 0x00FFFFFF;

			//log_warning("Received GPU rectangle primitive command - Rect size is %x\n", gpu_state.rect_size);
			break;

		case GP0_VRAM_TO_VRAM_BLIT:
			log_warning("Received GPU VRAM-to-VRAM blit command\n");
			break;

		case GP0_VRAM_TO_CPU_BLIT:
			log_warning("Received GPU CPU-to-VRAM blit command\n");
			break;

		case GP0_CPU_TO_VRAM_BLIT:
			log_warning("Received GPU VRAM-to-CPU blit command\n");
			break;

		case GP0_ENVIRONMENT:
			log_warning("Received GPU environment command\n");
			break;
	}
}

static void handle_gp1_command(uint32_t value)
{
	uint32_t command_mask = 0xFF000000;
	uint32_t command = (value & command_mask) >> 24;

	switch (command)
	{
		case GP1_RESET:
			log_warning("GP1 Command: Reset GPU\n");
			break;

		case GP1_RESET_CMD_BUFFER:
			gpu_state.running_gp0_command = false;
			//log_warning("GP1 Command: Reset command buffer\n");
			break;

		case GP1_ACK_IRQ:
			log_warning("GP1 Command: ACK GPU IRQ\n");
			break;

		case GP1_DISPLAY_ENABLE:
			log_warning("GP1 Command: Display enable\n");
			break;

		case GP1_DMA_DIRECTION:
			log_warning("GP1 Command: DMA Direction/Data request\n");
			break;

		case GP1_DISPLAY_START_VRAM:
			log_warning("GP1 Command: Start of display area in VRAM\n");
			break;

		case GP1_DISPLAY_RANGE_H:
			log_warning("GP1 Command: Horizontal display range\n");
			break;

		case GP1_DISPLAY_RANGE_V:
			log_warning("GP1 Command: Vertical display range\n");
			break;

		case GP1_DISPLAY_MODE:
			log_warning("GP1 Command: Display mode\n");
			break;

		case GP1_VRAM_SIZE_V2:
			log_warning("GP1 Command: Set VRAM size (v2)\n");
			break;

		case GP1_READ_INTERNAL_REG:
			log_warning("GP1 Command: Read GPU internal register\n");
			break;

		case GP1_VRAM_SIZE_V1:
			log_warning("GP1 Command: Set VRAM size (v1)\n");
			break;

		default:
			log_warning("Unhandled GP1 command!\n");
			break;
	}
}

void write_gpu(uint32_t address, uint32_t value)
{
	if (address == 0x1F801810)
		handle_gp0_command(value);
	else if (address == 0x1F801814)
		handle_gp1_command(value);
	else
		log_warning("Unhandled GPU register write at address %x with value %x\n", address, value);

	//log_warning("Unhandled GPU register write at address %x with value %x\n", address, value);
}
