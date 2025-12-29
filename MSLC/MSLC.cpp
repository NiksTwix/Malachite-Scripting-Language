// MSLC.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>

#include "include\Tokenization\Lexer.hpp"

using namespace MSLC;

int main()
{
    Diagnostics::Logger::Get().PrintWithFormat(Diagnostics::InformationMessage("Hello world, %s!", Diagnostics::MessageType::TypeError, 0 == -1 ? Diagnostics::None : Diagnostics::SourceCode, 0), "Nirinis");

    std::string b = R"Code(
// Malachite Scripting Language syntax example

//Directives (compiler)
#define_const CONTANT_1 = 2000	//Inserts in the place of using
#define_const string_1 = "String"


#define_insertion FUNC(x,y,z)
{
	x = y + z * x + *x
}



#import "test1.msl"		//imports module 
#importlib "test2"		//imports library 

//Defining variables

int x = 200;
real y = 200.0;

const int x1 = 200;

//Arrays
int size = 10
`int array = allocate_memory(size)	//allocate_memory - its function in standard library
free_memory(array,size)

//or

`int array = new int[size]	
delete array			

//VM programming

op_code			//Lite mode (vm asm compiler)
{
	//default pseudo commands: mathematic, logic, push/pop, load/store, allocate/free,  label and jumping
}

op_code kernel		//Kernel mode (vm asm compiler, but with more instructions)
{
	//all vmoperations
}

/*
	Compiler takes from 120 (i think it will be 115-120) general register 6 for op_code section and gives them names: RA,RB,RC,RD,RE,RF 
	VM can use 114 register for operations.
*/


//Condition checking
if (x == y): x = 2001
elif (x != y + 1)
{
	x = 2002
}
else
{
	x = 2003
}

//Loops

for x (start,end,step)
{

}
//Interval cycles (in the future)
for x in [start_index,end_index,step] // another combinations: (), [), (]
{

}

while (x==y)
{
	if (x != y) break;
	elif (x != y + 1)
	{
		x += 1
		y += 1
	}
}

func f1(int x, int y, int z) -> int
{
	return x*y*z;
}

//func inheritance 

func f2(int x,int y, int z) -> int
{
	@fblock result_calculating
	{
	
		return x*y*z;
	}
}
@finherit f2 (x,y,z) //new names of parameters; copies code from f1 and inserts here
func f3(int x,int y, int z) -> int
{
	//code before result_calculating
	@foverride_block result_calculating
	{
		return x*y*z * 2;
	}
	//code after result_calculating	
}


//OOP
object_template INT
{
@access_mode(private)
	int x;
@access_mode(public)
	INT();
}
object_template Vector: INT
{
//code
@access_mode(public)
	@property(get:public, set:protected)
	int field1;
} 

Vector k = new Vector();	//Pointer!
k->y = 0;			//Pointer + y_offset
delete k

gc Vector k = new Vector();	//GC mode

//deletes automatically
 
//Alias
alias meter: int


namespace MalachiteTest	
{
	@access_mode(private) 
	int test = 0		//label "private" - another symbols are automatically public
	@stack_only
	@access_mode(private) 
	object_template Vector1
	{
		
	}

	@access_mode(private)
	int c = 0;
	@access_mode(public)
	func K():return 10


}
)Code";
	Tokenization::Lexer lex;
	auto tokens = lex.ToTokens(b, 0);

	for (auto token : tokens) 
	{
		std::cout << "Type:" << (uint64_t)token.type << "\t|Line:" << token.line << "|Value:" << token.value.ToString() << "\n";
	
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
