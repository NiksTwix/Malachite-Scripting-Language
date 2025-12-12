// MSLVM.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "include/mslvm/mslvm.hpp"
#include <vector>
#include <string>
using namespace MSLVM;
int main()
{
    VMState state;

    std::vector<VMOperation> operations =
    {
        { VMOperationCode::MOV_RI_INTEGER,0,0,0, Register((uint64_t)2000)},
        { VMOperationCode::MOV_RI_INTEGER,1,0,0, Register((uint64_t)2000)},
        { VMOperationCode::ADD_RRR_INTEGER,0,0,1},
        { VMOperationCode::PUSH,8,6,0},
        { VMOperationCode::STORE_LOCAL,0,0,8},
        { VMOperationCode::PUSH,8,6,0},
        { VMOperationCode::LOAD_LOCAL,3,0,8},
        { VMOperationCode::STORE_LOCAL,8,0,8},
        { VMOperationCode::STORE_LOCAL,2,0,2},
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

    VMState state1;

    operations =
    {
       {VMOperationCode::MOV_RI_INTEGER,0,0,0, Register((uint64_t)2000)},
       { VMOperationCode::MOV_RI_INTEGER,1,0,0, Register((uint64_t)2000)},
       { VMOperationCode::ADD_RRR_INTEGER,0,0,1},
       { VMOperationCode::PUSH,8,6,0},
       { VMOperationCode::STORE_GLOBAL,0,0,8},
       { VMOperationCode::PUSH,8,6,0},
       { VMOperationCode::LOAD_GLOBAL,3,0,8},
       { VMOperationCode::STORE_GLOBAL,8,0,8},
    };
    execute_code_switch(state1, operations.data(), operations.size());
    for (int i = 0; i < 8; i++)
    {
        std::cout << std::to_string(state1.registers[i].u) << " ";
    }
    std::cout << "\n";
    for (int i = 0; i < 16; i++)
    {
        std::cout << std::to_string(state1.memory.stack[i]) << " ";
    }

    while (true) {}

}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
