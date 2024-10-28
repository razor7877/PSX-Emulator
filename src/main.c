#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "logging.h"
#include "cpu.h"
#include "tests.h"
#include "debug.h"

const char bios_path[] = "roms/Sony PlayStation SCPH-1001 - DTLH-3000 BIOS v2.2 (1995-12-04)(Sony)(US).bin";

static int load_bios(const char* path)
{
	FILE* bios_file = fopen(path, "rb");
	if (!bios_file)
	{
		log_error("Error while trying to load the BIOS file!\n");
		return -1;
	}

	fseek(bios_file, 0, SEEK_END);
	size_t bios_size = ftell(bios_file);
	rewind(bios_file);

	if (bios_size != 512 * KIB_SIZE)
		log_warning("BIOS file size is not 512 KiB\n");

	load_bios_into_mem(bios_file);

	return 0;
}

int main(int argc, char** argv)
{
	test_memory();
	test_instructions();

	if (load_bios(bios_path) != 0)
	{
		log_error("Couldn't load BIOS at startup!\n");
		return -1;
	}

	// BIOS finished execution
	add_breakpoint(0x80030000, true, false);

	//add_breakpoint(0xBFC00004, true, false);

	//add_breakpoint(0xB0, true, false);
	add_breakpoint(0xB0, false, true);
	add_breakpoint(0xBFC0686C, true, false);

	for (;;)
	{
		if (!debug_state.in_debug)
		{
			handle_instruction(false);

			if ((cpu_state.pc == 0xA0 && R9 == 0x3C) || (cpu_state.pc == 0xB0 && R9 == 0x3D))
			{
				char c = (char)(uint8_t)R4;
				printf("%c\n", c);
			}

			check_code_breakpoints(cpu_state.pc);
		}
		else
			handle_debug_input();
	}

	return 0;
}