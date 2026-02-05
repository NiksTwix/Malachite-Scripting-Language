#pragma once
#include <vector>
#include <stack>
#include "..\..\Definitions\ChunkArray.hpp"
#include "..\PseudoTranslation\PseudoTranslationsInfo.hpp"
namespace MSLC 
{
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			#pragma region CommandsInfo
			//can be copied to linker's header
			enum CommandSource : uint8_t {
				RGeneral,      // General register (0-119)
				RSpecific,     // Specific register (SP, FP, IP...)
				RAccumulator,  // Accumulated (for immediate/often used)
				Address,       // Relative from start address 
				Immediate,     // Immediate value 
				Constant,      // Reference in constant pool
				Symbol,        // Symbol link (for linking)
				TypeID         // ID of type (for typed operations)
			};

			enum SpecialRegisterID : uint8_t
			{
				SP = 0,    // Stack Pointer
				FP,    // Frame Pointer
				IP,    // Instruction Pointer
				FL,    // Flags
				RT,     // Return Value Temporary
				// Could be added in the future:
				// ZERO,    // Всегда 0 (как в RISC-V)
				// RA,      // Return Address
				// GP       // Global Pointer
				EnumSize,		///not is a register. Enum size
			};

			struct CommandArgument {
				union {
					size_t memory_addr;    // For Address
					uint64_t immediate;    // For Immediate
					uint32_t const_index;  // For Constant
					uint32_t symbol_id;    // For Symbol
					uint32_t type_id;      // For TypeID
					struct {
						uint16_t reg_index;
						uint8_t reg_type;  // 0=general, 1=specific, 2=accumulator
					};
				};

				CommandSource type;

				// For comfort
				bool is_register() const {
					return type == RGeneral || type == RSpecific || type == RAccumulator;
				}
				bool is_memory() const { return type == Address; }
				bool is_immediate() const { return type == Immediate; }
			};

			struct ByteTranslationConfig {	//In the declaring time used standart values
				// Registers
				uint16_t general_registers_count = 120;
				uint16_t special_registers_count = 8;
				uint16_t accumulation_registers_count = 3;

				// Memory
				size_t stack_size = 1024 * 32;		// 32KB стек
				size_t heap_size = 1024 * 32;		// 32KB  heap
				size_t const_pool_size = 65536;		// 64KB для констант

				// Flags
				bool isAoT = false;				// AOT compiling in the future
				bool optimize = false;         // Optimization is enable
				bool debug_info = true;       // Save debug information

				// Target platform
				enum Target {
					MSLVM_V1,      // Basic VM (32-byte commands)
					MSLVM_COMPACT, // Little VM (8-byte)
					MSLVM_FAST,    // Fase VM (optimized)
					WASM,          // WebAssembly for browser (in the future)
					NATIVE_X64,    // Native code x86_64  (in the future)
				} target = MSLVM_V1;
			};

			enum ByteOpCode : uint8_t
			{
				ADDR,
				SUBR,
				DIVR,
				MULR,
				NEGR,

				ADDI,
				SUBI,
				DIVI,
				MULI,
				NEGI,
				MODI,

				ADDU,
				SUBU,
				DIVU,
				MULU,
				MODU,

				//Logic

				AND,
				OR,
				NOT,
				BIT_OR,
				BIT_NOT,
				BIT_AND,
				BIT_OFFSET_LEFT,
				BIT_OFFSET_RIGHT,

				CMPI,         
				CMPU,         
				CMPR,   

				MOVRR,
				MOVRI,
				PUSH,              
				POP,               
				LOAD_LOCAL,     
				STORE_LOCAL,    

				LOAD_CONST,
				MEMCP,		//Memory copy

				
				LOAD_BY_ADDRESS,			// register(ARG0), register - address(ARG1), size(ARG2)
				STORE_BY_ADDRESS,			// register(ARG0), register - address(ARG1), size(ARG2)

				ALLOCATE_MEMORY,		//arg0[register of address's saving], arg1[register with size of memory's interval]
				FREE_MEMORY,			//arg0[register with address], arg1[register with size of memory's interval]
				GRAB_FRAME,				//arg0[bytes] If bytes < 0 => grab to up, else - grab to down 
				// Control flow arg0 = where
				JMP,
				JMPCV,      //CV- Condition Valid - arg0[where], arg1[condition register]
				JMPCNV,     //CNV - Condition Not Valid - arg0[where], arg1[condition register]
				CALL,
				RET,
				EXT,	//EXIT

				// Type Convertion
				TC_ITR,    //Type Convertion Integer To Real
				TC_RTI,    //Type Convertion Real To Integer

				TC_UTR,    //Type Convertion Unsigned Integer To Real
				TC_UTI,    //Type Convertion Unsigned Integer To Integer

				TC_RTU,    //Type Convertion Real To Unsigned Integer
				TC_ITU,
			};

			struct ByteCommand {
				ByteOpCode code;
				CommandArgument arg0;
				CommandArgument arg1;
				CommandArgument arg2;

				// Meta
				uint32_t flags = 0;
				enum Flag {
					Volatile = 1 << 0,   // Command has side effects
					branch = 1 << 1,   //	Jump Command
					Call = 1 << 2,   // Calling command
					Return = 1 << 3,   // Return command
					Memory = 1 << 4,   // Memory 
					Syscall = 1 << 5,   // System call
					LocalOffset = 1 << 6,   //Flag "Local offset" used in function bodies in order to linker disable offset autocorrect
					UnhandledSymbol = 1 << 7,	//Linker must search and handle symbol in realocation table
				};

