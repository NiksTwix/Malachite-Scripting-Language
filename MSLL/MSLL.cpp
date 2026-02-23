// MSLL.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "include/Linker.hpp"
int main(int argc, char* argv[])    //argc - count of args, argv - array of arg-strings
{
    MSLL::Linker linker;


    if (argc < 2) 
    {
        std::cout << "Interactive mode\n";

        std::cout << "Enter path to file:";

        std::string path;

        std::getline(std::cin, path);


        

        uint16_t index = 0;
        for (const std::string& str : linker.GetVMList()) 
        {
            std::cout << index << ") " << str << "\n";
            index++;
        }
        std::cout << "Enter vm model version (number):";
        uint16_t choice;
        std::cin >> choice;

        linker.Link(path, choice);

    }
    else if (argc > 1)
    {
        uint16_t index = 0;
        if (argc > 2) 
        {
            std::string version = std::string(argv[2]);
            index = UINT16_MAX; //MAX of uint16_t
            const auto& vms = linker.GetVMList();
            for (uint16_t i = 0; i < vms.size(); i++)
            {
                if (version == vms[i])
                {
                    index = i;
                    break;
                }
            }
            if (index == UINT16_MAX)
            {
                std::cerr << "Invalid vm_type parameter.\n";
                return 0;
            }
        }
    
        std::string path = std::string(argv[1]);
        linker.Link(path, index);
    }
    else 
    {
        std::cout << argc << " error";
    }
}

