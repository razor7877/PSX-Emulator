#include "tests.h"
#include "cpu.h"
#include "logging.h"
#include "memory.h"

void test_addi()
{
    R0 = 32;

    // Add 8 to r0, store in r1
    current_opcode = 0b00100000000000010000000000001000;

    addi();

    if (R1 != 40)
        log_error("ADDI instruction did not successfully add and store the sum\n");

    reset_cpu_state();

    R0 = 32;

    // Add -10 to r0, store in r1
    current_opcode = 0b00100000000000011111111111110110;

    addi();

    if (R1 != 22)
        log_error("ADDI instruction did not successfully add and store the sum with negative constant\n");

    log_info("Finished testing ADDI\n");

    reset_cpu_state();
}

void test_jr()
{

}

void test_beq()
{
    R0 = 0;
    R1 = 0;

    current_opcode = 0b00010100000000010000000001000000;

    uint32_t last_pc = pc;
    beq();

    // NOP until we actually jump to the branch
    current_opcode = 0;
    handle_instruction();

    if (pc != (last_pc + 0x100))
        log_error("Incorrect BEQ behavior, did not trigger jump on equal registers\n");

    reset_cpu_state();

    R0 = 0;
    R1 = 1;

    // +0x100 offset
    current_opcode = 0b00010100000000010000000001000000;

    last_pc = pc;
    beq();

    // NOP until we actually jump to the branch
    current_opcode = 0;
    handle_instruction();

    if (pc == (last_pc + 0x100))
        log_error("Incorrect BEQ behavior, triggered jump on different registers\n");

    reset_cpu_state();

    log_info("Finished testing BEQ\n");
}

void test_bne()
{
    R0 = 0;
    R1 = 0;

    current_opcode = 0b00010100000000010000000001000000;

    uint32_t last_pc = pc;
    bne();

    // NOP until we actually jump to the branch
    current_opcode = 0;
    handle_instruction();

    if (pc == (last_pc + 0x100))
        log_error("Incorrect BNE behavior, triggered jump on equal registers\n");

    reset_cpu_state();

    R0 = 0;
    R1 = 1;

    // +0x100 offset
    current_opcode = 0b00010100000000010000000001000000;

    last_pc = pc;
    bne();
    
    // NOP until we actually jump to the branch
    current_opcode = 0;
    handle_instruction();

    if (pc != (last_pc + 0x100))
        log_error("Incorrect BNE behavior, did not trigger jump on different registers\n");

    reset_cpu_state();

    log_info("Finished testing BNE\n");
}

void test_lui()
{
    R0 = 0;

    // LUI 0xABCD into register 0
    current_opcode = 0b00111100000000001010101111001101;

    lui();

    if (R0 != 0xABCD0000)
        log_error("Incorrect value in r0 after LUI of 0xABCD! Got %x instead\n", R0);

    log_info("Finished testing LUI\n");

    reset_cpu_state();
}

void test_ori()
{
    R0 = 0xFFFF0000;

    // ORI r0 with 0xFFFF, store in R1
    current_opcode = 0b00110100000000011111111111111111;

    ori();

    if (R1 != 0xFFFFFFFF)
        log_error("ORI instruction did not correctly calculate the result! Got %x\n", R2);

    log_info("Finished testing ORI\n");

    reset_cpu_state();
}

void test_sw()
{
    R0 = 100;
    R1 = 0xABCDDCBA;

    uint16_t offset = -16;

    // SW r1 at address r0 offset by -16
    current_opcode = 0b10101100000000010000000000000000 | offset;

    sw();

    uint32_t value = read_word(R0 + (int16_t)offset);

    if (value != 0xABCDDCBA)
        log_error("SW instruction did not correctly store the word in memory! Got %x\n", value);

    log_info("Finished testing SW\n");

    reset_cpu_state();
}

void test_addiu()
{
    R0 = 13;
    uint16_t immediate = 37;

    current_opcode = 0b00100100000000010000000000000000 | immediate;

    addiu();

    if (R1 != 50)
        log_error("ADDIU instruction did not correctly calculate the result! Got %x\n", R1);

    reset_cpu_state();

    log_info("Finished testing ADDIU\n");
}

void test_j()
{

}

void test_or()
{
    R0 = 0xFFFF0000;
    R1 = 0x0000FFFF;

    // OR R0 with R1, store in R2
    current_opcode = 0b00000000000000010001000000100101;

    op_or();

    if (R2 != 0xFFFFFFFF)
        log_error("OR instruction did not correctly calculate the result! Got %x\n", R2);

    log_info("Finished testing OR\n");

    reset_cpu_state();
}

void test_instructions()
{
    log_info("Starting CPU instructions unit tests...\n");

    test_beq();
    test_bne();
    test_addi();
    test_or();
    test_ori();
    test_lui();
    test_sw();
    test_addiu();
}

void test_memory()
{
    uint8_t value = 0;

    for (int i = 0; i < 2048 * KIB_SIZE; i++)
    {
        write_word(i, value);

        if (read_word(i) != value)
            log_error("KUSEG RAM error, did not get written value back when reading at %x\n", i);

        value++;
    }

    clear_memory();

    for (int i = 0; i < 2048 * KIB_SIZE; i++)
    {
        write_word(0x80000000 + i, value);

        if (read_word(0x80000000 + i) != value)
            log_error("KSEG0 RAM error, did not get written value back when reading at %x\n", i);

        value++;
    }

    clear_memory();

    for (int i = 0; i < 2048 * KIB_SIZE; i++)
    {
        write_word(0xA0000000 + i, value);

        if (read_word(0xA0000000 + i) != value)
            log_error("KSEG1 RAM error, did not get written value back when reading at %x\n", i);

        value++;
    }

    clear_memory();

    log_info("Finished testing RAM in KUSEG, KSEG0, KSEG1\n");
}
