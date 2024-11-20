#include "cdrom.h"
#include "logging.h"
#include "debug.h"
#include "interrupt.h"

CDController cd_controller = {
	.status_register = 0b00011000,
	.current_index = 0,
	.interrupt_enable = 0,
	.interrupt_flag = 0,
	.parameter_fifo = {0},
	.response_fifo = {0},
	.data_fifo = {0},
	.pending_cdrom_irq = false,
	.cycles_until_irq = 0,
};

uint32_t read_cdrom(uint32_t address)
{
	//log_warning("Got CDROM read at address %x with index %d\n", address, cd_controller.current_index);
	if (address == 0x1F801800)
	{
		// Parameter FIFO empty
		if (cd_controller.parameter_fifo.index == 0)
			cd_controller.status_register |= 1 << 3; // Set parameter fifo empty bit
		else
			cd_controller.status_register &= ~(1 << 3);

		// Parameter FIFO full
		if (cd_controller.parameter_fifo.index == CD_FIFO_SIZE - 1)
			cd_controller.status_register &= ~(1 << 4); // Set parameter fifo empty bit
		else
			cd_controller.status_register |= 1 << 4;

		// Response FIFO empty
		if (cd_controller.response_fifo.index == 0)
			cd_controller.status_register &= ~(1 << 5); // Clear response fifo empty bit
		else
			cd_controller.status_register |= 1 << 5;

		// Data FIFO empty
		if (cd_controller.data_fifo.index == 0)
			cd_controller.status_register &= ~(1 << 6); // Clear data fifo empty bit
		else
			cd_controller.status_register |= 1 << 6;

		return cd_controller.status_register;
	}

	if (address == 0x1F801801)
	{
		log_warning("Unhandled CDROM Response FIFO read\n");
		//debug_state.in_debug = true;
		return 1;
	}

	if (address == 0x1F801802)
	{
		log_warning("Unhandled CDROM Data FIFO read -- pc %x\n", cpu_state.pc);
		//debug_state.in_debug = true;
		return 1;
	}

	if (address == 0x1F801803)
	{
		if (cd_controller.current_index == 0 || cd_controller.current_index == 2)
			return cd_controller.interrupt_enable;

		if (cd_controller.current_index == 1 || cd_controller.current_index == 3)
		{
			cd_controller.interrupt_flag &= ~0b111;
			cd_controller.interrupt_flag |= 0x3;
			return cd_controller.interrupt_flag;
		}
	}

	log_warning("Unhandled CDROM read at address %x\n", address);

	return 0xFFFFFFFF;
}

void write_cdrom(uint32_t address, uint32_t value)
{
	log_info("Got CDROM write at address %x with value %x -- pc is %x\n", address, value, cpu_state.pc);
	if (address == 0x1F801800)
	{
		// Clear the 2 writable bits
		cd_controller.status_register &= ~0b11;
		// Set the 2 bits from written value
		cd_controller.status_register |= (value & 0b11);
		cd_controller.current_index = value & 0b11;

		log_debug("Got CDROM write, new controller port index is %x with reg value %x\n",
			cd_controller.current_index,
			cd_controller.status_register
		);
	}
	else if (address == 0x1F801801)
	{
		if (cd_controller.current_index == 0)
		{
			log_warning("Unhandled CDROM command with value %x\n", value & 0xFF);
			cd_controller.pending_cdrom_irq = true;
			cd_controller.cycles_until_irq = 500;
		}
		else if (cd_controller.current_index == 1)
		{
			log_warning("Unhandled CDROM Sound Map Data Out register write --- value %x\n", value);
		}
		else if (cd_controller.current_index == 2)
		{
			log_warning("Unhandled CDROM Sound Map Coding Info register write --- value %x\n", value);
		}
		else if (cd_controller.current_index == 3)
		{
			log_warning("Unhandled CDROM Right-CD to Right-SPU register write --- value %x\n", value);
		}
	}
	else if (address == 0x1F801802)
	{
		if (cd_controller.current_index == 0)
		{
			log_warning("Unhandled CDROM parameter FIFO register write --- value %x\n", value);
		}
		else if (cd_controller.current_index == 1)
		{
			cd_controller.interrupt_enable = value & 0b11111;
			log_warning("Unhandled CDROM interrupt enable register write --- value %x\n", value);
		}
		else if (cd_controller.current_index == 2)
		{
			log_warning("Unhandled CDROM Left-CD to Left-SPU register write --- value %x\n", value);
		}
		else if (cd_controller.current_index == 3)
		{
			log_warning("Unhandled CDROM Right-CD to Left-SPU register write --- value %x\n", value);
		}
	}
	else if (address == 0x1F801803)
	{
		if (cd_controller.current_index == 0)
		{
			log_warning("Unhandled CDROM request register write --- value %x\n", value);
		}
		else if (cd_controller.current_index == 1)
		{
			cd_controller.interrupt_flag = value;
		}
		else if (cd_controller.current_index == 2)
		{
			log_warning("Unhandled CDROM Left-CD to Right-SPU Volume register write --- value %x\n", value);
		}
		else if (cd_controller.current_index == 3)
		{
			log_warning("Unhandled CDROM Audio Volume Apply Changes register write --- value %x\n", value);
		}
	}
	else
		log_warning("Unhandled CDROM write at address %x\n", address);
}

void tick_cdrom(int cycles)
{
	if (cd_controller.pending_cdrom_irq && cd_controller.cycles_until_irq > 0)
	{
		cd_controller.cycles_until_irq -= cycles;

		if (cd_controller.cycles_until_irq <= 0)
		{
			request_interrupt(IRQ_CDROM);
			cd_controller.pending_cdrom_irq = false;
		}
	}
}
