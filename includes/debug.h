#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MAX_BREAKPOINTS 32

typedef struct
{
	uint32_t code_breakpoints[MAX_BREAKPOINTS];
	int breakpoint_count;
	bool in_debug;
} debug_struct;

extern debug_struct debug_state;

/// <summary>
/// Queries user input for the debugger
/// </summary>
void handle_debug_input();

/// <summary>
/// Checks an address and triggers breakpoints if necessary
/// </summary>
/// <param name="address">The address to check</param>
/// <returns></returns>
void check_break_address(uint32_t address);

/// <summary>
/// Adds a new code breakpoint
/// </summary>
/// <param name="address">The address to break on</param>
void add_code_breakpoint(uint32_t address);

/// <summary>
/// Removes a code breakpoint
/// </summary>
/// <param name="index">The index of the breakpoint to remove</param>
void delete_code_breakpoint(int index);
