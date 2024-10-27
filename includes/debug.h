#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MAX_BREAKPOINTS 32

typedef struct
{
	uint32_t address;
	bool break_on_data;
	bool break_on_code;
	bool enabled;
} breakpoint;

typedef struct
{
	breakpoint code_breakpoints[MAX_BREAKPOINTS];
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
void check_code_breakpoints(uint32_t address);

/// <summary>
/// Checks an address and triggers breakpoints if necessary
/// </summary>
/// <param name="address">The address to check</param>
void check_data_breakpoints(uint32_t address);

/// <summary>
/// Adds a new code breakpoint
/// </summary>
/// <param name="address">The address to break on</param>
void add_breakpoint(uint32_t address, bool break_on_code, bool break_on_data);

/// <summary>
/// Removes a code breakpoint
/// </summary>
/// <param name="index">The index of the breakpoint to remove</param>
void delete_breakpoint(int index);
