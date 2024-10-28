#pragma once

#define cop0_code(value) ((((value & 0x03E00000) >> 21)))
#define CPR0(value) _cop0_registers[value]

extern uint32_t _cop0_registers[64];

void handle_cop0_instruction();
void handle_cop1_instruction();
void handle_cop2_instruction();
void handle_cop3_instruction();
