#include "cdrom.h"
#include "logging.h"
#include "debug.h"

CDController cd_controller = {
	.current_index = 0,
	.status_register = 0b00011000,
	.interrupt_enable = 0,
	.interrupt_flag = 0,
};

uint32_t read_cdrom(uint32_t address)
{
	if (address == 0x1F801800)
		return cd_controller.status_register;

	if (address == 0x1F801801)
	{
		log_warning("Unhandled CDROM Response FIFO read\n");
		return 0xFFFFFFFF;
	}

	if (address == 0x1F801802)
	{
		log_warning("Unhandled CDROM Data FIFO read\n");
		return 0xFFFFFFFF;
	}

	if (address == 0x1F801803)
	{
		if (cd_controller.current_index == 0 || cd_controller.current_index == 2)
		{
			log_warning("Unhandled CDROM Interrupt Enable register read\n");
			return 0xFFFFFFFF;
		}

		if (cd_controller.current_index == 1 || cd_controller.current_index == 3)
		{
			log_warning("Unhandled CDROM Interrupt Flag register read\n");
			return 0xFFFFFFFF;
		}
	}

	log_warning("Unhandled CDROM read at address %x\n", address);

	return 0xFFFFFFFF;
}

void write_cdrom(uint32_t address, uint32_t value)
{
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
			log_warning("Unhandled CDROM command with value %x\n", value);
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
			log_warning("Unhandled CDROM interrupt flag register write --- value %x\n", value);
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