				// Debug_info
				uint32_t source_line = 0;
				uint32_t pseudo_op_index = 0;  // Reference of pseudo command

				// For linking
				uint32_t relocation_index = 0xFFFFFFFF;  // Index in the realocation table

				// Constructors for comfort
				ByteCommand(ByteOpCode c) : code(c) {}
				ByteCommand(ByteOpCode c, CommandArgument a0) : code(c), arg0(a0) {}
				ByteCommand(ByteOpCode c, CommandArgument a0, CommandArgument a1)
					: code(c), arg0(a0), arg1(a1) {
				}
				ByteCommand(ByteOpCode c, CommandArgument a0, CommandArgument a1, CommandArgument a2)
					: code(c), arg0(a0), arg1(a1), arg2(a2) {
				}
			};

			#pragma endregion


			constexpr size_t InvalidRegister = SIZE_MAX;
			struct RegistersTable {
			private:
				uint8_t* bitmap;
				size_t total_regs;
				size_t general_count;
				size_t accum_count;
				size_t special_count;

			public:
				RegistersTable(size_t general_regs, size_t accum_regs)
					: general_count(general_regs),
					accum_count(accum_regs),
					special_count(SpecialRegisterID::EnumSize)
				{
					total_regs = general_count + accum_count + special_count;
					size_t bytes = (total_regs + 7) / 8;
					bitmap = static_cast<uint8_t*>(calloc(bytes, 1));
					if (!bitmap) {
						total_regs = 0;
					}
				}

				~RegistersTable() {
					if (bitmap) free(bitmap);
				}

				RegistersTable(const RegistersTable&) = delete;
				RegistersTable& operator=(const RegistersTable&) = delete;
				RegistersTable(RegistersTable&&) = delete;
				RegistersTable& operator=(RegistersTable&&) = delete;



				bool IsUsed(size_t index) const {
					if (index >= total_regs || !bitmap) return true;
					size_t byte_idx = index / 8;
					size_t bit_idx = index % 8;
					return (bitmap[byte_idx] >> bit_idx) & 1;
				}
				void SetUsed(size_t index) {
					if (index >= total_regs || !bitmap) return;
					size_t byte_idx = index / 8;
					size_t bit_idx = index % 8;
					bitmap[byte_idx] |= (1 << bit_idx);
				}

				void SetFree(size_t index) {
					if (index >= total_regs || !bitmap) return;
					size_t byte_idx = index / 8;
					size_t bit_idx = index % 8;
					bitmap[byte_idx] &= ~(1 << bit_idx);
				}

				void ClearAll() {
					if (!bitmap) return;
					size_t bytes = (total_regs + 7) / 8;
					memset(bitmap, 0, bytes);
				}

				size_t FindFreeGeneral() const {
					for (size_t i = 0; i < general_count; ++i) {
						if (!IsUsed(i)) return i;
					}
					return InvalidRegister;
				}

				size_t FindFreeAccumulator() const {
					for (size_t i = general_count; i < general_count + accum_count; ++i) {
						if (!IsUsed(i)) return i;
					}
					return InvalidRegister;
				}

				CommandSource GetRegType(size_t index) const {
					if (index < general_count) return RGeneral;
					if (index < general_count + accum_count) return RAccumulator;
					return RSpecific;  
				}
			};


			struct ValueFrame
			{
				//enum ValueType { Pointer, };
				enum ValueNativeType { UInt, Int, Real };	
				union 
				{
					size_t used_register;
					size_t pointer;
				};
				//ValueType value_type = ValueType::Pointer;
				ValueNativeType native_type = ValueNativeType::UInt;
			};

			using BCommandsArray = Definitions::ChunkArray<ByteCommand>;

			struct StackFrame 
			{
				size_t start_address = 0;
				size_t size = 0;
				StackFrame(size_t start_address): start_address(start_address) {}
			};


			struct ByteTranslationState
			{
			private:
				size_t global_us_id = 0;
			public:
				uint64_t pseudo_ip = 0;
				RegistersTable registers_table;
				int64_t current_depth = 0; //Program starts by OpenVisibleScope and ends by CloseVisibleScope, but we need start depth = 0
				std::stack<ValueFrame> value_stack{};         //Stack for operations
				std::stack<StackFrame> frame_stack{};    //When we create variable add it size to frame_size stack;

				BCommandsArray result;

				ByteTranslationConfig config;

				CompilationInfo::CompilationState* cs_observer;

				ByteTranslationState(ByteTranslationConfig& config, CompilationInfo::CompilationState* c_state) : registers_table(config.general_registers_count,config.accumulation_registers_count),cs_observer(c_state)
				{
					this->config = config;
					result = BCommandsArray();
					PushFrame();
				}
				inline void PushFrame()
				{
					if (frame_stack.empty())frame_stack.push(StackFrame(0));
					else 
					{
						auto& frame = frame_stack.top();
						frame_stack.push(StackFrame(frame.start_address + frame.size));
					}
				}
				inline bool PopFrame() 
				{
					if (frame_stack.empty()) return false;
					else frame_stack.pop();
				}
			};
		}
	}
}