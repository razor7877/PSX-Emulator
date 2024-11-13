#include <stdint.h>
#include <stdbool.h>

#include "interrupt.h"
#include "cpu.h"
#include "logging.h"
#include "coprocessor.h"

InterruptState interrupt_regs = {
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
		log_debug("Handling I_STAT write! Old value is %x\n", interrupt_regs.I_STAT);
		for (int irq = 0; irq < 11; irq++)
		{
			IRQ_MASK mask = 1 << irq;

			// An IRQ is acknowledged by setting the corresponding bit to 0 while 1 keeps the flag unchanged
			if ((value & mask) == 0)
				interrupt_regs.I_STAT &= ~mask;
		}
		log_debug("New I_STAT value %x\n", interrupt_regs.I_STAT);
	}
	else if (address == 0x1F801074)
	{
		// Only the 11 first bits are used
		log_debug("I_MASK write, old value is %x\n", interrupt_regs.I_MASK);
		interrupt_regs.I_MASK = value & 0x7FF;
		log_debug("New value is %x\n", interrupt_regs.I_MASK);
	}
	else
		log_warning("Unhandled interrupt control write at address %x!\n", cpu_state.pc);
}

void request_interrupt(IRQ_MASK irq)
{
	log_debug("Got an IRQ with flag %x\n", irq);
	uint32_t mask = 1 << irq;
	interrupt_regs.I_STAT |= mask;
}

void service_interrupts()
{
	for (int i = 0; i < 11; i++)
	{
		uint32_t mask = 1 << i;

		uint32_t i_mask_val = (interrupt_regs.I_MASK & mask) >> i;
		uint32_t i_stat_val = (interrupt_regs.I_STAT & mask) >> i;

		if (i_mask_val && i_stat_val)
		{
			log_debug("Got IRQ to service with flag %x!\n", i);
			// Set cop0r13 to request an IRQ to the coprocessor
			CPR0(13) |= 1 << 10;
			// Clear I_STAT bit
			interrupt_regs.I_STAT &= ~mask;
		}
	}

	bool im = CPR0(13) & (0b11111111 << 8);
	bool ip = CPR0(12) & (0b11111111 << 8);
	bool interrupt_enable = CPR0(12) & 0b1;

	if (interrupt_enable && ip && im)
	{
		CPR0(13) &= ~(0b11111111 << 8);
		handle_exception(INTERRUPT);
	}
}
