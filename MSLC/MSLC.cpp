// MSLC.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>

#include "include\Tokenization\Lexer.hpp"
#include "include\Preprocessing\AMGBuilder.hpp"
using namespace MSLC;

int main()
{
    Diagnostics::Logger::Get().PrintWithFormat(Diagnostics::InformationMessage("Hello world, %s!", Diagnostics::MessageType::TypeError, 0 == -1 ? Diagnostics::None : Diagnostics::SourceCode, 0), "Nirinis");

    std::string b_path = "C:\\Users\\korsi\\OneDrive\\Рабочий стол\\MSLCTests\\ImportChainTests\\p1.txt";

    Preprocessing::AMGBuilder amg;

    auto result = amg.Analyze(b_path);

    for (auto id: result.processing_order)
    {
        std::cout << result.files[id].path << "\n";
    }

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
