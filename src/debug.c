#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "debug.h"
#include "logging.h"
#include "cpu.h"

debug_struct debug_state = {
	.code_breakpoints = {0},
	.breakpoint_count = 0,
	.in_debug = false
};

char input[256];

static void set_breakpoint_user()
{
	log_info_no_prefix("Address to break on: ");
	fgets(input, sizeof(input), stdin);

	uint32_t address = strtoull(input, NULL, 16);
	log_info_no_prefix("Adding breakpoint at address %x\n", address);

	add_breakpoint(address, true, false);
}

static void delete_breakpoint_user()
{
	log_info_no_prefix("Index to delete: ");
	fgets(input, sizeof(input), stdin);

	int index = strtoul(input, NULL, 10);

	log_info_no_prefix("Deleting breakpoint %d\n", index);

	delete_breakpoint(index);
}

static void show_breakpoints()
{
	log_info_no_prefix("Showing breakpoints:\n");
	for (int i = 0; i < debug_state.breakpoint_count; i++)
	{
		breakpoint br = debug_state.code_breakpoints[i];
		log_info_no_prefix("%d : %08x --- Code: %d / Data: %d\n", i, br.address, br.break_on_code, br.break_on_data);
	}
}

static void continue_execution()
{
	log_info_no_prefix("Resume execution\n");
	debug_state.in_debug = false;
}

static void show_cpu_state()
{
	for (int i = 0; i < 8; i ++)
	{
		log_info_no_prefix("R%02d: %08x R%02d: %08x R%02d: %08x R%02d: %08x\n",
			i * 4, cpu_state.registers[i * 4],
			i * 4 + 1, cpu_state.registers[i * 4 + 1],
			i * 4 + 2, cpu_state.registers[i * 4 + 2],
			i * 4 + 3, cpu_state.registers[i * 4 + 3]
		);
	}

	log_info_no_prefix("\tpc: %x --- hi: %x --- lo: %x\n", cpu_state.pc, cpu_state.hi, cpu_state.lo);
}

void handle_debug_input()
{
	fgets(input, sizeof(input), stdin);

	switch (input[0])
	{
		case 's': // Add breakpoint
			set_breakpoint_user();
			break;

		case 'u': // Delete breakpoint
			delete_breakpoint_user();
			break;

		case 'l': // Show breakpoints
			show_breakpoints();
			break;

		case 'c': // Resume execution
			continue_execution();
			break;

		case 'n': // Step next instruction
			handle_instruction(true);
			break;

		case 'r': // Show registers / CPU state
			show_cpu_state();
			break;

		default:
			log_info_no_prefix("Use the following commands to interact with the debugger:\n\ts - set breakpoint\n\tu - unset breakpoint\n\tl - list breakpoints\n\tc - continue execution\n\tn - step next instruction\n\tr - show registers\n");
			break;
	}
}

void check_code_breakpoints(uint32_t address)
{
	// Check for any reached breakpoints
	for (int i = 0; i < debug_state.breakpoint_count; i++)
	{
		breakpoint br = debug_state.code_breakpoints[i];

		if (br.enabled && br.break_on_code && address == br.address)
		{
			log_info_no_prefix("Reached code breakpoint at %x\n", debug_state.code_breakpoints[i]);
			debug_state.in_debug = true;
		}
	}
}

void check_data_breakpoints(uint32_t address)
{
	// Check for any reached breakpoints
	for (int i = 0; i < debug_state.breakpoint_count; i++)
	{
		breakpoint br = debug_state.code_breakpoints[i];

		if (br.enabled && br.break_on_data && address == br.address)
		{
			log_info_no_prefix("Reached data breakpoint at %x\n", debug_state.code_breakpoints[i]);
			debug_state.in_debug = true;
		}
	}
}

void add_breakpoint(uint32_t address, bool break_on_code, bool break_on_data)
{
	if (debug_state.breakpoint_count >= MAX_BREAKPOINTS)
	{
		log_error("Max breakpoint amount already reached!\n");
		return;
	}

	debug_state.code_breakpoints[debug_state.breakpoint_count].enabled = true;
	debug_state.code_breakpoints[debug_state.breakpoint_count].address = address;
	debug_state.code_breakpoints[debug_state.breakpoint_count].break_on_code = break_on_code;
	debug_state.code_breakpoints[debug_state.breakpoint_count].break_on_data = break_on_data;

	debug_state.breakpoint_count++;
}

void delete_breakpoint(int index)
{
	if (index < 0 || index >= debug_state.breakpoint_count)
	{
		log_error("Incorrect index when trying to delete code breakpoint!\n");
		return;
	}

	debug_state.code_breakpoints[index].address = true;
	debug_state.code_breakpoints[index].enabled = false;
	debug_state.code_breakpoints[index].break_on_code = false;
	debug_state.code_breakpoints[index].break_on_data = false;
}
