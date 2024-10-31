#pragma once

#include <stdint.h>

/// <summary>
/// A struct used to contain the header data for a PSX EXE file 
/// </summary>
typedef struct
{
	uint8_t ascii_id[8];
	uint8_t zero_filled[8];
	uint32_t initial_pc;
	uint32_t initial_r28;
	uint32_t destination_address;
	uint32_t file_size;
	uint32_t data_start_address;
	uint32_t data_size;
	uint32_t bss_start_address;
	uint32_t bss_size;
	uint32_t initial_r29_r30_address;
	int32_t initial_r29_r30_offset;
	uint8_t a_func_placeholder[20];
} exe_header;
