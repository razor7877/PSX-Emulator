#pragma once

#include <stdint.h>
#include <stdbool.h>

#define DOT_CLOCK_CYCLES 
#define SYS_CLOCK_8_CYCLES 8

/// <summary>
/// Functions and state for emulating the timer behavior
/// </summary>

typedef enum
{
	SYNC_MODE_0, // Pause counter during HBlank
	SYNC_MODE_1, // Reset counter to 0 at HBlank
	SYNC_MODE_2, // Reset counter to 0 at HBlank + pause outside of HBlank
	SYNC_MODE_3, // Pause until HBlank occurs, then switch to free run
} SyncMode;

typedef enum
{
	CLOCK_SRC_0,
	CLOCK_SRC_1,
	CLOCK_SRC_2,
	CLOCK_SRC_3,
} ClockSource;

typedef enum
{
	SYS_CLOCK,
	SYS_CLOCK_8,
	HBLANK_CLOCK,
	DOT_CLOCK
} ClockSourceType;

typedef struct
{
	uint32_t counter_value;
	uint16_t counter_target;
	uint32_t counter_mode;

	bool sync_enable;
	SyncMode sync_mode;
	bool reset_on_target;
	bool irq_on_target;
	bool irq_on_max_value;
	bool repeat_irq;
	bool irq_pulse_toggle;
	ClockSource clock_source;
	bool irq_bit;
	bool reached_target;
	bool reached_max_value;
} Timer;

typedef struct
{
	/// <summary>
	/// The state of timers 0-2
	/// </summary>
	Timer timers[3];

	/// <summary>
	/// An internal counter to keep track of the HBlank clock source
	/// </summary>
	int hblank_clock_internal;

	/// <summary>
	/// An internal counter to keep track of the dot clock counter
	/// </summary>
	int dot_clock_internal;

	/// <summary>
	/// An internal counter to keep track of the 1/8th system clock clock source
	/// </summary>
	int sys_clock_8_internal;
} TimerState;

void reset_timer_state();

uint32_t read_timer(uint32_t address);
void write_timer(uint32_t address, uint32_t value);

void system_clock_tick(int cycles);
