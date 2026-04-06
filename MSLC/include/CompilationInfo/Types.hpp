#pragma once
#include "CommonInfo.hpp"
#include <unordered_map>

namespace MSLC 
{
	namespace CompilationInfo 
	{
		namespace Types 
		{
			constexpr size_t STANDART_ALIGNMENT = 8;


			enum TypeFlags : uint32_t {
				None = 0,
				Signed = 1 << 0,      // for integers
				Const = 1 << 1,       // const type
				Volatile = 1 << 2,    // volatile
				Trivial = 1 << 3,     // trivial for copy
				Pod = 1 << 4,         // plain old data
				Abstract = 1 << 5,    // absract object_template
				Final = 1 << 6        // inheriting is forbidden
			};
			enum class TypeCategory {
				Void,
				Primitive,      // int, float, bool, char
				Pointer,        // int*, MyStruct*
				Array,          // int[10], int[]
				ObjectTemplate,
				Interface,
				Enum,
				Alias,          // typedef/alias
				Function,       // Type for functions (for functions pointers)
				Template        // For generics (in the future....probably)
			};


			struct TypeField 
			{
				std::string name;		//name of field
				Values::ValueInfo vinfo;
				uint32_t offset = 0;	//Offset from start of object
				AccessMode read_access_mode = AccessMode::Public;
				AccessMode write_access_mode = AccessMode::Public;
			};

			enum PrimitiveAnalogs //with priority 0 - 2
			{
				Int,
				UInt,
				Real,
			};

			class TypeDescription : public SymbolDescription
			{
			public:
				std::unordered_map<std::string, TypeField> fields;
				std::unordered_map<std::string, Functions::FunctionID> methods;
				std::unordered_map<std::string, Functions::FunctionID> connected_functions;
				size_t size;
				TypeID inheriting_id = INVALID_ID;
				TypeFlags flags;
				TypeCategory category;
				PrimitiveAnalogs primitive_analog;
				size_t alignment = STANDART_ALIGNMENT;

				//Meta
				size_t GetAlignedSize() 
				{
					return ((size + alignment - 1) / alignment) * alignment;
				}

			};

		}
	}
}