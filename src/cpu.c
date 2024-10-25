#include <stdbool.h>

#include "cpu.h"
#include "memory.h"
#include "logging.h"
#include "coprocessor.h"

#define R0 _registers[0]
#define R1 _registers[1]
#define R2 _registers[2]
#define R3 _registers[3]
#define R4 _registers[4]
#define R5 _registers[5]
#define R6 _registers[6]
#define R7 _registers[7]
#define R8 _registers[8]
#define R9 _registers[9]
#define R10 _registers[10]
#define R11 _registers[11]
#define R12 _registers[12]
#define R13 _registers[13]
#define R14 _registers[14]
#define R15 _registers[15]
#define R16 _registers[16]
#define R17 _registers[17]
#define R18 _registers[18]
#define R19 _registers[19]
#define R20 _registers[20]
#define R21 _registers[21]
#define R22 _registers[22]
#define R23 _registers[23]
#define R24 _registers[24]
#define R25 _registers[25]
#define R26 _registers[26]
#define R27 _registers[27]
#define R28 _registers[28]
#define R29 _registers[29]
#define R30 _registers[30]
#define R31 _registers[31]

// Initialize registers
uint32_t _registers[32] = { 0 };

/// <summary>
/// Program counter
/// </summary>
uint32_t pc = 0xBFC00000;

/// <summary>
/// Multiply/divide high result
/// </summary>
uint32_t hi = 0;

/// <summary>
/// Multiply/divde low result
/// </summary>
uint32_t lo = 0;

/// <summary>
/// The 32 bit value of the currently executed opcode
/// </summary>
uint32_t current_opcode = 0;

/// <summary>
/// Whether a jump should be executed on the next instruction
/// </summary>
bool jmp_next_instr = false;

/// <summary>
/// The address to jump to
/// </summary>
uint32_t jmp_address = 0x00;

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
    { "DIV", div },               // 1Ah
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

void undefined()
{
    log_error("Attempted to execute undefined opcode!\n");
}

void b_cond_z()
{
    int32_t rs_val = (int32_t)R(rs(current_opcode));

    R31 = pc + 0x8;

    if (rs_val > 0)
    {
        uint16_t offset_16 = current_opcode & 0xFFFF;
        int32_t offset_18 = (int32_t)(offset_16 << 2);

        pc = pc + offset_18;
    }
}

void j()
{
    // Get 28 lower address bits from 26 lowest bits of the opcode
    uint32_t instr_index = (current_opcode & 0x03FFFFFF) << 2;
    uint32_t upper_bits = (pc + 0x4) & (0xF0000000);

    jmp_address = upper_bits | instr_index;
    jmp_next_instr = true;
}

void jal()
{
    // Save return address in R31
    R31 = pc + 0x8;

    // Get 28 lower address bits from 26 lowest bits of the opcode
    uint32_t instr_index = (current_opcode & 0x03FFFFFF) << 2;
    uint32_t upper_bits = (pc + 0x4) & (0xF0000000);

    jmp_address = upper_bits | instr_index;
    jmp_next_instr = true;
}

void beq()
{
    uint32_t rs_val = R(rs(current_opcode));
    uint32_t rt_val = R(rt(current_opcode));

    uint16_t offset_16 = current_opcode & 0xFFFF;
    int32_t offset_18 = (int32_t)(offset_16 << 2);

    if (rs_val == rt_val)
        pc = pc + offset_18;
}

void bne()
{
    uint32_t rs_val = R(rs(current_opcode));
    uint32_t rt_val = R(rt(current_opcode));

    uint16_t offset_16 = current_opcode & 0xFFFF;
    int32_t offset_18 = (int32_t)(offset_16 << 2);

    if (rs_val != rt_val)
        pc = pc + offset_18;
}

void blez()
{
    int32_t rs_val = (int32_t)R(rs(current_opcode));

    uint16_t offset_16 = current_opcode & 0xFFFF;
    int32_t offset_18 = (int32_t)(offset_16 << 2);

    if (rs_val <= 0)
        pc = pc + offset_18;
}

void bgtz()
{
    int32_t rs_val = R(rs(current_opcode));

    uint16_t offset_16 = current_opcode & 0xFFFF;
    int32_t offset_18 = (int32_t)(offset_16 << 2);

    if (rs_val > 0)
        pc = pc + offset_18;
}

