#include "gpu.h"
#include "logging.h"

uint32_t gp0;
uint32_t gp1;
uint32_t gpu_read = 0;
//uint32_t gpu_stat = 0x10000000;
uint32_t gpu_stat = 0xFFFFFFFF;

uint32_t read_gpu(uint32_t address)
{
	if (address == 0x1F801810)
		return gpu_read;

	if (address == 0x1F801814)
		return gpu_stat;

	log_warning("Unhandled GPU register read at address %x\n", address);

	return 0xFFFFFFFF;
}

static void handle_gp0_command(uint32_t value)
{
	uint32_t command_mask = (0b111 << 29);
	uint32_t command = (value & command_mask) >> 29;

	switch (command)
	{
		case 0b000:
			log_warning("Received GPU misc command\n");
			break;

		case 0b001:
			log_warning("Received GPU polygon primitive command\n");
			break;

		case 0b010:
			log_warning("Received GPU line primitive command\n");
			break;

		case 0b011:
			log_warning("Received GPU rectangle primitive command\n");
			break;

		case 0b100:
			log_warning("Received GPU VRAM-to-VRAM blit command\n");
			break;

		case 0b101:
			log_warning("Received GPU CPU-to-VRAM blit command\n");
			break;

		case 0b110:
			log_warning("Received GPU VRAM-to-CPU blit command\n");
			break;

		case 0b111:
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
		case 0x0:
			log_warning("GP1 Command: Reset GPU\n");
			break;

		case 0x1:
			log_warning("GP1 Command: Reset command buffer\n");
			break;

		case 0x2:
			log_warning("GP1 Command: ACK GPU IRQ\n");
			break;

		case 0x3:
			log_warning("GP1 Command: Display enable\n");
			break;

		case 0x4:
			log_warning("GP1 Command: DMA Direction/Data request\n");
			break;

		case 0x5:
			log_warning("GP1 Command: Start of display area in VRAM\n");
			break;

		case 0x6:
			log_warning("GP1 Command: Horizontal display range\n");
			break;

		case 0x7:
			log_warning("GP1 Command: Vertical display range\n");
			break;

		case 0x8:
			log_warning("GP1 Command: Display mode\n");
			break;

		case 0x9:
			log_warning("GP1 Command: Set VRAM size (v2)\n");
			break;

		case 0x10:
			log_warning("GP1 Command: Read GPU internal register\n");
			break;

		case 0x20:
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
}
