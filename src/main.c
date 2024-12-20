#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "main.h"
#include "memory.h"
#include "cpu.h"
#include "debug.h"
#include "tests.h"
#include "frontend/gl.h"
#include "logging.h"
#include "gpu.h"
#include "interrupt.h"

const char bios_path[] = "roms/Sony PlayStation SCPH-1001 - DTLH-3000 BIOS v2.2 (1995-12-04)(Sony)(US).bin";
const char exe_path[] = "roms/psxtest_cpu.exe";

MainState main_state = {
	.finished_bios_boot = false,
	.file_header = {0},
	.exe_contents = NULL,
};

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

static void print_exe_header(EXEHeader file_header)
{
	log_info_no_prefix("--- SIDELOADED EXE INTO MEMORY ---\n\n");
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
	log_info_no_prefix("\n--- SIDELOADED EXE HEADER END ---\n");
}

int load_exe(const char* exe_path)
{
	FILE* exe_file = fopen(exe_path, "rb");
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

	memset(&main_state.file_header, 0, sizeof(main_state.file_header));
	fread(&main_state.file_header, sizeof(uint8_t), sizeof(main_state.file_header), exe_file);
	rewind(exe_file);

	print_exe_header(main_state.file_header);

	if (main_state.exe_contents != NULL)
		free(main_state.exe_contents);

	main_state.exe_contents = malloc(main_state.file_header.file_size);

	if (main_state.exe_contents == NULL)
	{
		log_error("Error while trying to allocate memory for the EXE contents!\n");
		return -1;
	}

	// Seek the end of the header to load the actual EXE data
	fseek(exe_file, 0x800, SEEK_SET);
	// Load the EXE data into RAM
	fread(main_state.exe_contents, sizeof(uint32_t), main_state.file_header.file_size / 4, exe_file);

	main_state.finished_bios_boot = false;

	return 0;
}

/// <summary>
/// Attempts to sideload an EXE file into RAM after BIOS boot
/// </summary>
/// <returns>0 if the sideloading worked, -1 otherwise</returns>
static int sideload_exe()
{
	sideload_exe_into_mem(main_state.file_header, main_state.exe_contents);
	main_state.finished_bios_boot = true;

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

	if (load_exe(exe_path) != 0)
		log_warning("Couldn't load EXE file at startup!\n");

	if (start_interface() != 0)
	{
		log_error("Couldn't start interface!\n");
		return -1;
	}

	int frame_count = 0;

	// Emulation loop
	while (update_interface() == 0)
	{
		if (debug_state.in_debug)
			continue;

		int cycle_count = 0;

		// Run emulation until we finish a frame or we encounter a breakpoint
		while (cycle_count < NTSC_FRAME_CYCLE_COUNT && !debug_state.in_debug)
		{
			handle_instruction(debug_state.print_instructions);
			cycle_count++;

			if (!main_state.finished_bios_boot && cpu_state.pc == 0x80030000)
				sideload_exe();
		}

		frame_count++;

		if (frame_count == 59)
		{
			frame_count = 0;
			request_interrupt(IRQ_VBLANK);
		}

		// TODO : Implement proper timings and emulate this correctly
		gpu_state.gpu_status.drawing_odd_lines = !gpu_state.gpu_status.drawing_odd_lines;
		update_gpustat();
	}

	stop_interface();

	return 0;
}
