#include "gpu.h"
#include "logging.h"
#include "frontend.h"

GPU gpu_state = {
	.gpu_read = 0,
	.gpu_stat = 0xFFFFFFFF,
	.running_gp0_command = false,
	.current_gp0_command = 0,
	.rect_size = 0,
	.rect_color = 0,
	.display_mode = 0,
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

static void gp0_env(uint32_t value)
{
	// If the first 3 bits are on, then we have a command in the range 0xE1-0xE6
	uint32_t env_command_mask = 0b11111 << 24;
	uint32_t env_command = (value & env_command_mask) >> 24;

	log_warning("Received GPU environment command number %x -- value is %x\n", env_command, value);

	switch (env_command)
	{
		case 0xE1:
			log_warning("GP0(0xE1) - Draw mode settings\n");
			gpu_state.gpu_status.texture_page_x_base = value & 0b1111;
			gpu_state.gpu_status.texture_page_y_base_1 = (value & (1 << 4)) >> 4;
			gpu_state.gpu_status.semi_transparency = (value & (0b11 << 5)) >> 5;
			gpu_state.gpu_status.texture_page_colors = (value & (0b11 << 7)) >> 7;
			gpu_state.gpu_status.dither_24_to_15 = (value & (1 << 9)) >> 9;
			gpu_state.gpu_status.draw_to_display = (value & (1 << 10)) >> 10;
			gpu_state.gpu_status.texture_page_y_base_2 = (value & (1 << 11)) >> 11;
			// TODO : Set textured rectangle X/Y flip from bit 12-13
			update_gpustat();
			break;

		case 0xE2:
			break;

		case 0xE3:
			break;

		case 0xE4:
			break;

		case 0xE5:
			break;

		case 0xE6:
			break;
	}
}

static void gp0_misc(uint32_t value)
{
	if (!gpu_state.running_gp0_command)
	{
		log_warning("Received GPU render command bits with no command running -- value is %x\n", value);
		return;
	}

	if (gpu_state.current_gp0_command == GP0_RECTANGLE)
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
	}
	else
	{
		log_warning("Got misc command with unhandled GP0 command!\n");
	}
}

static void handle_gp0_command(uint32_t value)
{
	uint32_t command_mask = (0b111 << 29);
	uint32_t command = (value & command_mask) >> 29;

	switch (command)
	{
		case GP0_MISC:
			gp0_misc(value);
			break;

		case GP0_POLYGON:
			log_warning("Received GPU polygon primitive command -- value is %x\n", value);
			break;

		case GP0_LINE:
			log_warning("Received GPU line primitive command -- value is %x\n", value);
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
			log_warning("Received GPU VRAM-to-VRAM blit command -- value is %x\n", value);
			break;

		case GP0_VRAM_TO_CPU_BLIT:
			log_warning("Received GPU CPU-to-VRAM blit command -- value is %x\n", value);
			break;

		case GP0_CPU_TO_VRAM_BLIT:
			log_warning("Received GPU VRAM-to-CPU blit command -- value is %x\n", value);
			break;

		case GP0_ENVIRONMENT:
			gp0_env(value);
			break;

		default:
			log_error("Unhandled GP0 command! -- value is %x\n", value);
			break;
	}
}

static inline Vec2 get_screen_resolution(DisplayMode display_mode)
{
	int fbo_h_res = 0;
	int fbo_v_res = 240;

	if (display_mode.h_res_2 == H_RES_368)
		fbo_h_res = 368;
	else if (display_mode.h_res_1 == H_RES_256)
		fbo_h_res = 256;
	else if (display_mode.h_res_1 == H_RES_320)
		fbo_h_res = 320;
	else if (display_mode.h_res_1 == H_RES_512)
		fbo_h_res = 512;
	else if (display_mode.h_res_1 == H_RES_640)
		fbo_h_res = 640;

	// Vertical resolution is 240px unless v_res bit is set & vertical interlace is on
	if (display_mode.v_res & display_mode.use_vertical_interlace)
		fbo_v_res = 480;

	Vec2 new_size = {
		.x = fbo_h_res,
		.y = fbo_v_res,
	};

	return new_size;
}

