#include <stdint.h>

#include "dma.h"
#include "logging.h"
#include "memory.h"

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
		else if (dma_register == 4)
		{
			log_debug("DMA BCR read on channel %x\n", dma_channel);
			return dma_regs.channels[dma_channel].dma_bcr;
		}
		else if (dma_register == 8)
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

static void handle_dma_transfer(DMAChannel* channel)
{
	DMATransferState* state = &channel->transfer_state;

	if (state->transfer_mode == DMA_TRANSFER_LINKED_LIST && state->dma_direction == DMA_RAM_TO_DEVICE)
	{
		// The address of the first node
		uint32_t address = channel->dma_madr;
		// Get the first linked list node
		uint32_t ll_start = read_word(address);

		log_debug("Going through linked list starting at %x\n", ll_start);

		while (true)
		{
			
			// Get number of words from the 8 highest bits
			uint32_t words_to_transfer = (ll_start & 0xFF000000) >> 24;
			log_debug("%d words to transfer for this node\n", words_to_transfer);

			while (words_to_transfer--)
			{
				int increment = channel->transfer_state.madr_increment ? -4 : 4;
				address += increment;

				write_word(0x1F801810, read_word(address));
			}

			// Finish transfer if we have an end address
			if (address & 0x1FFFFC)
				break;

			address = ll_start & 0xFFFFFF;
			// The address is mapped into RAM
			ll_start = read_word(address);
		}
	}
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
			log_debug("DMA MADR write on channel %x -- value %x\n", dma_channel, value);
		}
		else if (dma_register == 4)
		{
			dma_regs.channels[dma_channel].dma_bcr = value;
			log_debug("DMA BCR write on channel %x -- value\n", dma_channel, value);
		}
		else if (dma_register == 8)
		{
			// TODO : Only some bits can be written to on channel 6 (OT)
			DMAChannel* channel = &dma_regs.channels[dma_channel];
			DMATransferState* state = &channel->transfer_state;
			
			// Update the channel state depending on the value written to it
			state->dma_direction = value & 1;
			state->madr_increment = (value & 0b10) >> 1;
			state->bit_8 = (value & (1 << 8)) >> 8;
			state->transfer_mode = (value & (0b11 << 9)) >> 9;
			state->chopping_dma_size = (value & (0b111 << 16)) >> 16;
			state->chopping_cpu_size = (value & (0b111 << 20)) >> 20;
			state->start_transfer = (value & (1 << 24)) >> 24;
			
			channel->dma_chcr = value;
			log_debug("DMA CHCR write on channel %x -- value %x\n", dma_channel, value);

			if (state->start_transfer)
			{
				log_debug("--- STARTING DMA TRANSFER ---\n");

				log_debug("Transfer direction is %s\n", dma_direction_str[state->dma_direction]);
				log_debug("Transfer mode is %s\n", dma_mode_str[state->transfer_mode]);

				handle_dma_transfer(channel);

				// Clear bit 24 to indicate that the transfer has been completed
				channel->dma_chcr &= ~(1 << 24);

				log_debug("--- ENDING DMA TRANSFER ---\n");
			}
			else
				log_debug("No DMA started...\n");
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
