#include <stdbool.h>

#include "cpu.h"
#include "memory.h"
#include "logging.h"
#include "coprocessor.h"

cpu cpu_state = {
    .registers = {0},
    .current_opcode = 0x00,
    .pc = 0xBFC00000,
    .hi =  0,
    .lo =  0,
    .delay_jump = false,
    .jmp_address = 0x00
};

const instruction primary_opcodes[0x40] = {
    { "SPECIAL", NULL },       // 00h
    { "BCONDZ", b_cond_z },       // 01h
    { "J", j },                   // 02h
    { "JAL", jal },               // 03h
    { "BEQ", beq },               // 04h
    { "BNE", bne },               // 05h
    { "BLEZ", blez },             // 06h
    { "BGTZ", bgtz },             // 07h
    { "ADDI", addi },             // 08h
    { "ADDIU", addiu },           // 09h
    { "SLTI", slti },             // 0Ah
    { "SLTIU", sltiu },           // 0Bh
    { "ANDI", andi },             // 0Ch
    { "ORI", ori },               // 0Dh
    { "XORI", xori },             // 0Eh
    { "LUI", lui },               // 0Fh
    { "COP0", handle_cop0_instruction },             // 10h
    { "COP1", handle_cop1_instruction },             // 11h
    { "COP2", handle_cop2_instruction },             // 12h
    { "COP3", handle_cop3_instruction },             // 13h
    { "N/A", undefined },         // 14h
    { "N/A", undefined },         // 15h
    { "N/A", undefined },         // 16h
    { "N/A", undefined },         // 17h
    { "N/A", undefined },         // 18h
    { "N/A", undefined },         // 19h
    { "N/A", undefined },         // 1Ah
    { "N/A", undefined },         // 1Bh
    { "N/A", undefined },         // 1Ch
    { "N/A", undefined },         // 1Dh
    { "N/A", undefined },         // 1Eh
    { "N/A", undefined },         // 1Fh
    { "LB", lb },                 // 20h
    { "LH", lh },                 // 21h
    { "LWL", lwl },               // 22h
    { "LW", lw },                 // 23h
    { "LBU", lbu },               // 24h
    { "LHU", lhu },               // 25h
    { "LWR", lwr },               // 26h
    { "N/A", undefined },         // 27h
    { "SB", sb },                 // 28h
    { "SH", sh },                 // 29h
    { "SWL", swl },               // 2Ah
    { "SW", sw },                 // 2Bh
    { "N/A", undefined },         // 2Ch
    { "N/A", undefined },         // 2Dh
    { "SWR", swr },               // 2Eh
    { "N/A", undefined },         // 2Fh
    { "LWC0", lwc0 },             // 30h
    { "LWC1", lwc1 },             // 31h
    { "LWC2", lwc2 },             // 32h
    { "LWC3", lwc3 },             // 33h
    { "N/A", undefined },         // 34h
    { "N/A", undefined },         // 35h
    { "N/A", undefined },         // 36h
    { "N/A", undefined },         // 37h
    { "SWC0", swc0 },             // 38h
    { "SWC1", swc1 },             // 39h
    { "SWC2", swc2 },             // 3Ah
    { "SWC3", swc3 },             // 3Bh
    { "N/A", undefined },         // 3Ch
    { "N/A", undefined },         // 3Dh
    { "N/A", undefined },         // 3Eh
    { "N/A", undefined }          // 3Fh
};