/// <summary>
/// Updates the integer GPUSTAT register using the state in GPUStatus
/// </summary>
static void update_gpustat()
{
	GPUStatus status = gpu_state.gpu_status;

	uint32_t new_stat = 0;

	log_debug("Updating GPUSTAT...\n");

	new_stat |= status.texture_page_x_base & 0xF;
	new_stat |= status.texture_page_y_base_1 << 1;
	new_stat |= status.semi_transparency << 5;
	new_stat |= status.texture_page_colors << 7;
	new_stat |= status.dither_24_to_15 << 9;
	new_stat |= status.draw_to_display << 10;
	new_stat |= status.set_mask_on_draw << 11;
	new_stat |= status.draw_pixels << 12;
	new_stat |= status.interlace_field << 13;
	new_stat |= status.flip_screen_horizontal << 14;
	new_stat |= status.texture_page_y_base_2 << 15;
	new_stat |= status.h_res_2 << 16;
	new_stat |= status.h_res_1 << 17;
	new_stat |= status.v_res << 19;
	new_stat |= status.video_mode << 20;
	new_stat |= status.color_depth << 21;
	new_stat |= status.use_vertical_interlace << 22;
	new_stat |= status.display_enable << 23;
	new_stat |= status.irq_1_on << 24;
	new_stat |= status.dma_request << 25;
	//new_stat |= status.can_receive_cmd << 26;
	new_stat |= (1 << 26);
	//new_stat |= status.can_receive_dma_block << 27;
	new_stat |= (1 << 27);
	//new_stat |= status.can_receive_dma_block << 28;
	new_stat |= (1 << 28);
	new_stat |= status.dma_direction << 29;
	new_stat |= status.drawing_odd_lines << 31;

	log_debug("Old value is %x and new value is %x\n", gpu_state.gpu_stat, new_stat);
	gpu_state.gpu_stat = new_stat;
}

/// <summary>
/// Updates the GPUSTAT parameters after a GP1 display mode command
/// </summary>
/// <param name="display_mode">The display mode parameters</param>
static void update_gpustat_display_mode(DisplayMode display_mode)
{
	GPUStatus* status = &gpu_state.gpu_status;

	status->h_res_1 = display_mode.h_res_1;
	status->h_res_2 = display_mode.h_res_2;
	status->v_res = display_mode.v_res;
	status->video_mode = display_mode.video_mode;
	status->color_depth = display_mode.color_depth;
	status->use_vertical_interlace = display_mode.use_vertical_interlace;
	status->flip_screen_horizontal = display_mode.flip_screen_horizontal;

	update_gpustat();
}

/// <summary>
/// Executes a GP1 display mode command
/// </summary>
/// <param name="value">The GP1 display mode command</param>
static void gp1_display_mode(uint32_t value)
{
	DisplayMode* display_mode = &gpu_state.display_mode;

	display_mode->h_res_1 = value & 0b11;
	display_mode->v_res = (value & (1 << 2)) >> 2;
	display_mode->video_mode = (value & (1 << 3)) >> 3;
	display_mode->color_depth = (value & (1 << 4)) >> 4;
	display_mode->use_vertical_interlace = (value & (1 << 5)) >> 5;
	display_mode->h_res_2 = (value & (1 << 6)) >> 6;
	display_mode->flip_screen_horizontal = (value & (1 << 7)) >> 7; 

	// Update GPUSTAT register
	update_gpustat_display_mode(gpu_state.display_mode);

	Vec2 screen_size = get_screen_resolution(gpu_state.display_mode);
	resize_psx_framebuffer(screen_size);
}

static void handle_gp1_command(uint32_t value)
{
	uint32_t command_mask = 0xFF000000;
	uint32_t command = (value & command_mask) >> 24;

	switch (command)
	{
		case GP1_RESET:
			log_debug("GP1 Command: Reset GPU\n");
			handle_gp1_command(0x01 << 24);
			handle_gp1_command(0x02 << 24);
			handle_gp1_command(0x03 << 24);
			handle_gp1_command(0x04 << 24);
			handle_gp1_command(0x05 << 24);
			handle_gp1_command(0x06 << 24);
			handle_gp1_command(0x07 << 24);
			handle_gp1_command(0x08 << 24);

			handle_gp0_command(0xE1 << 24);
			handle_gp0_command(0xE2 << 24);
			handle_gp0_command(0xE3 << 24);
			handle_gp0_command(0xE4 << 24);
			handle_gp0_command(0xE5 << 24);
			handle_gp0_command(0xE6 << 24);
			break;

		case GP1_RESET_CMD_BUFFER:
			log_debug("GP1 Command: Reset command buffer\n");
			gpu_state.running_gp0_command = false;
			break;

		case GP1_ACK_IRQ:
			log_debug("GP1 Command: ACK GPU IRQ -- old flag is %x\n", gpu_state.gpu_status.irq_1_on);
			gpu_state.gpu_status.irq_1_on = false;
			update_gpustat();
			log_debug("New flag is %x\n", gpu_state.gpu_status.irq_1_on);
			break;

		case GP1_DISPLAY_ENABLE:
			log_debug("GP1 Command: Display enable -- old enable is %x\n", gpu_state.gpu_status.display_enable);
			gpu_state.gpu_status.display_enable = value & 1;
			update_gpustat();
			log_debug("New enable is %x\n", gpu_state.gpu_status.display_enable);
			break;

		case GP1_DMA_DIRECTION:
			log_debug("GP1 Command: DMA Direction/Data request -- old direction is %x\n", gpu_state.gpu_status.dma_direction);
			gpu_state.gpu_status.dma_direction = (value & 0b11) << 29;
			update_gpustat();
			log_debug("New direction is %x\n", gpu_state.gpu_status.dma_direction);
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
			log_debug("GP1 Command: Display mode\n");
			gp1_display_mode(value);
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
			log_error("Unhandled GP1 command!\n");
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
