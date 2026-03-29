#pragma once
#include <cstdint>
#include <vector>
#include <string_view>
#include <string>
#include <unordered_map>
namespace MSLL 
{
	namespace LinkDefinitions 
	{
		using moduleid = uint16_t;
		using constantid = size_t;
		using symbolid = size_t;

		enum CommandSource : uint8_t {
			Register = 1,        // register (index), can contain a dynamic memory address
			MemoryAddress,   // address in memory (offset/static)
			Immediate,       // immediated value
			Constant,         // index in the constants poop
			Symbol,				//for linking with symbols table
		};
		enum OutputTypes : uint8_t
		{
			Invalid = 0xff,
			Char = 0,
			Int,
			Real,
		};

		enum class ByteOpCode : uint8_t
		{
			NOP,

			COMP_FLG,	//CompilationFlag arg0  - CompilationFlag. In functions declarations

			SECTION_ARITHMETIC_ST,	//reg0,reg1,reg2
			ADDR,
			SUBR,
			DIVR,
			MULR,
			NEGR,
			EXPR,

			ADDI,
			SUBI,
			DIVI,
			MULI,
			NEGI,
			MODI,
			EXPI,

			ADDU,
			SUBU,
			DIVU,
			MULU,
			MODU,
			EXPU,
			SECTION_ARITHMETIC_ED,
			//Logic
			SECTION_LOGIC_ST,
			AND,
			OR,
			NOT,
			BIT_OR,
			BIT_NOT,
			BIT_AND,
			BIT_OFFSET_LEFT,
			BIT_OFFSET_RIGHT,

			CMPI,       // destination|source0|source1 If VM using flag system destination can be ignored by linker  
			CMPU,       // destination|source0|source1 If VM using flag system destination can be ignored by linker  
			CMPR,   	// destination|source0|source1 If VM using flag system destination can be ignored by linker
			SECTION_LOGIC_ED,
			//Memory
			SECTION_MEMORY_ST,
			MOVRR,
			MOVRI,
			STACK_UP,		//size - arg0      
			STACK_DOWN,    //size - arg0 
			PUSH,		//reg and size
			POP,		//reg and size
			LEA_STATIC,		// Static address calculating: MemoryAddress (static) -> reg 
			LEA_DYNAMIC,	// Dynamic address calculating: register (with address) -> register_dest

			LOAD_DYNAMIC,	//Loading by address in register //Register With Loading Address,Register,Size
			STORE_DYNAMIC,	//Storing by address in register //Register With Storing Address,Register,Size

			LOAD_CONST_STATIC,	//load constant value to register. Linker will replace id on offset
			LEA_CONST,	//Load constant pointer in register. ConstantID, register

			LOAD_STATIC,	//Loading by address in register //MemoryAddress,Register,Size
			STORE_STATIC,	//Storing by address in register //MemoryAddress,Register,Size

			SECTION_MEMORY_ED,
			// Control flow arg0 = where
			SECTION_CONTROL_FLOW_ST,
			JMPLABEL,
			JMP,
			JMPCV,      //CV- Condition Valid - arg0[where], arg1[condition register]
			JMPCNV,     //CNV - Condition Not Valid - arg0[where], arg1[condition register]
			CALL,
			RET,		//-> inserting GRAB_FRAME with negative in MSLVM_1
			EXT,	//EXIT
			SECTION_CONTROL_FLOW_ED,

			SECTION_SPECIAL_ST,
			// Type Convertion
			TC_ITR,    //Type Convertion Integer To Real
			TC_RTI,    //Type Convertion Real To Integer

			TC_UTR,    //Type Convertion Unsigned Integer To Real
			TC_UTI,    //Type Convertion Unsigned Integer To Integer

			TC_RTU,    //Type Convertion Real To Unsigned Integer
			TC_ITU,


			SYMBOL_LABEL,	//Symbol declaring (function), arg0 - symbol_id -> inserting GRAB_FRAME with positive in MSLVM_1

			SECTION_SPECIAL_ED,
		};

		enum Flag {
			UnhandledSymbol = 1 << 0,
		};

		struct CommandArgument {
			union {
				size_t data;		// For Address
			};

			CommandSource type;

			// For comfort
			bool is_register() const {
				return type == Register;
			}

			CommandArgument() = default;
			CommandArgument(size_t data, CommandSource source) : data(data), type(source) {}
		};
		struct ByteCommand {
			ByteOpCode code;
			CommandArgument arg0;
			CommandArgument arg1;
			CommandArgument arg2;

			// Meta
			uint32_t flags = 0;
			enum Flag {
				UnhandledSymbol = 1 << 0,	//Linker must search and handle symbol in realocation table
				//Volatile = 1 << 0,   // Command has side effects
				//Branch = 1 << 1,   //	Jump Command
				//Call = 1 << 2,   // Calling command
				//Return = 1 << 3,   // Return command
				//Memory = 1 << 4,   // Memory 
				//Syscall = 1 << 5,   // System call
				//LocalOffset = 1 << 6,   //Flag "Local offset" used in function bodies in order to linker disable offset autocorrect

			};

			// Debug_info
			uint32_t source_line = 0;


