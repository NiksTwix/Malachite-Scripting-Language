#include <iostream>
#include <vector>
#include <string>
#include "include/mslvm/mslvm.hpp"

using namespace MSLVM;

int main()
{
    VirtualMachine machine;

    bool loading = machine.LoadScript("D:\\Games\\MSLCTests\\AST\\first\\first.msli");
    std::cout << loading << "\n";
    clear_vm_state(machine.GetState(),false);
    execute_code_switch(machine.GetState());
    std::cout << "\n";
    //state.memory.HFI.debug_print();
    std::cout << "Errors:\n";

    while (!machine.GetState().error_stack.empty()) 
    {
        auto error = machine.GetState().error_stack.pop();
        std::cout << error.code << ";";
    }


    std::cout << "\nRegisters:\n";
    for (int i = 0; i < 8; i++)
    {
        std::cout << std::to_string(machine.GetState().registers[i].u) << " ";
    }

    //std::cout << "\n";
    //for (int i = 0; i < 16; i++) 
    //{
    //    std::cout << std::to_string(state.memory.stack[i]) << " ";
    //}
    //std::cout << "\nglobal----\n";

    std::cin;

}


