#include "cpu_utils.h"

//NEIL: Instruction Flow (Fetch, Decode, Control) 

uint32_t Fetch(const std::vector<uint32_t>& instruction_memory) {
    //Pulling 32 bit instruction from memory based on pc, pc increments by 4 every cycle

    //Access instruction from memory 
    uint32_t inst = instruction_memory[pc / 4];
    
    //Default next PC
    next_pc = pc + 4;
    
    return inst;
}

void Decode(uint32_t inst, DecodedInst& decoded) {
    //bit masking to break apart instruction into opcode, register indices, immediates
    //handled sign extension for i/s-type immediates

    decoded.instruction = inst;
    
    //Extraction
    decoded.opcode = inst & 0x7F;
    decoded.rd_index = (inst >> 7) & 0x1F;
    int funct3 = (inst >> 12) & 0x7;
    int rs1 = (inst >> 15) & 0x1F;
    int rs2 = (inst >> 20) & 0x1F;
    int funct7 = (inst >> 25) & 0x7F;

    decoded.rs1_val = rf[rs1];
    decoded.rs2_val = rf[rs2];

    //Control Logic & Immediate Generation
    if (decoded.opcode == 0x33) { // R-type
        decoded.reg_write = true;
        decoded.imm = 0;
        //0x20 in funct7 indicates SUB
        decoded.alu_ctrl = (funct7 == 0x20) ? 1 : 0; 
    } 
    else if (decoded.opcode == 0x13) { //I-type (addi)
        decoded.reg_write = true;
        decoded.imm = (int)inst >> 20; //Sign-extend 12-bit immediate
        decoded.alu_ctrl = 0; //ADD
    }
    else if (decoded.opcode == 0x03) { //lw
        decoded.reg_write = true;
        decoded.mem_read = true;
        decoded.imm = (int)inst >> 20;
        decoded.alu_ctrl = 0; //ALU calculates address
    }
    else if (decoded.opcode == 0x23) { // sw
        decoded.mem_write = true;
        int imm_low = (inst >> 7) & 0x1F;
        int imm_high = (inst >> 25) & 0x7F;
        int s_imm = (imm_high << 5) | imm_low;
        // Manual Sign Extension for 12-bit
        if (s_imm & 0x800) s_imm |= 0xFFFFF000;
        decoded.imm = s_imm;
        decoded.alu_ctrl = 0;
    }
    else if (decoded.opcode == 0x63) { // beq
        decoded.is_branch = true;
        // B-type imm reconstruction: [31][7][30:25][11:8]
        int b_imm = ((inst >> 31) << 12) | (((inst >> 7) & 0x1) << 11) | 
                    (((inst >> 25) & 0x3F) << 5) | (((inst >> 8) & 0xF) << 1);
        // Sign extend from 13th bit
        if (b_imm & 0x1000) b_imm |= 0xFFFFE000;
        decoded.imm = b_imm;
    }
}

//CRISTIAN: Data Path & Memory (Execute, Mem, WB)

void Execute(DecodedInst& decoded) {
    //take decoded values and run them thru ALU
    //handles arithmetic for R/I type instructions and calcualtes branch_target for branches

    // ALU Operations
    if (decoded.opcode == 0x33) { // R-type
        if (decoded.alu_ctrl == 1) decoded.alu_result = decoded.rs1_val - decoded.rs2_val;
        else decoded.alu_result = decoded.rs1_val + decoded.rs2_val;
    } 
    else if (decoded.opcode == 0x13 || decoded.opcode == 0x03 || decoded.opcode == 0x23) {
        // I-type, Load, Store all use RS1 + IMM
        decoded.alu_result = decoded.rs1_val + decoded.imm;
    }
    else if (decoded.is_branch) {
        // Subtraction to check for equality
        decoded.alu_result = decoded.rs1_val - decoded.rs2_val;
    }

    alu_zero = (decoded.alu_result == 0) ? 1 : 0;
    branch_target = pc + decoded.imm;

    // Resolve Next PC
    if (decoded.is_branch && alu_zero) {
        pc = branch_target;
    } else {
        pc = next_pc;
    }
}

void Mem(DecodedInst& decoded) {
    //manage data memory array loads and stores

    // Memory access is Word-aligned (divide address by 4)
    int address_index = decoded.alu_result / 4;

    if (decoded.mem_read) {
        decoded.mem_read_data = d_mem[address_index];
    }
    if (decoded.mem_write) {
        d_mem[address_index] = decoded.rs2_val;
    }
}

void WriteBack(DecodedInst& decoded) {
    //final result is written back to correct destination register in register file

    if (decoded.reg_write && decoded.rd_index != 0) {
        // Decide whether to write ALU result or Memory Data
        rf[decoded.rd_index] = decoded.mem_read ? decoded.mem_read_data : decoded.alu_result;
    }
    //total_clock_cycles++;
}