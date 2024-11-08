#include <stdint.h>

#include "interrupt.h"
#include "cpu.h"
#include "logging.h"

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
	log_warning("Got an IRQ with flag %x\n", irq);
}

void service_interrupts()
{

}
