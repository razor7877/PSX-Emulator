#include "timer.h"
#include "logging.h"

TimerState timer_state = {0};

uint32_t read_timer(uint32_t address)
{
	uint32_t timer_channel = (address & 0xF0) >> 4;

	// The address of the register in the timer
	uint8_t timer_register = address & 0xF;

	if (timer_register == 0)
		return timer_state.timers[timer_channel].counter_value;
	
	if (timer_register == 4)
		return timer_state.timers[timer_channel].counter_mode;
	
	if (timer_register == 8)
		return timer_state.timers[timer_channel].counter_target;

	log_warning("Unhandled timer read at address %x -- timer %d register %d\n", address, timer_channel, timer_register);

	return 0xFFFFFFFF;
}

void write_timer(uint32_t address, uint32_t value)
{
	uint32_t timer_channel = (address & 0xF0) >> 4;

	// The address of the register in the timer
	uint8_t timer_register = address & 0xF;

	if (timer_register == 0)
	{
		timer_state.timers[timer_channel].counter_value = value & 0xFFFF;
	}
	else if (timer_register == 4)
	{
		timer_state.timers[timer_channel].counter_mode = value;


	}
	else if (timer_register == 8)
	{
		timer_state.timers[timer_channel].counter_target = value & 0xFFFF;
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
