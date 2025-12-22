#include <iostream>
#include <vector>
#include <string>
#include "include/mslvm/mslvm.hpp"

using namespace MSLVM;

int main()
{
    VMState state;

    std::vector<VMOperation> operations =
    {
        
        {VMOperationCode::MOV_RI, 2, 9000},
        {VMOperationCode::MOV_RI, 5, HEAP_SIZE / 2},
        {VMOperationCode::ALLOCATE_MEMORY,0,5},
        {VMOperationCode::ALLOCATE_MEMORY,1,5},
        {VMOperationCode::STORE_BY_ADDRESS, 2, 1, 8},
        {VMOperationCode::LOAD_BY_ADDRESS, 3, 1, 8},
    };

    execute_code_switch(state, operations.data(), operations.size());
    std::cout << "\n";
    state.memory.HFI.debug_print();
    std::cout << "\nRegisters:\n";
    for (int i = 0; i < 8; i++)
    {
        std::cout << std::to_string(state.registers[i].u) << " ";
    }

    operations =
    {
        {VMOperationCode::FREE_MEMORY,0,5},
        {VMOperationCode::FREE_MEMORY,1,5},
    };
    state.registers[SpecialRegister::IP] = 0;
    execute_code_switch(state, operations.data(), operations.size());
    std::cout << "\n";
    state.memory.HFI.debug_print();
    //std::cout << "\n";
    //for (int i = 0; i < 16; i++) 
    //{
    //    std::cout << std::to_string(state.memory.stack[i]) << " ";
    //}
    //std::cout << "\nglobal----\n";

    std::cin;

}