void addi()
{
    uint32_t rs_val = R(rs(current_opcode));

    int16_t signed_add = (int16_t)(current_opcode & 0xFFFF);
    uint64_t sum = (uint64_t)(rs_val + signed_add);

    if (sum > 0xFFFFFFFF)
        log_warning("ADDI instruction resulted in overflow!\n");
    else if (signed_add < 0 && sum < signed_add)
        log_warning("ADDI instruction resulted in underflow!\n");
    else
        R(rd(current_opcode)) = sum;
}

void addiu()
{
    uint32_t rs_val = R(rs(current_opcode));
    uint32_t rt_val = R(rt(current_opcode));

    uint64_t sum = (uint64_t)(rs_val + rt_val);
    R(rd(current_opcode)) = sum;
}

void slti()
{
    int32_t rs_val = (int32_t)R(rs(current_opcode));
    int16_t immediate = (int16_t)(current_opcode & 0xFFFF);

    R(rt(current_opcode)) = rs_val < immediate;
}

void sltiu()
{
    uint32_t rs_val = R(rs(current_opcode));
    // Sign extend the immediate but use it as an unsigned value
    uint32_t immediate = (int32_t)(current_opcode & 0xFFFF);

    R(rt(current_opcode)) = rs_val < immediate;
}

void andi()
{
    uint32_t rs_val = R(rs(current_opcode));
    // Get immediate from 16 lowest bits
    uint16_t imm = current_opcode & 0xFFFF;

    R(rt(current_opcode)) = rs_val & imm;
}

void ori()
{
    uint16_t immediate = current_opcode & 0xFFFF;
    uint32_t rs_val = R(rs(current_opcode));

    uint32_t result = rs_val | immediate;
    R(rt(current_opcode)) = result;
}

void xori()
{
    uint16_t immediate = current_opcode & 0xFFFF;
    uint32_t rs_val = R(rs(current_opcode));

    uint32_t result = rs_val ^ immediate;
    R(rt(current_opcode)) = result;
}

void lui()
{
    uint16_t immediate = current_opcode & 0xFFFF;
    uint32_t upper_value = immediate << 16;

    R(rt(current_opcode)) = upper_value;
}

