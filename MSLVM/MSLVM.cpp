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
        { VMOperationCode::MOV_RI_INTEGER,0,2000},
        { VMOperationCode::MOV_RI_INTEGER,1,'+'},
        { VMOperationCode::MOV_RI_INTEGER,2,' '},
        { VMOperationCode::MOV_RI_INTEGER,9,'\n'},
        { VMOperationCode::ADD_RRR_INTEGER,0,0,1},
        { VMOperationCode::PUSH,8,6,0},
        { VMOperationCode::STORE_LOCAL,0,0,8},
        { VMOperationCode::PUSH,8,6,0},
        { VMOperationCode::LOAD_LOCAL,3,0,8},
        { VMOperationCode::STORE_LOCAL,8,0,8},
        { VMOperationCode::STORE_LOCAL,2,0,2},
        { VMOperationCode::VM_CALL,VMCallType::PRINT_CHAR,1,0},
        { VMOperationCode::VM_CALL,VMCallType::PRINT_CHAR,2,0},
        { VMOperationCode::VM_CALL,VMCallType::PRINT_INTEGER,0,0},
        { VMOperationCode::VM_CALL,VMCallType::PRINT_CHAR,2,0},
        { VMOperationCode::VM_CALL,VMCallType::PRINT_INTEGER,3,0},
        { VMOperationCode::VM_CALL,VMCallType::PRINT_CHAR,9,0},
        { VMOperationCode::MOV_RI_REAL, 4, Register(3.14) },  // R4 = 3.14
        { VMOperationCode::VM_CALL, VMCallType::PRINT_REAL, 4, 0 },
        { VMOperationCode::VM_CALL,VMCallType::PRINT_CHAR,9,0 },
    };

    execute_code_switch(state, operations.data(), operations.size());
    for (int i = 0; i < 8; i++)
    {
        std::cout << std::to_string(state.registers[i].u) << " ";
    }
    std::cout << "\n";
    for (int i = 0; i < 16; i++) 
    {
        std::cout << std::to_string(state.memory.stack[i]) << " ";
    }
    std::cout << "\nglobal----\n";

    //VMState state1;
    //
    //execute_code_switch(state1, operations.data(), operations.size());
    //for (int i = 0; i < 8; i++)
    //{
    //    std::cout << std::to_string(state1.registers[i].u) << " ";
    //}
    //std::cout << "\n";
    //for (int i = 0; i < 16; i++)
    //{
    //    std::cout << std::to_string(state1.memory.stack[i]) << " ";
    //}

    std::cin;

}


