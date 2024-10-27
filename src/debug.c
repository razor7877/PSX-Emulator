#include <stdint.h>
#include <stdio.h>

#include "debug.h"
#include "logging.h"

debug_struct debug_state = {
	.code_breakpoints = {0},
	.breakpoint_count = 0,
	.in_debug = false
};

char input[256];

static void set_breakpoint()
{
	log_info_no_prefix("Address to break on: ");
	fgets(input, sizeof(input), stdin);

	uint32_t address = strtoull(input, NULL, 16);
	log_info_no_prefix("Adding breakpoint at address %x\n", address);

	debug_state.code_breakpoints[debug_state.breakpoint_count] = address;
	debug_state.breakpoint_count++;
}

static void delete_breakpoint()
{
	log_info_no_prefix("Index to delete: ");
	fgets(input, sizeof(input), stdin);

	int index = strtoul(input, NULL, 10);

	log_info_no_prefix("Deleting breakpoint %d\n", index);

	if (index > 0 && index < debug_state.breakpoint_count)
		debug_state.code_breakpoints[index] = 0x00;

}

static void show_breakpoints()
{
	log_info_no_prefix("Showing breakpoints:\n");
	for (int i = 0; i < debug_state.breakpoint_count; i++)
		log_info_no_prefix("%d : %x\n", i, debug_state.code_breakpoints[i]);
}

static void continue_execution()
{
	log_info_no_prefix("Resume execution\n");
}

void handle_debug_input()
{
	fgets(input, sizeof(input), stdin);

	switch (input[0])
	{
		case 's':
			set_breakpoint();
			break;

		case 'u':
			delete_breakpoint();
			break;

		case 'l':
			show_breakpoints();
			break;

		case 'c':
			continue_execution();
			break;

		default:
			log_info_no_prefix("Use the following commands to interact with the debugger: s(et), u(nset), l(ist), c(ontinue)\n");
			break;
	}
}

void check_break_address(uint32_t address)
{
	// Check for any reached breakpoints
	for (int i = 0; i < debug_state.breakpoint_count; i++)
	{
		if (address == debug_state.code_breakpoints[i] && debug_state.code_breakpoints[i] != 0x00)
		{
			log_info_no_prefix("Reached code breakpoint at %x\n", debug_state.code_breakpoints[i]);
			debug_state.in_debug = true;
		}
	}
}

void add_code_breakpoint(uint32_t address)
{
	if (debug_state.breakpoint_count >= MAX_BREAKPOINTS)
	{
		log_error("Max breakpoint amount already reached!\n");
		return;
	}

	debug_state.code_breakpoints[debug_state.breakpoint_count] = address;
	debug_state.breakpoint_count++;
}

void delete_code_breakpoint(int index)
{
	if (index < 0 || index >= debug_state.breakpoint_count)
	{
		log_error("Incorrect index when trying to delete code breakpoint!\n");
		return;
	}

	debug_state.code_breakpoints[index] = 0x00;
}
