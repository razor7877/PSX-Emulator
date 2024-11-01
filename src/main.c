#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "memory.h"
#include "logging.h"
#include "cpu.h"
#include "tests.h"
#include "debug.h"

const char bios_path[] = "roms/Sony PlayStation SCPH-1001 - DTLH-3000 BIOS v2.2 (1995-12-04)(Sony)(US).bin";

bool finished_bios_boot = false;

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

static void print_exe_header(exe_header file_header)
{
	log_info_no_prefix("--- SIDELOADED EXE INTO MEMORY ---\n");
	log_info_no_prefix("ASCII ID: %s\n", file_header.ascii_id);
	log_info_no_prefix("Initial PC: %x\n", file_header.initial_pc);
	log_info_no_prefix("Initial GP/R28: %x\n", file_header.initial_r28);
	log_info_no_prefix("Destination address: %x\n", file_header.destination_address);
	log_info_no_prefix("Filesize: %x\n", file_header.file_size);
	log_info_no_prefix("Data start address: %x\n", file_header.data_start_address);
	log_info_no_prefix("Data size: %x\n", file_header.data_size);
	log_info_no_prefix("BSS start address: %x\n", file_header.bss_start_address);
	log_info_no_prefix("BSS size: %x\n", file_header.bss_size);
	log_info_no_prefix("Initial R29/R30 base: %x\n", file_header.initial_r29_r30_address);
	log_info_no_prefix("Initial R29/R30 offset: %x\n", file_header.initial_r29_r30_offset);
}

/// <summary>
/// Attempts to sideload an EXE file into RAM after BIOS boot
/// </summary>
/// <returns>0 if the sideloading worked, -1 otherwise</returns>
static int sideload_exe()
{
	FILE* exe_file = fopen("roms/psxtest_cpu.exe", "rb");
	if (!exe_file)
	{
		log_error("Error while trying to load the EXE file!\n");
		return -1;
	}

	fseek(exe_file, 0, SEEK_END);
	size_t exe_size = ftell(exe_file);
	rewind(exe_file);

	if (exe_size < 0x800)
	{
		log_error("File is too small! This is probably not a PSX exe!\n");
		return -1;
	}

	exe_header file_header = {0};
	fread(&file_header, sizeof(uint8_t), sizeof(file_header), exe_file);
	rewind(exe_file);

	print_exe_header(file_header);

	sideload_exe_into_mem(file_header, exe_file);
	finished_bios_boot = true;

	return 0;
}

int main(int argc, char** argv)
{
	// Unit tests
	test_memory();
	test_instructions();

	// We need a loaded BIOS for the emulator to work
	if (load_bios(bios_path) != 0)
	{
		log_error("Couldn't load BIOS at startup!\n");
		return -1;
	}

	// Emulation loop
	for (;;)
	{
		if (!debug_state.in_debug)
		{
			handle_instruction(debug_state.print_instructions);

			//if (!finished_bios_boot && cpu_state.pc == 0x80030000)
			//	sideload_exe();
		}
		else // In debug mode, query user input
			handle_debug_input();
	}

	return 0;
}