const instruction secondary_opcodes[0x40] = {
    { "SLL", sll },               // 00h
    { "N/A", undefined },         // 01h
    { "SRL", srl },               // 02h
    { "SRA", sra },               // 03h
    { "SLLV", sllv },             // 04h
    { "N/A", undefined },         // 05h
    { "SRLV", srlv },             // 06h
    { "SRAV", srav },             // 07h
    { "JR", jr },                 // 08h
    { "JALR", jalr },             // 09h
    { "N/A", undefined },         // 0Ah
    { "N/A", undefined },         // 0Bh
    { "SYSCALL", syscall },       // 0Ch
    { "BREAK", op_break },        // 0Dh
    { "N/A", undefined },         // 0Eh
    { "N/A", undefined },         // 0Fh
    { "MFHI", mfhi },             // 10h
    { "MTHI", mthi },             // 11h
    { "MFLO", mflo },             // 12h
    { "MTLO", mtlo },             // 13h
    { "N/A", undefined },         // 14h
    { "N/A", undefined },         // 15h
    { "N/A", undefined },         // 16h
    { "N/A", undefined },         // 17h
    { "MULT", mult },             // 18h
    { "MULTU", multu },           // 19h
    { "DIV", op_div },               // 1Ah
    { "DIVU", divu },             // 1Bh
    { "N/A", undefined },         // 1Ch
    { "N/A", undefined },         // 1Dh
    { "N/A", undefined },         // 1Eh
    { "N/A", undefined },         // 1Fh
    { "ADD", add },               // 20h
    { "ADDU", addu },             // 21h
    { "SUB", sub },               // 22h
    { "SUBU", subu },             // 23h
    { "AND", and },               // 24h
    { "OR", op_or },                 // 25h
    { "XOR", op_xor },               // 26h
    { "NOR", nor },               // 27h
    { "N/A", undefined },         // 28h
    { "N/A", undefined },         // 29h
    { "SLT", slt },               // 2Ah
    { "SLTU", sltu },             // 2Bh
    { "N/A", undefined },         // 2Ch
    { "N/A", undefined },         // 2Dh
    { "N/A", undefined },         // 2Eh
    { "N/A", undefined },         // 2Fh
    { "N/A", undefined },         // 30h
    { "N/A", undefined },         // 31h
    { "N/A", undefined },         // 32h
    { "N/A", undefined },         // 33h
    { "N/A", undefined },         // 34h
    { "N/A", undefined },         // 35h
    { "N/A", undefined },         // 36h
    { "N/A", undefined },         // 37h
    { "N/A", undefined },         // 38h
    { "N/A", undefined },         // 39h
    { "N/A", undefined },         // 3Ah
    { "N/A", undefined },         // 3Bh
    { "N/A", undefined },         // 3Ch
    { "N/A", undefined },         // 3Dh
    { "N/A", undefined },         // 3Eh
    { "N/A", undefined }          // 3Fh
};

void reset_cpu_state()
{
    for (int i = 0; i < 32; i++)
        cpu_state.registers[i] = 0;

    cpu_state.pc = 0xBFC00000;
    cpu_state.hi =  0;
    cpu_state.lo =  0;
    cpu_state.current_opcode = 0;
    cpu_state.delay_jump = false;
    cpu_state.jmp_address = 0;
}

void undefined()
{
    log_error("Attempted to execute undefined opcode!\n");
}

void b_cond_z()
{
    int32_t rs_val = (int32_t)R(rs(cpu_state.current_opcode));

    R31 = cpu_state.pc + 0x4;

    if (rs_val > 0)
    {
        uint16_t offset_16 = cpu_state.current_opcode & 0xFFFF;
        int32_t offset_18 = (int32_t)(offset_16 << 2);

        cpu_state.pc = cpu_state.pc + offset_18;
    }
}

void j()
{
    // Get 28 lower address bits from 26 lowest bits of the opcode
    uint32_t instr_index = (cpu_state.current_opcode & 0x03FFFFFF) << 2;
    uint32_t upper_bits = (cpu_state.pc + 0x4) & (0xF0000000);

    cpu_state.jmp_address = upper_bits | instr_index;
    cpu_state.delay_jump = true;
}

void jal()
{
    // Save return address in R31
    R31 = cpu_state.pc + 0x4;

    // Get 28 lower address bits from 26 lowest bits of the opcode
    uint32_t instr_index = (cpu_state.current_opcode & 0x03FFFFFF) << 2;
    uint32_t upper_bits = (cpu_state.pc + 0x4) & (0xF0000000);

    cpu_state.jmp_address = upper_bits | instr_index;
    cpu_state.delay_jump = true;
}

void beq()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    uint16_t offset_16 = cpu_state.current_opcode & 0xFFFF;
    int32_t offset_18 = (int32_t)(offset_16 << 2);

    if (rs_val == rt_val)
    {
        cpu_state.delay_jump = true;
        cpu_state.jmp_address = cpu_state.pc + offset_18;
    }
}

