#include <stdint.h>

#include "coprocessor.h"
#include "cpu.h"
#include "logging.h"

#define cop0_code(value) ((((value & 0x03E00000) >> 21)))
#define CPR0(value) _cop0_registers[value]

uint32_t _cop0_registers[64] = { 0 };

void mfc()
{
    
}

void cfc()
{

}

void mtc()
{
    uint32_t dest_reg = rd(current_opcode);

    if (dest_reg > 31)
        log_error("MFC0 attempted to move to register outside of data registers\n");
    else
        CPR0(dest_reg) = R(rt(current_opcode));
}

void ctc()
{
    uint32_t dest_reg = rd(current_opcode);

    if (dest_reg < 32 || dest_reg > 63)
        log_error("CFC0 attempted to move to register outside of control registers\n");
    else
        CPR0(dest_reg) = R(rt(current_opcode));
}

void handle_cop0_instruction()
{
    uint8_t opcode = cop0_code(current_opcode);

    switch (opcode)
    {
    case 0b00000:
        mfc();
        log_warning("Unhandled COP0 instruction mfc\n");
        break;

    case 0b00010:
        cfc();
        log_warning("Unhandled COP0 instruction cfc\n");
        break;

    case 0b00100:
        mtc();
        break;

    case 0b00110:
        ctc();
        break;

    case 0b01000:
        log_warning("Unhandled COP0 instruction bcnf/bcnt\n");
        break;

    default:
        log_error("Unhandled COP0 instruction with opcode %x\n", current_opcode);
        break;
    }
}

void handle_cop1_instruction()
{
    log_warning("Unhandled COP1 instruction\n");
}

void handle_cop2_instruction()
{
    log_warning("Unhandled COP2 instruction\n");
}

void handle_cop3_instruction()
{
    log_warning("Unhandled COP3 instruction\n");
}
