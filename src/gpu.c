#include <string.h>

#include "frontend/gl.h"
#include "gpu.h"
#include "logging.h"
#include "cpu.h"
#include "memory.h"

GPU gpu_state = {
	.gpu_read = 0,
	.gpu_stat = 0,
	.gpu_status = {0},
	.display_mode = 0,
	.running_gp0_command = false,
	.current_gp0_command = 0,
	.blit_words_remaining = 0,
	.command_buffer_index = 0,
	.command_buffer = {0},
	.texture_window_mask = {0},
	.texture_window_offset = {0},
	.drawing_area_top_left = {0},
	.drawing_area_bottom_right = {0},
	.drawing_area_offset = {0},
	.set_mask_while_drawing = false,
	.check_mask_before_draw = false,
	.display_area_start = 0,
	.display_range_horizontal = 0,
	.display_range_vertical = 0,
	.vram = {0},
};

void reset_gpu_state()
{
	memset(&gpu_state, 0, sizeof(gpu_state));
}

uint32_t read_gpu(uint32_t address)
{
	if (address == 0x1F801810)
		return gpu_state.gpu_read;

	if (address == 0x1F801814)
	{
		gpu_state.gpu_status.drawing_odd_lines = !gpu_state.gpu_status.drawing_odd_lines;
		update_gpustat();
		return gpu_state.gpu_stat;
	}

	log_warning("Unhandled GPU register read at address %x\n", address);

	return 0xFFFFFFFF;
}

void write_gpu(uint32_t address, uint32_t value)
{
	if (address == 0x1F801810)
		handle_gp0_command(value);
	else if (address == 0x1F801814)
		handle_gp1_command(value);
	else
		log_warning("Unhandled GPU register write at address %x with value %x\n", address, value);
}

static void gp0_env(uint32_t value)
{
	// If the first 3 bits are on, then we have a command in the range 0xE1-0xE6
	uint32_t env_command_mask = 0b11111 << 24;
	uint32_t env_command = (value & env_command_mask) >> 24;

	switch (env_command)
	{
		case 0x01:
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

		case 0x02:
			gpu_state.texture_window_mask.x = value & 0b11111;
			gpu_state.texture_window_mask.y = (value & (0b11111 << 5)) >> 5;

			gpu_state.texture_window_offset.x = (value & (0b11111 << 10)) >> 10;
			gpu_state.texture_window_offset.y = (value & (0b11111 << 15)) >> 15;
			break;

		case 0x03:
			// Bottom right x value from lowest 10 bits
			gpu_state.drawing_area_top_left.x = value & 0x3FF;
			// Bottom right y value from bits 10-19
			gpu_state.drawing_area_top_left.y = (value & (0x3FF << 10)) >> 10;
			break;

		case 0x04:
			// Bottom right x value from lowest 10 bits
			gpu_state.drawing_area_bottom_right.x = value & 0x3FF;
			// Bottom right y value from bits 10-19
			gpu_state.drawing_area_bottom_right.y = (value & (0x3FF << 10)) >> 10;
			break;

		case 0x05:
		{
			// x signed value from lowest 11 bits
			uint16_t x_value = value & 0x7FF;
			if (x_value & (1 << 10))
				x_value |= (0b11111 << 11);

			// y signed value from next 11 bits
			uint16_t y_value = (value & (0x7FF << 11)) >> 11;
			if (y_value & (1 << 10))
				y_value |= (0b11111 << 11);

			gpu_state.drawing_area_offset.x = (int16_t)x_value;
			gpu_state.drawing_area_offset.y = (int16_t)y_value;
			break;
		}

		case 0x06:
			gpu_state.set_mask_while_drawing = value & 0b1;
			gpu_state.check_mask_before_draw = (value & 0b10) >> 1;
			break;

		default:
			log_error("Received unhandled GP0 environment command! Value is %x\n", value);
			break;
	}
}

