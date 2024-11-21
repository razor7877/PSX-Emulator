#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cpu.h"

#define MAX_BREAKPOINTS 32
#define CPU_TRACE_SIZE 40

/// <summary>
/// Functions and state for debugging tools - This is emulator level debugging, not emulation
/// of the PSX hardware/software breakpoints
/// </summary>

typedef struct
{
	uint32_t address;
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
} DebugState;

extern DebugState debug_state;

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
