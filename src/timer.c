#include "timer.h"
#include "logging.h"

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
	for (int i = 0; i < 3; i++)
	{
		Timer* timer = &timer_state.timers[i];
		uint32_t value = timer->counter_value;
		uint32_t target = timer->counter_target;

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
