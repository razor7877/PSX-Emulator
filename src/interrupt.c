#include <stdint.h>

#include "interrupt.h"
#include "cpu.h"

typedef struct
{
	uint32_t I_STAT;
	uint32_t I_MASK;
} interrupt;

interrupt interrupt_regs = {
	.I_STAT = 0,
	.I_MASK = 0
};

uint32_t read_interrupt_control(uint32_t address)
{
	if (address == 0x1F801070)
		return interrupt_regs.I_STAT;

	if (address == 0x1F801074)
		return interrupt_regs.I_MASK;

	log_warning("Unhandled interrupt control read at address %x!\n", cpu_state.pc);

	return 0xFFFFFFFF;
}

void write_interrupt_control(uint32_t address, uint32_t value)
{
	if (address == 0x1F801070)
	{
		log_warning("Unhandled I_STAT write!\n");
	}
	else if (address == 0x1F801074)
	{
		log_warning("Unhandled I_MASK write!\n");
	}
	else
		log_warning("Unhandled interrupt control write at address %x!\n", cpu_state.pc);
}