void lb()
{
    // Base register
    uint32_t base_addr = R(rs(current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(current_opcode & 0x0000FFFF);

    int address = base_addr + offset;

    // Get the word that contains the byte
    uint32_t word = (uint32_t)read_word(address / 4);
    uint8_t byte_index = address % 4;

    // First byte in first 8 bits, second in the next 8 and so on
    uint32_t mask = 0xFF000000 >> (byte_index * 8);
    // Shift the value to bring it to an 8 bit value
    int8_t byte = (word & mask) >> (24 - byte_index * 8);

    int32_t sign_extended = (int32_t)byte;
    R(rt(current_opcode)) = sign_extended;
}

void lh()
{
    // Base register
    uint32_t base_addr = R(rs(current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(current_opcode & 0x0000FFFF);

    int address = base_addr + offset;

    // Get the word that contains the byte
    uint32_t word = (uint32_t)read_word(address / 4);
    uint16_t byte_index = address % 2;

    // First byte in first 8 bits, second in the next 8 and so on
    uint32_t mask = 0xFFFF0000 >> (byte_index * 16);
    // Shift the value to bring it to an 8 bit value
    int16_t half_word = (word & mask) >> (16 - byte_index * 16);

    int32_t sign_extended = (int32_t)half_word;
    R(rt(current_opcode)) = sign_extended;
}

void lwl()
{
    log_warning("Unhandled instruction lwl\n");
}

void lw()
{
    // Base register
    uint32_t base_addr = R(rs(current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(current_opcode & 0x0000FFFF);

    int address = base_addr + offset;

    // Get the word
    uint32_t word = (uint32_t)read_word(address / 4);

    R(rt(current_opcode)) = word;
}

void lbu()
{
    // Base register
    uint32_t base_addr = R(rs(current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(current_opcode & 0x0000FFFF);

    int address = base_addr + offset;

    // Get the word that contains the byte
    uint32_t word = (uint32_t)read_word(address / 4);
    uint8_t byte_index = address % 4;

    // First byte in first 8 bits, second in the next 8 and so on
    uint32_t mask = 0xFF000000 >> (byte_index * 8);
    // Shift the value to bring it to an 8 bit value
    uint8_t byte = (word & mask) >> (24 - byte_index * 8);

    R(rt(current_opcode)) = byte;
}

void lhu()
{
    // Base register
    uint32_t base_addr = R(rs(current_opcode));

    // Get signed offset from 16 lower bits
    int16_t offset = (int16_t)(current_opcode & 0x0000FFFF);

    int address = base_addr + offset;

    // Get the word that contains the byte
    uint32_t word = (uint32_t)read_word(address / 4);
    uint16_t byte_index = address % 2;

    // First byte in first 8 bits, second in the next 8 and so on
    uint32_t mask = 0xFFFF0000 >> (byte_index * 16);
    // Shift the value to bring it to an 8 bit value
    uint16_t half_word = (word & mask) >> (16 - byte_index * 16);

    R(rt(current_opcode)) = half_word;
}

void lwr()
{
    log_warning("Unhandled instruction lwr\n");
}

void sb()
{
    log_warning("Unhandled instruction sb\n");
}

void sh()
{
    log_warning("Unhandled instruction sh\n");
}

void swl()
{
    log_warning("Unhandled instruction swl\n");
}

void sw()
{
    uint32_t base_addr = R(rs(current_opcode));
    uint32_t rt_val = R(rt(current_opcode));
    
    int16_t offset = (int16_t)(current_opcode & 0xFFFF);
    uint32_t address = base_addr + offset;

    if ((address & 0b11) != 0)
        log_error("Unaligned address exception with sw instruction!\n");
    else
        write_word(address, rt_val);
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
    uint8_t immediate = imm5(current_opcode);
    uint32_t rt_val = R(rt(current_opcode));

    R(rd(current_opcode)) = rt_val << immediate;
}

void srl()
{
    uint8_t immediate = imm5(current_opcode);
    uint32_t rt_val = R(rt(current_opcode));

    R(rd(current_opcode)) = rt_val >> immediate;
}

void sra()
{
    uint8_t immediate = imm5(current_opcode);
    int32_t rt_val = (int32_t)R(rt(current_opcode));

    // TODO : Make sure this does an arithmetic shift
    R(rd(current_opcode)) = rt_val >> immediate;
}

void sllv()
{
    uint8_t rs_val_5 = R(rs(current_opcode)) & 0b11111;
    uint32_t rt_val = R(rt(current_opcode));

    R(rd(current_opcode)) = rt_val << rs_val_5;
}

void srlv()
{
    uint8_t rs_val_5 = R(rs(current_opcode)) & 0b11111;
    uint32_t rt_val = R(rt(current_opcode));

    R(rd(current_opcode)) = rt_val >> rs_val_5;
}

void srav()
{
    uint8_t rs_val_5 = R(rs(current_opcode)) & 0b11111;
    int32_t rt_val = (int32_t)R(rt(current_opcode));

    // TODO : Make sure this does an arithmetic shift
    R(rd(current_opcode)) = rt_val >> rs_val_5;
}

void jr()
{
    jmp_address = R(rs(current_opcode));
    jmp_next_instr = true;
}

void jalr()
{
    // Save return address in R31
    R(rd(current_opcode)) = pc + 0x8;

    jmp_address = R(rs(current_opcode));
    jmp_next_instr = true;
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
    R(rd(current_opcode)) = hi;
}

void mthi()
{
    hi = R(rs(current_opcode));
}

void mflo()
{
    R(rd(current_opcode)) = lo;
}

void mtlo()
{
    lo = R(rs(current_opcode));
}

void mult()
{
    int32_t rs_val = (int32_t)R(rs(current_opcode));
    int32_t rt_val = (int32_t)R(rt(current_opcode));

    int64_t mult_result = rs_val * rt_val;
    hi = (mult_result & 0xFFFFFFFF00000000) >> 32;
    lo = mult_result & 0xFFFFFFFF;
}

void multu()
{
    uint32_t rs_val = R(rs(current_opcode));
    uint32_t rt_val = R(rt(current_opcode));

    uint64_t mult_result = rs_val * rt_val;
    hi = (mult_result & 0xFFFFFFFF00000000) >> 32;
    lo = mult_result & 0xFFFFFFFF;
}

void div()
{
    int32_t rs_val = (int32_t)R(rs(current_opcode));
    int32_t rt_val = (int32_t)R(rs(current_opcode));

    if (rt_val == 0)
        log_error("DIV instruction attempted divide by zero!\n");

    uint32_t quotient = rs_val / rt_val;
    uint32_t remainder = rs_val % rt_val;

    hi = remainder;
    lo = quotient;
}

void divu()
{
    uint32_t rs_val = R(rs(current_opcode));
    uint32_t rt_val = R(rs(current_opcode));

    if (rt_val == 0)
        log_error("DIV instruction attempted divide by zero!\n");

    uint32_t quotient = rs_val / rt_val;
    uint32_t remainder = rs_val % rt_val;

    hi = remainder;
    lo = quotient;
}

void add()
{
    uint64_t sum = R(rs(current_opcode)) + R(rt(current_opcode));

    if (sum > 0xFFFFFFFF)
        log_warning("ADD instruction resulted in overflow!\n");
    else
        R(rd(current_opcode)) = sum;
}

void addu()
{
    uint8_t rs_val = R(rs(current_opcode));

    int16_t signed_add = (int16_t)(current_opcode & 0xFFFF);
    uint64_t sum = (uint64_t)(rs_val + signed_add);

    R(rd(current_opcode)) = sum;
}

void sub()
{
    int32_t rs_val = (int32_t)R(rs(current_opcode));
    int32_t rt_val = (int32_t)R(rt(current_opcode));

    int64_t sub_result = rs_val - rt_val;

    if (sub_result > 0xFFFFFFFF)
        log_error("SUB instruction caused overflow!\n");
    else
        R(rd(current_opcode)) = sub_result & 0xFFFFFFFF;
}

void subu()
{
    uint32_t rs_val = R(rs(current_opcode));
    uint32_t rt_val = R(rt(current_opcode));

    uint64_t sub_result = rs_val - rt_val;
    R(rd(current_opcode)) = sub_result & 0xFFFFFFFF;
}

void and()
{
    uint32_t rs_val = R(rs(current_opcode));
    uint32_t rt_val = R(rt(current_opcode));

    R(rd(current_opcode)) = rs_val & rt_val;
}

void op_or()
{
    uint32_t rs_val = R(rs(current_opcode));
    uint32_t rt_val = R(rt(current_opcode));

    uint32_t result = rs_val | rt_val;
    R(rt(current_opcode)) = result;
}

void op_xor()
{
    uint32_t rs_val = R(rs(current_opcode));
    uint32_t rt_val = R(rt(current_opcode));

    uint32_t result = rs_val ^ rt_val;
    R(rt(current_opcode)) = result;
}

void nor()
{
    uint32_t rs_val = R(rs(current_opcode));
    uint32_t rt_val = R(rt(current_opcode));

    uint32_t nor_result = ~(rs_val | rt_val);
    R(rd(current_opcode)) = nor_result;
}

void slt()
{
    int32_t rs_val = (int32_t)R(rs(current_opcode));
    int32_t rt_val = (int32_t)R(rt(current_opcode));

    R(rt(current_opcode)) = rs_val < rt_val;
}

void sltu()
{
    uint32_t rs_val = R(rs(current_opcode));
    uint32_t rt_val = R(rs(current_opcode));

    R(rd(current_opcode)) = rs_val < rt_val;
}

void handle_instruction()
{
    current_opcode = read_word(pc);

    // Get primary opcode from 6 highest bits
    uint8_t primary_opcode = (current_opcode & 0xFC000000) >> 26;
    // Get secondary opcode from 6 lowest bits
    uint8_t secondary_opcode = current_opcode & 0x3F;

    if (primary_opcode == 0x00)
        ((void (*)(void))secondary_opcodes[secondary_opcode].function)();
    else
        ((void (*)(void))primary_opcodes[primary_opcode].function)();

    // Jump if we have a jump in delay slot, otherwise increment pc normally
    if (jmp_next_instr)
    {
        jmp_next_instr = false;
        pc = jmp_address;
    }
    else
        pc += 0x4;
}
