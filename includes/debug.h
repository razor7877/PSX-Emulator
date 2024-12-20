#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "cpu.h"

#define MAX_BREAKPOINTS 8
#define CPU_TRACE_SIZE 40
#define TTY_BUFFER_SIZE (2048 * 32)

/// <summary>
/// Functions and state for debugging tools - This is emulator level debugging, not emulation
/// of the PSX hardware/software breakpoints
/// </summary>

typedef struct
{
	uint32_t address;
	bool in_use;
	bool break_on_data;
	bool break_on_code;
	bool enabled;
} Breakpoint;

typedef struct
{
	/// <summary>
	/// Stores information about the breakpoints used by the debugger
	/// </summary>
	Breakpoint code_breakpoints[MAX_BREAKPOINTS];

	/// <summary>
	/// The number of breakpoints used
	/// </summary>
	int breakpoint_count;

	/// <summary>
	/// Whether we are in currently in debug mode
	/// </summary>
	bool in_debug;

	/// <summary>
	/// Whether we should print the executed instructions to console
	/// </summary>
	bool print_instructions;

	/// <summary>
	/// Contains a trace of the CPU state during the last few instructions
	/// </summary>
	cpu cpu_trace[CPU_TRACE_SIZE];

	/// <summary>
	/// The current index of the earliest instruction in the CPU trace
	/// </summary>
	int trace_start;

	/// <summary>
	/// A buffer to store the data printed out to the TTY
	/// </summary>
	char tty[TTY_BUFFER_SIZE];

	/// <summary>
	/// The current index into the TTY
	/// </summary>
	int char_index;
} DebugState;

extern DebugState debug_state;

void reset_debug_state(bool reset_breakpoints);

/// <summary>
/// Queries user input for the debugger
/// </summary>
void handle_debug_input();

/// <summary>
/// Adds a new state to the CPU trace
/// </summary>
/// <param name="cpu_state">The current CPU state to be saved</param>
void add_cpu_trace(cpu cpu_state);

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
