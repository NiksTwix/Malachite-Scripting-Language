// MSLL.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//


#include "include/Linker.hpp"

#include <string>

int main(int argc, char* argv[])    //argc - count of args, argv - array of arg-strings
{
    MSLL::Linker linker;


    if (argc < 2) 
    {
        std::cout << "Interactive mode\n";

        std::cout << "Enter path to file:";

        std::string path;

        std::getline(std::cin, path);


        

        uint8_t index = 1;
        for (const std::string& str : linker.GetVMList()) 
        {
            std::cout << std::to_string(index) << ") " << str << "\n";
            index++;
        }
        std::cout << "Enter vm model version (number):";
        char choice;
        std::cin >> choice;
        linker.SetOutputMode(true);
        bool result = linker.Link(path, (MSLL::LinkDefinitions::VMs)(choice - '0'));
        if (!result)
        {
            std::cout << "Unnamed linking error";
            std::cin;
            return 1;
        }
    }
    else if (argc > 1)
    {
        uint8_t index = 0;
        if (argc > 2) 
        {
            std::string version = std::string(argv[2]);
            index = UINT8_MAX; //MAX of uint16_t
            const auto& vms = linker.GetVMList();
            for (uint8_t i = 0; i < vms.size(); i++)
            {
                if (version == vms[i])
                {
                    index = i;
                    break;
                }
            }
            if (index == UINT8_MAX)
            {
                std::cerr << "Invalid vm_type parameter.\n";
                return 1;
            }
        }
    
        std::string path = std::string(argv[1]);
        linker.SetOutputMode(false);//Without debug output(Loading and etc)
        bool result = linker.Link(path, (MSLL::LinkDefinitions::VMs)index);

        if (!result) return 1;
    }
}

