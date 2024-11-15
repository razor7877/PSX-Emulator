#pragma once

#include <stdint.h>

typedef struct
{
	uint8_t status_register;
	uint8_t current_index;
	uint8_t interrupt_enable;
	uint8_t interrupt_flag;
} CDController;

uint32_t read_cdrom(uint32_t address);
void write_cdrom(uint32_t address, uint32_t value);
