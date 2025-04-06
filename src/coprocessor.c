#include <stdint.h>
#include <string.h>

#include "coprocessor.h"
#include "cpu.h"
#include "logging.h"
#include "debug.h"

uint32_t _cop0_registers[64] = { 0 };

void reset_cop0_state()
{
    memset(_cop0_registers, 0, sizeof(_cop0_registers));
}

static void mfc()
{
    uint32_t source_reg = rd(cpu_state.current_opcode);

    if (source_reg > 31)
        log_error("MFC0 attempted to move from register outside of data registers\n");
    else
        R(rt(cpu_state.current_opcode)) = CPR0(source_reg);
}

static void cfc()
{
    uint32_t source_reg = rd(cpu_state.current_opcode);

    if (source_reg < 32 || source_reg > 63)
        log_error("CFC0 attempted to move from register outside of data registers\n");
    else
        R(rt(cpu_state.current_opcode)) = CPR0(source_reg);
}

static void mtc()
{
    uint32_t dest_reg = rd(cpu_state.current_opcode);

    if (dest_reg == 13)
    {
        // Clear out bits 8-9
        CPR0(13) &= ~(0b11 << 8);
        // Set bits 8-9 from write value
        CPR0(13) |= R(rt(cpu_state.current_opcode)) & (0b11 << 8);

        return;
    }

    if (dest_reg > 31)
        log_error("MTC0 attempted to move to register outside of data registers\n");
    else
        CPR0(dest_reg) = R(rt(cpu_state.current_opcode));
}

static void ctc()
{
    uint32_t dest_reg = rd(cpu_state.current_opcode);

    if (dest_reg < 32 || dest_reg > 63)
        log_error("CTC0 attempted to move to register outside of control registers\n");
    else
        CPR0(dest_reg) = R(rt(cpu_state.current_opcode));
}

static void rfe()
{
    // Old interrupt enable & kernel/user mode bits
    uint8_t old = (SR & 0b110000) >> 4;
    // Previous interrupt enable & kernel/user mode bits
    uint8_t previous = (SR & 0b001100) >> 2;

    // Clear previous & current interrupt enable & kernel/user mode bits
    SR &= ~0b1111;
    // Old into previous
    SR |= old << 2;
    // Previous into current
    SR |= previous;
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

        case 0b10000:
            log_debug("Return from exception! --- PC is %x --- Return address at %x\n", cpu_state.pc, R31);
            rfe();
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

void handle_mem_exception(ExceptionType exception, uint32_t address)
{
    BAD_VADDR = address;
    handle_exception(exception);
}

void handle_exception(ExceptionType exception)
{
    //log_debug("Handling exception at PC %x\n", cpu_state.pc);

    // Clear exception code bits
    CAUSE &= ~(0b11111 << 2);
    // Set the current exception code
    CAUSE |= (exception << 2);

    if (cpu_state.delay_jump)
    {
        EPC = cpu_state.pc - 8;
        CAUSE |= 0x80000000;
    }
    else
        EPC = cpu_state.pc - 4;

    // Boot exception vectors in RAM/ROM
    bool bev = SR & (1 << 22);
    bool is_cop0_break = false;

    switch (exception)
    {
        case INTERRUPT: // Interrupt
            break;

        case ADEL: // Address error - Data load/Instruction fetch
            break;

        case ADES: // Address error - Data store
            break;

        case IBE: // Bus error on instruction fetch
            log_error("Unhandled IBE exception!\n");
            break;

        case DBE: // Bus error on data load/store
            log_error("Unhandled DBE exception!\n");
            break;

        case SYSCALL: // System call
            break;

        case BP: // Breakpoint
            is_cop0_break = true;
            log_error("Unhandled breakpoint exception!\n");
            break;

        case RI: // Reserved instruction
            log_error("Unhandled reserved instruction exception!\n");
            break;

        case CPU: // Coprocessor unusable
            log_error("Unhandled coprocessor unusable exception!\n");
            break;

        case OVERFLOW: // Arithmetic overflow
            break;

        default:
            log_error("Unhandled exception!\n");
            break;
    }

    // Jump to the corresponding exception vector
    if (is_cop0_break)
    {
        if (bev)
            cpu_state.pc = 0xBFC00140;
        else
            cpu_state.pc = 0x80000040;
    }
    else
    {
        if (bev)
            cpu_state.pc = 0xBFC00180;
        else
            cpu_state.pc = 0x80000080;
    }
}
