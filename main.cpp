#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <iomanip>
#include "cpu_utils.h"

//NEIL
//main loop + fetch coordination
//file reading logic and 5 stage execution flow

int pc = 0;
int next_pc = 0;
int rf[32] = {0};
int d_mem[32] = {0};
int total_clock_cycles = 0;
int alu_zero = 0;
int branch_target = 0;

int main() {
    // Initial state
    rf[1] = 0x20; rf[2] = 0x5; rf[10] = 0x70; rf[11] = 0x4;
    d_mem[28] = 0x5; d_mem[29] = 0x10; 

    // Hardcoded filename
    std::ifstream file("sample_part1.txt"); 
    if (!file.is_open()) {
        std::cerr << "Error: Could not find sample_part1.txt" << std::endl;
        return 1;
    }

    std::vector<uint32_t> instruction_memory;
    std::string binaryLine;
    while (file >> binaryLine) {
        if (binaryLine.length() >= 32) {
            instruction_memory.push_back(std::bitset<32>(binaryLine).to_ulong());
        }
    }
    //5 stage counting logic
    while (pc / 4 < instruction_memory.size()) {
        DecodedInst current_inst = {0};

        // Simulate each stage taking 1 cycle
        uint32_t raw_inst = Fetch(instruction_memory); 
        //total_clock_cycles++; // Fetch finishes (Cycle 1)
        
        Decode(raw_inst, current_inst);                
       // total_clock_cycles++; // Decode finishes (Cycle 2)
        
        Execute(current_inst);                         
       // total_clock_cycles++; // Execute finishes (Cycle 3)
        
        Mem(current_inst);                             
       // total_clock_cycles++; // Memory finishes (Cycle 4)
        
        WriteBack(current_inst);                       
        total_clock_cycles++; // Writeback finishes (Cycle 5)

        
        std::cout << "total_clock_cycles " << total_clock_cycles << ":" << std::endl;
        
        if (current_inst.reg_write && current_inst.rd_index != 0) {
            std::cout << " x" << current_inst.rd_index << " is modified to 0x" 
                      << std::hex << rf[current_inst.rd_index] << std::dec << std::endl;
        }
        
        if (current_inst.mem_write) {
            std::cout << " memory 0x" << std::hex << current_inst.alu_result << std::dec 
                      << " is modified to 0x" << std::hex << current_inst.rs2_val << std::dec << std::endl;
        }
        
        std::cout << " pc is modified to 0x" << std::hex << pc << std::dec << "\n" << std::endl;
    }
    std::cout << "program terminated:" << std::endl;
    std::cout << "total execution time is " << total_clock_cycles << " cycles" << std::endl;

    return 0;
}


//Need to update decode func to handle J-type immediate encoding + register offset calculation for JALR
//Need to update fetch logic, need to add 3rd option for jump address
//Need to update writeback func, for jump instructions we need to save return address into destination register