void bne()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    int16_t offset_16 = cpu_state.current_opcode & 0xFFFF;
    int32_t offset_18 = (int32_t)(offset_16 << 2);

    if (rs_val != rt_val)
    {
        cpu_state.delay_jump = true;
        cpu_state.jmp_address = cpu_state.pc + offset_18;
    }
}

void blez()
{
    int32_t rs_val = (int32_t)R(rs(cpu_state.current_opcode));

    uint16_t offset_16 = cpu_state.current_opcode & 0xFFFF;
    int32_t offset_18 = (int32_t)(offset_16 << 2);

    if (rs_val <= 0)
    {
        cpu_state.delay_jump = true;
        cpu_state.jmp_address = cpu_state.pc + offset_18;
    }
}

void bgtz()
{
    int32_t rs_val = R(rs(cpu_state.current_opcode));

    uint16_t offset_16 = cpu_state.current_opcode & 0xFFFF;
    int32_t offset_18 = (int32_t)(offset_16 << 2);

    if (rs_val > 0)
    {
        cpu_state.delay_jump = true;
        cpu_state.jmp_address = cpu_state.pc + offset_18;
    }
}

void addi()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));

    int16_t signed_add = (int16_t)(cpu_state.current_opcode & 0xFFFF);
    int64_t sum = (uint64_t)(rs_val + signed_add);

    if (sum > 0xFFFFFFFF)
        log_warning("ADDI instruction resulted in overflow!\n");
    else if (signed_add < 0 && sum < signed_add)
        log_warning("ADDI instruction resulted in underflow!\n");
    else
        R(rt(cpu_state.current_opcode)) = sum;
}

void addiu()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));

    int16_t signed_add = (int16_t)(cpu_state.current_opcode & 0xFFFF);

    uint64_t sum = (uint64_t)(rs_val + signed_add);
    R(rt(cpu_state.current_opcode)) = sum;
}

void slti()
{
    int32_t rs_val = (int32_t)R(rs(cpu_state.current_opcode));
    int16_t immediate = (int16_t)(cpu_state.current_opcode & 0xFFFF);

    R(rt(cpu_state.current_opcode)) = rs_val < immediate;
}

void sltiu()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    // Sign extend the immediate but use it as an unsigned value
    uint32_t immediate = (int32_t)(cpu_state.current_opcode & 0xFFFF);

    R(rt(cpu_state.current_opcode)) = rs_val < immediate;
}

void andi()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    // Get immediate from 16 lowest bits
    uint16_t imm = cpu_state.current_opcode & 0xFFFF;

    R(rt(cpu_state.current_opcode)) = rs_val & imm;
}

void ori()
{
    uint16_t immediate = cpu_state.current_opcode & 0xFFFF;
    uint32_t rs_val = R(rs(cpu_state.current_opcode));

    uint32_t result = rs_val | immediate;
    R(rt(cpu_state.current_opcode)) = result;
}

void xori()
{
    uint16_t immediate = cpu_state.current_opcode & 0xFFFF;
    uint32_t rs_val = R(rs(cpu_state.current_opcode));

    uint32_t result = rs_val ^ immediate;
    R(rt(cpu_state.current_opcode)) = result;
}

void lui()
{
    uint16_t immediate = cpu_state.current_opcode & 0xFFFF;
    uint32_t upper_value = immediate << 16;

    R(rt(cpu_state.current_opcode)) = upper_value;
}