static void finish_gp0_command()
{
	gpu_state.current_gp0_command = 0;
	gpu_state.running_gp0_command = false;
	gpu_state.command_buffer_index = 0;

	memset(gpu_state.command_buffer, 0, sizeof(gpu_state.command_buffer));

	gpu_state.blit_words_remaining = 0;
	gpu_state.blit_x_count = 0;
	gpu_state.blit_y_count = 0;
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
		uint32_t size_mask = 0b11 << 27;
		uint32_t rect_size = (gpu_state.command_buffer[0] & size_mask) >> 27;
		uint32_t rect_color = gpu_state.command_buffer[0] & 0x00FFFFFF;

		uint8_t red = rect_color & 0x0000FF;
		uint8_t green = (rect_color & 0x00FF00) >> 8;
		uint8_t blue = (rect_color & 0xFF0000) >> 16;

		uint16_t x_coord = value & 0xFFFF;
		uint16_t y_coord = (value & 0xFFFF0000) >> 16;

		if (rect_size == SINGLE_PIXEL)
			draw_pixel(x_coord, y_coord, red, green, blue);
		else
			log_warning("Unhandled rectangle draw with size %x\n", rect_size);
	}
	else if (gpu_state.current_gp0_command == GP0_CPU_TO_VRAM_BLIT)
	{
		//log_info("Received parameter for CPU to VRAM blit command index %d\n", gpu_state.command_buffer_index);
		gpu_state.command_buffer[gpu_state.command_buffer_index] = value;
		gpu_state.command_buffer_index++;
		
		if (gpu_state.command_buffer_index >= 3)
		{
			uint16_t x_pos = gpu_state.command_buffer[1] & 0x3FF;
			uint16_t y_pos = (gpu_state.command_buffer[1] & 0x1FF0000) >> 16;

			gpu_state.blit_position.x = x_pos;
			gpu_state.blit_position.y = y_pos;

			uint16_t x_size = gpu_state.command_buffer[2] & 0xFFFF;
			uint16_t y_size = (gpu_state.command_buffer[2] & 0xFFFF0000) >> 16;

			gpu_state.blit_size.x = ((x_size - 1) & 0x3FF) + 1;
			gpu_state.blit_size.y = ((y_size - 1) & 0x1FF) + 1;

			int word_count = ((x_size * y_size) + 1) & 0xFFFFFFFE;

			// VRAM is laid out as 512 lines of 2048 bytes / 1024 half words
			int dest_location = y_pos * 1024 + x_pos;

			// Round up if we have an uneven number of pixels, since we send 2 words at a time
			word_count = (word_count + 1) & ~1;

			gpu_state.blit_words_remaining = word_count / 2;
			gpu_state.blit_x_count = 0;
			gpu_state.blit_y_count = 0;

			log_info("Starting CPU to VRAM blit -- dest is %x %x -- size is %x %x (%x)\n",
				x_pos, y_pos,
				x_size, y_size,
				word_count
			);
		}
	}
	else if (gpu_state.current_gp0_command == GP0_POLYGON)
	{
		uint32_t command_value = gpu_state.command_buffer[0];

		// Gouraud or flat shading
		bool is_gouraud_shading = (command_value & (1 << 28)) >> 28;
		// Triangle or rectangle primitive
		bool is_rectangle = (command_value & (1 << 27)) >> 27;
		// Textured/untextured
		bool is_textured = (command_value & (1 << 26)) >> 26;
		// Semi transparent or opaque
		bool is_semi_transparent = (command_value & (1 << 25)) >> 25;
		// Raw texture or modulation
		bool use_raw_texture = (command_value & (1 << 24)) >> 24;

		int vertices_count = is_rectangle ? 4 : 3;
		int vertex_word_size = 1;

		if (is_gouraud_shading)
			vertex_word_size++;
		if (is_textured)
			vertex_word_size++;

		// The polygon command will take up to 12 words + 1 for the command itself
		int total_cmd_size = vertices_count * vertex_word_size + 1;
		// Substract one since the first color is in the command value when using Gouraud shading
		total_cmd_size -= is_gouraud_shading;

		gpu_state.command_buffer[gpu_state.command_buffer_index] = value;
		gpu_state.command_buffer_index++;

		if (gpu_state.command_buffer_index >= total_cmd_size)
		{
			int color_offset = 0;
			int vertex_offset = 0;
			int uv_offset = 1;

			if (is_gouraud_shading)
			{
				color_offset = -1;
				uv_offset++;
			}

			Vec2 positions[4] = {0};
			Vec3 colors[4] = {0};
			Vec2 uv_coords[4] = {0};
			UVData uv_data = {0};

			uint16_t clut_index = 0;
			uint16_t texture_page_info = 0;

			if (is_textured)
			{
				// One word of UV data per vertex, the first vertex also contains the CLUT index
				clut_index = (gpu_state.command_buffer[1 + uv_offset] & 0xFFFF0000) >> 16;
				// One word of UV data per vertex, the first vertex also contains the texture page data
				texture_page_info = (gpu_state.command_buffer[1 + vertex_word_size + uv_offset] & 0xFFFF0000) >> 16;

				// x coord in 16 halfword steps
				uv_data.clut_position.x = clut_index & 0b111111;
				// y coord in line steps
				uv_data.clut_position.y = (clut_index & (0b111111111 << 6)) >> 6;
				uv_data.texture_page_x_base = texture_page_info & 0b1111;
				uv_data.texture_page_y_base = (texture_page_info & (1 << 4)) >> 4;
				uv_data.semi_transparency = (texture_page_info & (0b11 << 5)) >> 5;
				uv_data.texture_page_colors = (texture_page_info & (0b11 << 7)) >> 7;
			}

			// For each vertex, we construct the position and color data
			for (int i = 0; i < vertices_count; i++)
			{
				// Multiply by vertex_word_size to get the correct stride
				uint32_t xy_pos = gpu_state.command_buffer[1 + i * vertex_word_size + vertex_offset];
				positions[i].x = (int16_t)(xy_pos & 0xFFFF);
				positions[i].y = (int16_t)((xy_pos & 0xFFFF0000) >> 16);

				uint32_t color;
				if (is_gouraud_shading)
					color = gpu_state.command_buffer[1 + i * vertex_word_size + color_offset];
				else
					color = command_value;

				colors[i].r = (color & 0x0000FF); // B
				colors[i].g = (color & 0x00FF00) >> 8; // G
				colors[i].b = (color & 0xFF0000) >> 16; // R

				if (is_textured)
				{
					uint32_t uv_index = 1 + i * vertex_word_size + uv_offset;
					uint32_t uv = gpu_state.command_buffer[uv_index];

					uv_coords[i].x = uv & 0xFF;
					uv_coords[i].y = (uv & 0xFF00) >> 8;
				}
			}

			if (is_rectangle)
			{
				Quad quad = {
					.v1 = { positions[0], colors[0], uv_coords[0] },
					.v2 = { positions[1], colors[1], uv_coords[1] },
					.v3 = { positions[2], colors[2], uv_coords[2] },
					.v4 = { positions[3], colors[3], uv_coords[3] },
					.uv_data = uv_data,
				};

				if (is_textured)
					draw_textured_quad(quad);
				else
					draw_quad(quad);
			}
			else
			{
				Triangle triangle = {
					.v1 = { positions[0], colors[0], uv_coords[0] },
					.v2 = { positions[1], colors[1], uv_coords[1] },
					.v3 = { positions[2], colors[2], uv_coords[2] },
					.uv_data = uv_data,
				};

				if (is_textured)
					draw_textured_triangle(triangle);
				else
					draw_triangle(triangle);
			}

			finish_gp0_command();
		}
	}
	else
	{
		log_warning("Got misc command with unhandled GP0 command!\n");
	}
}

