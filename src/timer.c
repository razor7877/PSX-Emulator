#include <stdbool.h>

#include "timer.h"
#include "logging.h"
#include "cpu.h"
#include "gpu.h"

TimerState timer_state = {0};

uint32_t read_timer(uint32_t address)
{
	uint32_t timer_channel = (address & 0xF0) >> 4;
	Timer* timer = &timer_state.timers[timer_channel];

	// The address of the register in the timer
	uint8_t timer_register = address & 0xF;

	if (timer_register == 0)
		return timer->counter_value;
	
	if (timer_register == 4)
	{
		uint32_t value = timer->counter_mode;
		// After reading, bit 10 and 11 are cleared (reached target/max value bits)
		timer->counter_mode &= ~(0b11 << 11);

		return value;
	}
	
	if (timer_register == 8)
		return timer->counter_target;

	log_warning("Unhandled timer read at address %x -- timer %d register %d\n", address, timer_channel, timer_register);

	return 0xFFFFFFFF;
}

void write_timer(uint32_t address, uint32_t value)
{
	uint32_t timer_channel = (address & 0xF0) >> 4;
	Timer* timer = &timer_state.timers[timer_channel];

	// The address of the register in the timer
	uint8_t timer_register = address & 0xF;

	if (timer_register == 0)
	{
		timer->counter_value = value & 0xFFFF;
	}
	else if (timer_register == 4)
	{
		timer->counter_mode = value;
		// Counter mode write resets counter value to 0
		timer->counter_value = 0;

		timer->sync_enable = value & 0b1;
		timer->sync_mode = (value & 0b110) >> 1;
		timer->reset_on_target = (value & (1 << 3)) >> 3;
		timer->irq_on_target = (value & (1 << 4)) >> 4;
		timer->irq_on_max_value = (value & (1 << 5)) >> 5;
		timer->repeat_irq = (value & (1 << 6)) >> 6;
		timer->irq_pulse_toggle = (value & (1 << 7)) >> 7;
		timer->clock_source = (value & (0b11 << 8)) >> 8;
		timer->irq_bit = (value & (1 << 10)) >> 10;
		timer->reached_target = (value & (1 << 11)) >> 12;
		timer->reached_max_value = (value & (1 << 11)) >> 12;
	}
	else if (timer_register == 8)
	{
		timer->counter_target = value & 0xFFFF;
	}
	else
	{
		log_warning("Unhandled timer write at address %x with value %x -- timer %d register %d\n",
			address,
			value,
			timer_channel,
			timer_register
		);
	}
}

void system_clock_tick(int cycles)
{
	bool hblank_tick = false;
	bool dot_clock_tick = false;
	bool sys_clock_8_tick = false;

	timer_state.hblank_clock_internal += cycles;

	if (timer_state.hblank_clock_internal >= (NTSC_SCANLINE_CYCLES * GPU_TO_CPU_CYCLES))
	{
		timer_state.hblank_clock_internal = 0;
		hblank_tick = true;
	}

	timer_state.dot_clock_internal += cycles;

	int dot_clock = 0;

	if (gpu_state.gpu_status.h_res_2 == H_RES_368)
		dot_clock = DOT_CLK_368;
	else if (gpu_state.gpu_status.h_res_1 == H_RES_256)
		dot_clock = DOT_CLK_256;
	else if (gpu_state.gpu_status.h_res_1 == H_RES_320)
		dot_clock = DOT_CLK_320;
	else if (gpu_state.gpu_status.h_res_1 == H_RES_512)
		dot_clock = DOT_CLK_512;
	else if (gpu_state.gpu_status.h_res_1 == H_RES_640)
		dot_clock = DOT_CLK_640;

	if (timer_state.dot_clock_internal >= (CPU_FREQ / dot_clock))
	{
		timer_state.dot_clock_internal = 0;
		dot_clock_tick = true;
	}

	timer_state.sys_clock_8_internal += cycles;

	if (timer_state.sys_clock_8_internal == SYS_CLOCK_8_CYCLES)
	{
		sys_clock_8_tick = true;
		timer_state.sys_clock_8_internal = 0;
	}

	for (int i = 0; i < 3; i++)
	{
		Timer* timer = &timer_state.timers[i];
		uint32_t value = timer->counter_value;
		uint32_t target = timer->counter_target;

		ClockSourceType clock_type = 0;

		// Check the clock source for the timer
		switch (timer->clock_source)
		{
			case CLOCK_SRC_0:
				clock_type = SYS_CLOCK;
				break;

			case CLOCK_SRC_1:
				if (i == 0)
					clock_type = DOT_CLOCK;
				else if (i == 1)
					clock_type = HBLANK_CLOCK;
				else
					clock_type = SYS_CLOCK;
				break;

			case CLOCK_SRC_2:
				if (i == 2)
					clock_type = SYS_CLOCK_8;
				else
					clock_type = SYS_CLOCK;
				break;

			case CLOCK_SRC_3:
				if (i == 0)
					clock_type = DOT_CLOCK;
				else if (i == 1)
					clock_type = HBLANK_CLOCK;
				else
					clock_type = SYS_CLOCK_8;
				break;
		}

		if (clock_type == SYS_CLOCK)
			timer->counter_value++;
		else if (clock_type == SYS_CLOCK_8 && sys_clock_8_tick)
			timer->counter_value++;
		else if (clock_type == HBLANK_CLOCK && hblank_tick)
			timer->counter_value++;
		else if (clock_type == DOT_CLOCK && dot_clock_tick)
			timer->counter_value++;

		// The timer counts up to and including the target value
		if (timer->reset_on_target && timer->counter_value == timer->counter_target)
		{
			timer->counter_value = 0;

			if (timer->irq_on_target && timer->irq_bit)
			{
				log_warning("Unhandled timer IRQ request on target\n");
			}
		}
		else if (timer->counter_value == 0xFFFF)
		{
			timer->counter_value = 0;

			if (timer->irq_on_max_value && timer->irq_bit)
			{
				log_warning("Unhandled timer IRQ request on max value\n");
			}
		}
	}
}