			// Constructors for comfort
			ByteCommand(ByteOpCode c) : code(c) {}
			ByteCommand(ByteOpCode c, CommandArgument a0) : code(c), arg0(a0), arg1(0, CommandSource::Immediate), arg2(0, CommandSource::Immediate) {}
			ByteCommand(ByteOpCode c, CommandArgument a0, CommandArgument a1)
				: code(c), arg0(a0), arg1(a1), arg2(0, CommandSource::Immediate) {
			}
			ByteCommand(ByteOpCode c, CommandArgument a0, CommandArgument a1, CommandArgument a2)
				: code(c), arg0(a0), arg1(a1), arg2(a2) {
			}
			ByteCommand(ByteOpCode c, CommandArgument a0, CommandArgument a1, CommandArgument a2, uint32_t flags)
				: code(c), arg0(a0), arg1(a1), arg2(a2), flags(flags) {
			}
			ByteCommand(ByteOpCode c, CommandArgument a0, CommandArgument a1, CommandArgument a2, uint32_t flags, uint32_t s_line)
				: code(c), arg0(a0), arg1(a1), arg2(a2), flags(flags),source_line(s_line) {
			}
			ByteCommand() : code(ByteOpCode::NOP), arg0(0, CommandSource::Immediate), arg1(0, CommandSource::Immediate), arg2(0, CommandSource::Immediate) {}
		};

		enum class SymbolType : uint8_t
		{
			Undefined,
			Variable,
			Type,
			Function,
			Namespace,
		};
		namespace MSLOData		//in bytes
		{
			constexpr uint32_t mo_header_reserved_size = 500;
			constexpr uint32_t co_header_reserved_size = 100;

			constexpr uint32_t magic_size = 5;
			constexpr uint32_t size_of_module_prefix = 32;//31 character + \0
			constexpr uint32_t size_of_module_extension = 10;
			constexpr uint32_t section_count = 4;

			constexpr uint32_t size_of_size = 8;
			constexpr uint32_t size_of_offset = 8;
			constexpr uint32_t size_of_id = 8;
			constexpr uint32_t size_of_module_id = 2;
			constexpr uint32_t size_of_section_data = 1 + size_of_offset + size_of_size;	//section_type (1 byte),offset(8 bytes), size (8 bytes)

			constexpr uint32_t size_of_constant_field_header = size_of_id + size_of_size;	//data can be infinity 
			constexpr uint32_t size_of_symbol_field = size_of_id + size_of_offset + size_of_module_id + sizeof(SymbolType);//19 bytes
			constexpr uint32_t command_size = 40;	//It's possible to use less, but the packaging will be more complicated.

			constexpr std::string_view module_prefix = "mslmodule";
			constexpr std::string_view module_extension = "mslco";
			constexpr std::string_view main_extension = "mslmo";

			constexpr std::string_view mo_magic = "MSLMO";
			constexpr std::string_view co_magic = "MSLCO";
			constexpr float FilesVersion = 1.0f;
		}

		enum SectionType : uint8_t
		{
			LINKING_ORDER = 14,
			CONSTANTS,
			SYMBOLS,
			DEBUG,

		};

		struct static_bpointer
		{
			char* ptr = nullptr;
			size_t bytes_size = 0;

			static_bpointer() = default;

			explicit static_bpointer(size_t bytes) {
				allocate(bytes);
			}

			bool allocate(size_t bytes) {
				ptr = static_cast<char*>(calloc(bytes, 1));
				if (!ptr) return false;
				bytes_size = bytes;
				return true;
			}

			void release() {
				if (ptr) ::free(ptr);
				ptr = nullptr;
				bytes_size = 0;
			}

			bool is_valid() { return ptr != nullptr && bytes_size != 0; }
		};


		struct ConstantData 
		{
			constantid id;
			size_t size_in_bytes;
			union 
			{
				char* data = nullptr;
				size_t* uint64_ptr_interpretation;
			};
			
			size_t memory_offset;
			void Free() 
			{
				if (data) free(data);
				data = nullptr;
			}
			void InitBy(char* source, size_t bytes_count) 
			{
				if (data != nullptr) Free();

				data = static_cast<char*>(calloc(bytes_count,1));
				memcpy(data, source, bytes_count);
				size_in_bytes = bytes_count;
			}
		};

		struct SymbolData 
		{
			symbolid id;
			//offset memory code
			size_t offset_m_c;//For variable its memory address, for function - code. 
			size_t native_code_offset; //Code address in target representation (SYMBOL_LABEL)
			moduleid module_id;
			
			SymbolType type;

			SymbolData(symbolid id,size_t ir_cbo, moduleid module_id, SymbolType type):id(id), offset_m_c(ir_cbo),module_id(module_id),type(type){}
		};


		struct LinkingState 
		{
		public:
			std::vector<ConstantData> constants;
			size_t constants_size = 0;

			std::vector<SymbolData> symbols;
			std::vector<moduleid> linking_order;

			std::unordered_map<moduleid, size_t> stack_offset_of_module;

			size_t global_memory_offset = 0;

			size_t rod_offset_aligned = 0;		//Saved in executable file

			uint16_t compilation_flags;
			float version = 1.0f;

			std::string module_prefix;
			std::string module_extention;


			void FreeConstants()
			{
				for (auto& constant : constants) 
				{
					constant.Free();
				}
				constants.clear();
			}



		};


		struct CommandsPool 
		{
			std::vector<ByteCommand> commands;
			float version = MSLOData::FilesVersion;
			uint16_t compilation_flags = 0;

			size_t code_size_in_bytes = 0;
			size_t stack_size = 0;

			moduleid m_id = UINT16_MAX;

		};

		enum class VMs : uint8_t
		{
			INVALID = 0,
			MSLVM_1 = 1,
		};

		struct ExecutionData 
		{
			static_bpointer read_only_data = {};
			static_bpointer code = {};

			size_t aligned_rod_size = 0;

			VMs vm_type = VMs::MSLVM_1;

			void Free() 
			{
				read_only_data.release();
				code.release();
			}
		};

		

	}
}