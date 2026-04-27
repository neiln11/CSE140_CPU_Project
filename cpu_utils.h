#ifndef CPU_UTILS_H
#define CPU_UTILS_H

#include <cstdint>
#include <iostream>

//SHARED WORK
// Global Variables 

extern int pc;
extern int next_pc;
extern int rf[32];
extern int d_mem[32];
extern int total_clock_cycles;
extern int alu_zero;
extern int branch_target;

// Struct to pass data between stages cleanly
struct DecodedInst {
    uint32_t instruction; // The raw 32-bit instruction
    int opcode;
    int rs1_val;          // Value read from rf[rs1]
    int rs2_val;          // Value read from rf[rs2]
    int rd_index;         // Where to write back
    int imm;              // Sign-extended immediate
    int alu_ctrl;         // What the ALU should do
    bool mem_read;
    bool mem_write;
    bool reg_write;
    bool is_branch;
    
    // Outputs from Execute & Mem
    int alu_result;
    int mem_read_data;
};

// Function Prototypes
uint32_t Fetch(const std::vector<uint32_t>& instruction_memory);
void Decode(uint32_t inst, DecodedInst& decoded);
void Execute(DecodedInst& decoded);
void Mem(DecodedInst& decoded);
void WriteBack(DecodedInst& decoded);

#endif