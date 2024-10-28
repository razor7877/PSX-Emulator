#include <stdint.h>

#include "coprocessor.h"
#include "cpu.h"
#include "logging.h"
#include "debug.h"

uint32_t _cop0_registers[64] = { 0 };

void mfc()
{
    uint32_t source_reg = rd(cpu_state.current_opcode);

    if (source_reg > 31)
        log_error("MFC0 attempted to move from register outside of data registers\n");
    else
        R(rt(cpu_state.current_opcode)) = CPR0(source_reg);
}

void cfc()
{
    uint32_t source_reg = rd(cpu_state.current_opcode);

    if (source_reg < 32 || source_reg > 63)
        log_error("CFC0 attempted to move from register outside of data registers\n");
    else
        R(rt(cpu_state.current_opcode)) = CPR0(source_reg);
}

void mtc()
{
    uint32_t dest_reg = rd(cpu_state.current_opcode);

    if (dest_reg > 31)
        log_error("MTC0 attempted to move to register outside of data registers\n");
    else
        CPR0(dest_reg) = R(rt(cpu_state.current_opcode));
}

void ctc()
{
    uint32_t dest_reg = rd(cpu_state.current_opcode);

    if (dest_reg < 32 || dest_reg > 63)
        log_error("CTC0 attempted to move to register outside of control registers\n");
    else
        CPR0(dest_reg) = R(rt(cpu_state.current_opcode));
}

void handle_cop0_instruction()
{
    uint8_t opcode = cop0_code(cpu_state.current_opcode);

    switch (opcode)
    {
    case 0b00000:
        mfc();
        break;

    case 0b00010:
        cfc();
        break;

    case 0b00100:
        mtc();
        break;

    case 0b00110:
        ctc();
        break;

    case 0b01000:
        debug_state.in_debug = true;
        log_warning("Unhandled COP0 instruction bcnf/bcnt\n");
        break;

    default:
        debug_state.in_debug = true;
        log_error("Unhandled COP0 instruction with opcode %x\n", cpu_state.current_opcode);
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