static void start_gp0_command(uint32_t value, GP0Command command)
{
	gpu_state.current_gp0_command = command;
	gpu_state.running_gp0_command = true;

	memset(gpu_state.command_buffer, 0, sizeof(gpu_state.command_buffer));
	gpu_state.command_buffer[0] = value;
	gpu_state.command_buffer_index = 1;
}

static void handle_gp0_command(uint32_t value)
{
	// If we are running a CPU to VRAM blit, consume the command instead as data
	if (gpu_state.blit_words_remaining)
	{
		// Transfer first half word
		int x_pos = (gpu_state.blit_position.x + gpu_state.blit_x_count) & 0x3FF;
		int y_pos = (gpu_state.blit_position.y + gpu_state.blit_y_count) & 0x1FF;

		gpu_state.vram[y_pos * 1024 + x_pos] = value & 0xFFFF;

		gpu_state.blit_x_count++;

		x_pos = (gpu_state.blit_position.x + gpu_state.blit_x_count) & 0x3FF;
		y_pos = (gpu_state.blit_position.y + gpu_state.blit_y_count) & 0x1FF;

		// Check for wrapping
		if (gpu_state.blit_x_count == gpu_state.blit_size.x)
		{
			gpu_state.blit_y_count++;
			gpu_state.blit_x_count = 0;

			x_pos = (gpu_state.blit_position.x + gpu_state.blit_x_count) & 0x3FF;
			y_pos = (gpu_state.blit_position.y + gpu_state.blit_y_count) & 0x1FF;
		}

		// Transfer second half word
		gpu_state.vram[y_pos * 1024 + x_pos] = (value & 0xFFFF0000) >> 16;

		gpu_state.blit_x_count++;

		// Check wrapping again
		if (gpu_state.blit_x_count == gpu_state.blit_size.x)
		{
			gpu_state.blit_y_count++;
			gpu_state.blit_x_count = 0;

			x_pos = (gpu_state.blit_position.x + gpu_state.blit_x_count) & 0x3FF;
			y_pos = (gpu_state.blit_position.y + gpu_state.blit_y_count) & 0x1FF;
		}

		gpu_state.blit_words_remaining--;

		if (gpu_state.blit_words_remaining == 0)
			finish_gp0_command();

		return;
	}

	if (gpu_state.running_gp0_command)
	{
		gp0_misc(value);
		return;
	}

	uint32_t command_mask = (0b111 << 29);
	uint32_t command = (value & command_mask) >> 29;

	switch (command)
	{
		case GP0_MISC:
			gp0_misc(value);
			break;

		case GP0_POLYGON:
			start_gp0_command(value, GP0_POLYGON);
			//log_info("GP0 command is %x\n", value);
			break;

		case GP0_LINE:
			log_warning("Received GPU line primitive command -- value is %x -- pc is %x\n", value, cpu_state.pc);
			break;

		case GP0_RECTANGLE:
			start_gp0_command(value, GP0_RECTANGLE);
			break;

		case GP0_VRAM_TO_VRAM_BLIT:
			log_warning("Received GPU VRAM-to-VRAM blit command -- value is %x\n", value);
			break;

		case GP0_CPU_TO_VRAM_BLIT:
			start_gp0_command(value, GP0_CPU_TO_VRAM_BLIT);
			break;

		case GP0_VRAM_TO_CPU_BLIT:
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
void update_gpustat()
{
	GPUStatus status = gpu_state.gpu_status;

	uint32_t new_stat = 0;

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
			finish_gp0_command();
			break;

		case GP1_ACK_IRQ:
			gpu_state.gpu_status.irq_1_on = false;
			update_gpustat();
			break;

		case GP1_DISPLAY_ENABLE:
			gpu_state.gpu_status.display_enable = value & 1;
			update_gpustat();
			break;

		case GP1_DMA_DIRECTION:
			gpu_state.gpu_status.dma_direction = (value & 0b11) << 29;
			update_gpustat();
			break;

		case GP1_DISPLAY_START_VRAM:
			gpu_state.display_area_start = value;
			break;

		case GP1_DISPLAY_RANGE_H:
			gpu_state.display_range_horizontal = value;
			break;

		case GP1_DISPLAY_RANGE_V:
			gpu_state.display_range_vertical = value;
			break;

		case GP1_DISPLAY_MODE:
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