void lb()
{
    // Base register
    uint32_t base_addr = R(rs(cpu_state.current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(cpu_state.current_opcode & 0x0000FFFF);

    int address = base_addr + offset;

    // Get the word that contains the byte
    uint32_t word = (uint32_t)read_word(address);
    uint8_t byte_index = address % 4;

    // First byte in first 8 bits, second in the next 8 and so on
    uint32_t mask = 0xFF000000 >> (byte_index * 8);
    // Shift the value to bring it to an 8 bit value
    int8_t byte = (word & mask) >> (24 - byte_index * 8);

    int32_t sign_extended = (int32_t)byte;
    R(rt(cpu_state.current_opcode)) = sign_extended;
}

void lh()
{
    // Base register
    uint32_t base_addr = R(rs(cpu_state.current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(cpu_state.current_opcode & 0x0000FFFF);

    int address = base_addr + offset;

    // Get the word that contains the byte
    uint32_t word = (uint32_t)read_word(address);
    uint16_t byte_index = address % 2;

    // First byte in first 8 bits, second in the next 8 and so on
    uint32_t mask = 0xFFFF0000 >> (byte_index * 16);
    // Shift the value to bring it to an 8 bit value
    int16_t half_word = (word & mask) >> (16 - byte_index * 16);

    int32_t sign_extended = (int32_t)half_word;
    R(rt(cpu_state.current_opcode)) = sign_extended;
}

void lwl()
{
    log_warning("Unhandled instruction lwl\n");
}

void lw()
{
    // Base register
    uint32_t base_addr = R(rs(cpu_state.current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(cpu_state.current_opcode & 0x0000FFFF);

    uint32_t address = base_addr + offset;

    // Get the word
    uint32_t word = (uint32_t)read_word(address);

    R(rt(cpu_state.current_opcode)) = word;
}

void lbu()
{
    // Base register
    uint32_t base_addr = R(rs(cpu_state.current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(cpu_state.current_opcode & 0x0000FFFF);

    int address = base_addr + offset;

    // Get the word that contains the byte
    uint32_t word = (uint32_t)read_word(address);
    uint8_t byte_index = address % 4;

    // First byte in first 8 bits, second in the next 8 and so on
    uint32_t mask = 0xFF000000 >> (byte_index * 8);
    // Shift the value to bring it to an 8 bit value
    uint8_t byte = (word & mask) >> (24 - byte_index * 8);

    R(rt(cpu_state.current_opcode)) = byte;
}

void lhu()
{
    // Base register
    uint32_t base_addr = R(rs(cpu_state.current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(cpu_state.current_opcode & 0x0000FFFF);

    int address = base_addr + offset;

    // Get the word that contains the byte
    uint32_t word = (uint32_t)read_word(address);
    uint16_t byte_index = address % 2;

    // First byte in first 8 bits, second in the next 8 and so on
    uint32_t mask = 0xFFFF0000 >> (byte_index * 16);
    // Shift the value to bring it to an 8 bit value
    uint16_t half_word = (word & mask) >> (16 - byte_index * 16);

    R(rt(cpu_state.current_opcode)) = half_word;
}

void lwr()
{
    log_warning("Unhandled instruction lwr\n");
}

void sb()
{
    // Base register
    uint32_t base_addr = R(rs(cpu_state.current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(cpu_state.current_opcode & 0x0000FFFF);

    int address = base_addr + offset;

    uint8_t value = R(rt(cpu_state.current_opcode)) & 0xFF;

    write_word(address, value);
}

void sh()
{
    // Base register
    uint32_t base_addr = R(rs(cpu_state.current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(cpu_state.current_opcode & 0x0000FFFF);

    int address = base_addr + offset;

    uint8_t value = R(rt(cpu_state.current_opcode)) & 0xFFFF;

    if ((address & 0b1) != 0)
        log_error("Unaligned address exception with sh instruction!\n");
    else
        write_word(address, value);
}

void swl()
{
    log_warning("Unhandled instruction swl\n");
}

void sw()
{
    uint32_t base_addr = R(rs(cpu_state.current_opcode));
    uint32_t rt_val = R(rt(cpu_state.current_opcode));
    
    int16_t offset = (int16_t)(cpu_state.current_opcode & 0xFFFF);
    uint32_t address = base_addr + offset;

    if ((address & 0b11) != 0)
        log_error("Unaligned address exception with sw instruction!\n");
    else
        write_word(address, rt_val);

    log_debug("Store word %x at address %x\n", rt_val, address);
}

void swr()
{
    log_warning("Unhandled instruction swr\n");
}

void lwc0()
{
    log_warning("Unhandled instruction LWC0\n");
}

void lwc1()
{
    log_warning("Unhandled instruction LWC1\n");
}

void lwc2()
{
    log_warning("Unhandled instruction LWC2\n");
}

void lwc3()
{
    log_warning("Unhandled instruction LWC3\n");
}

void swc0()
{
    log_warning("Unhandled instruction SWC0\n");
}

void swc1()
{
    log_warning("Unhandled instruction SWC1\n");
}

void swc2()
{
    log_warning("Unhandled instruction SWC2\n");
}

void swc3()
{
    log_warning("Unhandled instruction SWC3\n");
}

void sll()
{
    uint8_t immediate = imm5(cpu_state.current_opcode);
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    R(rd(cpu_state.current_opcode)) = rt_val << immediate;
}

void srl()
{
    uint8_t immediate = imm5(cpu_state.current_opcode);
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    R(rd(cpu_state.current_opcode)) = rt_val >> immediate;
}

void sra()
{
    uint8_t immediate = imm5(cpu_state.current_opcode);
    int32_t rt_val = (int32_t)R(rt(cpu_state.current_opcode));

    // TODO : Make sure this does an arithmetic shift
    R(rd(cpu_state.current_opcode)) = rt_val >> immediate;
}

void sllv()
{
    uint8_t rs_val_5 = R(rs(cpu_state.current_opcode)) & 0b11111;
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    R(rd(cpu_state.current_opcode)) = rt_val << rs_val_5;
}

void srlv()
{
    uint8_t rs_val_5 = R(rs(cpu_state.current_opcode)) & 0b11111;
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    R(rd(cpu_state.current_opcode)) = rt_val >> rs_val_5;
}

void srav()
{
    uint8_t rs_val_5 = R(rs(cpu_state.current_opcode)) & 0b11111;
    int32_t rt_val = (int32_t)R(rt(cpu_state.current_opcode));

    // TODO : Make sure this does an arithmetic shift
    R(rd(cpu_state.current_opcode)) = rt_val >> rs_val_5;
}

void jr()
{
    cpu_state.jmp_address = R(rs(cpu_state.current_opcode));
    cpu_state.delay_jump = true;
}

void jalr()
{
    // Save return address in R31
    R(rd(cpu_state.current_opcode)) = cpu_state.pc + 0x4;

    cpu_state.jmp_address = R(rs(cpu_state.current_opcode));
    cpu_state.delay_jump = true;
}

void syscall()
{
    log_warning("Unhandled syscall instruction!\n");
}

void op_break()
{
    log_warning("Unhandled break instruction!\n");
}

void mfhi()
{
    R(rd(cpu_state.current_opcode)) = cpu_state.hi;
}

void mthi()
{
    cpu_state.hi =  R(rs(cpu_state.current_opcode));
}

void mflo()
{
    R(rd(cpu_state.current_opcode)) = cpu_state.lo;
}

void mtlo()
{
    cpu_state.lo =  R(rs(cpu_state.current_opcode));
}

void mult()
{
    int32_t rs_val = (int32_t)R(rs(cpu_state.current_opcode));
    int32_t rt_val = (int32_t)R(rt(cpu_state.current_opcode));

    int64_t mult_result = rs_val * rt_val;
    cpu_state.hi =  (mult_result & 0xFFFFFFFF00000000) >> 32;
    cpu_state.lo =  mult_result & 0xFFFFFFFF;
}

void multu()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    uint64_t mult_result = rs_val * rt_val;
    cpu_state.hi =  (mult_result & 0xFFFFFFFF00000000) >> 32;
    cpu_state.lo =  mult_result & 0xFFFFFFFF;
}

void op_div()
{
    int32_t rs_val = (int32_t)R(rs(cpu_state.current_opcode));
    int32_t rt_val = (int32_t)R(rs(cpu_state.current_opcode));

    if (rt_val == 0)
        log_error("DIV instruction attempted divide by zero!\n");

    uint32_t quotient = rs_val / rt_val;
    uint32_t remainder = rs_val % rt_val;

    cpu_state.hi =  remainder;
    cpu_state.lo =  quotient;
}

void divu()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    uint32_t rt_val = R(rs(cpu_state.current_opcode));

    if (rt_val == 0)
        log_error("DIV instruction attempted divide by zero!\n");

    uint32_t quotient = rs_val / rt_val;
    uint32_t remainder = rs_val % rt_val;

    cpu_state.hi =  remainder;
    cpu_state.lo =  quotient;
}

void add()
{
    uint64_t sum = R(rs(cpu_state.current_opcode)) + R(rt(cpu_state.current_opcode));

    if (sum > 0xFFFFFFFF)
        log_warning("ADD instruction resulted in overflow!\n");
    else
        R(rd(cpu_state.current_opcode)) = sum;
}

void addu()
{
    uint8_t rs_val = R(rs(cpu_state.current_opcode));

    int16_t signed_add = (int16_t)(cpu_state.current_opcode & 0xFFFF);
    uint64_t sum = (uint64_t)(rs_val + signed_add);

    R(rd(cpu_state.current_opcode)) = sum;
}

void sub()
{
    int32_t rs_val = (int32_t)R(rs(cpu_state.current_opcode));
    int32_t rt_val = (int32_t)R(rt(cpu_state.current_opcode));

    int64_t sub_result = rs_val - rt_val;

    if (sub_result > 0xFFFFFFFF)
        log_error("SUB instruction caused overflow!\n");
    else
        R(rd(cpu_state.current_opcode)) = sub_result & 0xFFFFFFFF;
}

void subu()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    uint64_t sub_result = rs_val - rt_val;
    R(rd(cpu_state.current_opcode)) = sub_result & 0xFFFFFFFF;
}

void and()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    R(rd(cpu_state.current_opcode)) = rs_val & rt_val;
}

void op_or()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    uint32_t result = rs_val | rt_val;
    R(rd(cpu_state.current_opcode)) = result;
}

void op_xor()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    uint32_t result = rs_val ^ rt_val;
    R(rt(cpu_state.current_opcode)) = result;
}

void nor()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    uint32_t rt_val = R(rt(cpu_state.current_opcode));

    uint32_t nor_result = ~(rs_val | rt_val);
    R(rd(cpu_state.current_opcode)) = nor_result;
}

void slt()
{
    int32_t rs_val = (int32_t)R(rs(cpu_state.current_opcode));
    int32_t rt_val = (int32_t)R(rt(cpu_state.current_opcode));

    R(rt(cpu_state.current_opcode)) = rs_val < rt_val;
}

void sltu()
{
    uint32_t rs_val = R(rs(cpu_state.current_opcode));
    uint32_t rt_val = R(rs(cpu_state.current_opcode));

    R(rd(cpu_state.current_opcode)) = rs_val < rt_val;
}

static void print_debug_info(uint8_t primary_opcode, uint8_t secondary_opcode)
{
    char* disassembly = primary_opcodes[primary_opcode].disassembly;
    if (primary_opcode == 0x00)
        disassembly = secondary_opcodes[secondary_opcode].disassembly;

    log_info("Executing opcode %s - %x\n\tAddress %x\n\tRS(r%d): %x RT(r%d): %x RD(r%d): %x\n",
        disassembly, cpu_state.current_opcode, cpu_state.pc,
        rs(cpu_state.current_opcode), R(rs(cpu_state.current_opcode)),
        rt(cpu_state.current_opcode), R(rt(cpu_state.current_opcode)),
        rd(cpu_state.current_opcode), R(rd(cpu_state.current_opcode))
    );

    log_warning("Return address r31 %x\n", R31);
}

void handle_instruction()
{
    R0 = 0;
    // Decode next instruction
    cpu_state.current_opcode = read_word(cpu_state.pc);

    // Get primary opcode from 6 highest bits
    uint8_t primary_opcode = (cpu_state.current_opcode & 0xFC000000) >> 26;
    // Get secondary opcode from 6 lowest bits
    uint8_t secondary_opcode = cpu_state.current_opcode & 0x3F;

    if (cpu_state.pc > 0xBFC07000)
        print_debug_info(primary_opcode, secondary_opcode);

    // Jump if we have a jump in delay slot, otherwise increment pc normally
    if (cpu_state.delay_jump)
    {
        cpu_state.delay_jump = false;
        cpu_state.pc = cpu_state.jmp_address;
        log_debug("JUMPED at %x\n", cpu_state.jmp_address);
    }
    else
        cpu_state.pc += 0x4;

    if (primary_opcode == 0x00)
        ((void (*)(void))secondary_opcodes[secondary_opcode].function)();
    else
        ((void (*)(void))primary_opcodes[primary_opcode].function)();
}
