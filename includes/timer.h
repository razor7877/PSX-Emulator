#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	SYNC_MODE_0,
	SYNC_MODE_1,
	SYNC_MODE_2,
	SYNC_MODE_3,
} SyncMode;

typedef enum
{
	CLOCK_SRC_0,
	CLOCK_SRC_1,
	CLOCK_SRC_2,
	CLOCK_SRC_3,
} ClockSource;

typedef struct
{
	uint16_t counter_value;
	uint16_t counter_target;
	uint32_t counter_mode;

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
	Timer timers[3];
} TimerState;

uint32_t read_timer(uint32_t address);
void write_timer(uint32_t address, uint32_t value);

void tick_timers(int cycles);
