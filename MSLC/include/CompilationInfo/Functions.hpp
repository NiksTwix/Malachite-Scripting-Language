#pragma once

#include "CommonInfo.hpp"
#include <unordered_map>




namespace MSLC
{
	namespace CompilationInfo
	{
		namespace Functions
		{
			enum FunctionFlags : uint32_t {
				None = 0,
				Method = 1 << 0,          // method of object's template
				Static = 1 << 1,          // static method
				Virtual = 1 << 2,         // virtual method
				Override = 1 << 3,        // overriding
				ConstMethod = 1 << 4,     // const methods (doesnt edit object)
				Variadic = 1 << 5,        // varioty count of arguments
				Inline = 1 << 6,          // inline function
				External = 1 << 7,        // external function (from library)
				//Builtin = 1 << 8          // inner function of MSLC
			};

			struct FunctionArgument 
			{
				std::string name;
				Values::ValueInfo vinfo;
			};

			class FunctionDescription : public SymbolDescription
			{
			public:
				std::vector<FunctionArgument> arguments;

				Values::ValueInfo return_vinfo;

				FunctionFlags flags;

				size_t vtable_index = INVALID_ID; //Index in table of virtual methods (in the future)

				//For another info (maybe instruction pointer in pseudo/byte code)

				size_t start_byte_ip = INVALID_ID;


				
			};
		}
	}
}