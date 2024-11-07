#include "dma.h"
#include "logging.h"

#define DMA_CHANNELS_START 0x1F801080
#define DMA_CHANNELS_END (0x1F8010E0 + 0x10)

DMA dma_regs = {
	.channels = {0},
	.dpcr = 0,
	.dicr = 0,
};

const char dma_mode_str[][20] = {
	"burst",
	"slice",
	"linked list",
	"reserved"
};

const char dma_direction_str[][20] = {
	"device to ram",
	"ram to device"
};

uint32_t read_dma_regs(uint32_t address)
{
	if (address >= DMA_CHANNELS_START && address < DMA_CHANNELS_END)
	{
		// DMA0 at 0x1F801080, DMA1 at 0x1F801090 etc.
		// We get the 4 bits that change to know which DMA channel was accessed
		uint32_t dma_channel = ((address & 0xF0) >> 4) - 8;

		// The address of the register in the DMA channel
		uint8_t dma_register = address & 0xF;

		if (dma_register == 0)
		{
			log_debug("DMA MADR read on channel %x\n", dma_channel);
			return dma_regs.channels[dma_channel].dma_madr;
		}
		
		if (dma_register == 4)
		{
			log_debug("DMA BCR read on channel %x\n", dma_channel);
			return dma_regs.channels[dma_channel].dma_bcr;
		}

		if (dma_register == 8)
		{
			log_debug("DMA CHCR read on channel %x\n", dma_channel);
			return dma_regs.channels[dma_channel].dma_chcr;
		}

		log_warning("Unhandled DMA channels read at address %x -- channel %x\n", address, dma_channel);
		return 0xFFFFFFFF;
	}

	if (address == 0x1F8010F0)
		return dma_regs.dpcr;

	if (address == 0x1F8010F4)
		return dma_regs.dicr;

	log_warning("Unhandled DMA registers read at address %x\n", address);

	return 0xFFFFFFFF;
}

void write_dma_regs(uint32_t address, uint32_t value)
{
	if (address >= DMA_CHANNELS_START && address < DMA_CHANNELS_END)
	{
		// DMA0 at 0x1F801080, DMA1 at 0x1F801090 etc.
		// We get the 4 bits that change to know which DMA channel was accessed
		uint32_t dma_channel = ((address & 0xF0) >> 4) - 8;

		// The address of the register in the DMA channel
		uint8_t dma_register = address & 0xF;

		if (dma_register == 0)
		{
			dma_regs.channels[dma_channel].dma_madr = value;
			log_debug("DMA MADR write on channel %x\n", dma_channel);
		}
		else if (dma_register == 4)
		{
			dma_regs.channels[dma_channel].dma_bcr = value;
			log_debug("DMA BCR write on channel %x\n", dma_channel);
		}
		else if (dma_register == 8)
		{
			DMAChannel* channel = &dma_regs.channels[dma_channel];
			DMATransferState* state = &channel->transfer_state;

			state->dma_direction = value & 1;
			state->madr_increment = (value & 0b10) >> 1;
			state->bit_8 = (value & (1 << 8)) >> 8;
			state->transfer_mode = (value & (0b11 << 9)) >> 9;
			state->chopping_dma_size = (value & (0b111 << 16)) >> 16;
			state->chopping_cpu_size = (value & (0b111 << 20)) >> 20;
			state->start_transfer = (value & (1 << 24)) >> 24;
			
			channel->dma_chcr = value;
			log_debug("DMA CHCR write on channel %x\n", dma_channel);

			log_debug("Transfer direction is %s\n", dma_direction_str[state->dma_direction]);
			log_debug("Transfer mode is %s\n", dma_mode_str[state->transfer_mode]);
		}
		else
			log_warning("Unhandled DMA channels write at address %x\n", address);
	}

	else if (address == 0x1F8010F0)
		dma_regs.dpcr = value;

	else if (address == 0x1F8010F4)
		dma_regs.dicr = value;

	else
		log_warning("Unhandled DMA registers write at address %x\n", address);
}
