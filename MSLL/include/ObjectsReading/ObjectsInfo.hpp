#pragma once
#include <cstdint>
#include <vector>
namespace MSLL 
{
	namespace ObjectsInfo 
	{
		using fileid = uint16_t;
		using constantid = size_t;
		using symbolid = size_t;

		enum CommandSource : uint8_t {
			Register = 1,        // register (index)
			MemoryAddress,   // address in memory (offset)
			Immediate,       // immediated value
			Constant,         // index in the constants poop
			Symbol,				//for linking with symbols table
		};


		enum class ByteOpCode : uint8_t
		{
			NOP,

			COMP_FLG,	//CompilationFlag arg0  - CompilationFlag. In functions declarations

			SECTION_ARITHMETIC_ST,
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
			PUSH,
			POP,

			LEA_STATIC,		// Static address calculating: MemoryAddress (static) -> reg 
			LEA_DYNAMIC,	// Dynamic address calculating: register (with address) -> register_dest

			LOAD_DYNAMIC,	//Loading by address in register
			STORE_DYNAMIC,	//Storing by address in register

			LOAD_CONST_STATIC,	//load constant value to register. Linker will replace id on offset
			LOAD_CONST_DYNAMIC,	//load constant offset to register. Linker will replace id on offset

			LOAD_STATIC,	//Loading by address in register
			STORE_STATIC,	//Storing by address in register

			SECTION_MEMORY_ED,
			// Control flow arg0 = where
			SECTION_CONTROL_FLOW_ST,
			JMP,
			JMPCV,      //CV- Condition Valid - arg0[where], arg1[condition register]
			JMPCNV,     //CNV - Condition Not Valid - arg0[where], arg1[condition register]
			CALL,
			RET,
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
			uint32_t pseudo_op_index = 0;  // Reference of pseudo command

			// For linking
			uint32_t relocation_index = 0xFFFFFFFF;  // Index in the realocation table

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

		struct Constant 
		{
			constantid id;
			size_t data_size;
			char* data;
			
			void Free() 
			{
				delete data;
			}
		};

		struct Symbol 
		{
			symbolid id;
			fileid moduleid;
			size_t ir_code_base_offset;
			SymbolType type;
		};


		struct LinkingState 
		{
			std::vector<Constant> constants;
			std::vector<Symbol> symbols;



			void FreeConstants()
			{
				for (auto& constant : constants) 
				{
					constant.Free();
				}
				constants.clear();
			}
		};

	}
}