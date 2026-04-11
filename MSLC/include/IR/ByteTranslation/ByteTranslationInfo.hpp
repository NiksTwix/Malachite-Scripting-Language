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

			constexpr size_t LOGIC_RESULT_SIZE = 1;
			constexpr size_t POINTER_SIZE = 8;
			using PrimitiveAnalogs = CompilationInfo::Types::PrimitiveAnalogs;
			#pragma region CommandsInfo
			//can be copied to linker's header
			enum CommandSource : uint8_t {
				Register = 1,        // register (index), can contain a dynamic memory address
				MemoryAddress,   // address in memory (offset/static)
				Immediate,       // immediated value
				Constant,         // index in the constants poop
				Symbol,				//for linking with symbols table
			};

			enum SpecialRegisterID : uint8_t
			{
				SP = 0,    // Stack Pointer
				FP,    // Frame Pointer
				IP,    // Instruction Pointer
				FL,    // Flags
				RT,     // Return Value Temporary
				// Could be added in the future:
				// ZERO,    // Always 0 (as in RISC-V)
				// RA,      // Return Address
				// GP       // Global Pointer
				EnumSize,		///not is a register. Enum size
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
				bool declaring_place = true;       // Save debug information

				//// Target platform
				//enum Target {
				//	MSLVM_V1,      // Basic VM (32-byte commands)
				//	MSLVM_COMPACT, // Little VM (8-byte) (in the future)
				//	MSLVM_FAST,    // Fast VM (optimized) (in the future)
				//	WASM,          // WebAssembly for browser (in the future)
				//	NATIVE_X64,    // Native code x86_64  (in the future)
				//} target = MSLVM_V1;
			};

			enum CompilationFlag: uint16_t
			{
				None,
				LocalOffsetsStart,
				LocalOffsetsEnd,
			};

			enum class ByteOpCode : uint8_t
			{
				NOP = 0,

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
				SECTION_LOGIC_ED,

				//Comparing
				SECTION_COMPARING_ST,
				EQUALI,			// destination|source0|source1
				NOT_EQUALI,		// destination|source0|source1
				GREATERI,		// destination|source0|source1
				LESSI,			// destination|source0|source1
				EGREATERI,		// destination|source0|source1
				ELESSI,			// destination|source0|source1

				EQUALU,			// destination|source0|source1
				NOT_EQUALU,		// destination|source0|source1
				GREATERU,		// destination|source0|source1
				LESSU,			// destination|source0|source1
				EGREATERU,		// destination|source0|source1
				ELESSU,			// destination|source0|source1

				EQUALR,			// destination|source0|source1
				NOT_EQUALR,		// destination|source0|source1
				GREATERR,		// destination|source0|source1
				LESSR,			// destination|source0|source1
				EGREATERR,		// destination|source0|source1
				ELESSR,			// destination|source0|source1
				SECTION_COMPARING_ED,

				
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

				LOAD_DYNAMIC,	//Loading by address in register //Register With Loading Address[SRC],Register[DST],Size
				STORE_DYNAMIC,	//Storing by address in register //Register With Storing Address[DST],Register[SRC],Size

				LOAD_CONST_STATIC,	//load constant value to register. Linker will replace id on offset| Constant Register Size
				LEA_CONST,	//Load constant pointer in register. ConstantID, register

				LOAD_STATIC,	//Loading by address in register //MemoryAddress,Register,Size
				STORE_STATIC,	//Storing by address in register //MemoryAddress,Register,Size

				ALLOC,		//register for allocation's pointer, size in bytes, reset memory (1 or 0). If is third arg is null - automaticaly sets to 1
				FREE,	  //register with pointer, register with size

				COPY_DYNAMIC,		//Dynamic copy. Reg0 - address-dest, Reg1 - address-src, Reg2 - size
				COPY_STATIC,		//Static copy. Immediate0 - address-dest, Immediate1 - address-src, Immediate2 - size

				SECTION_MEMORY_ED,
				// Control flow arg0 = where
				SECTION_CONTROL_FLOW_ST,
				JMP_LABEL,	//arg0[where]
				JMP,		//arg0[where]
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

				MOV_FROM_SR,	//Move from special register. REG-DEST, SPECIAL_REG_ID-SRC
				SPEC_CALL,	//VM_CALL/SYSTEM_CALL CALL_TYPE, arg1, arg2

				SECTION_SPECIAL_ED,
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
				ByteCommand(ByteOpCode c, CommandArgument a0) : code(c), arg0(a0), arg1(0,CommandSource::Immediate), arg2(0, CommandSource::Immediate) {}
				ByteCommand(ByteOpCode c, CommandArgument a0, CommandArgument a1)
					: code(c), arg0(a0), arg1(a1), arg2(0, CommandSource::Immediate) {
				}
				ByteCommand(ByteOpCode c, CommandArgument a0, CommandArgument a1, CommandArgument a2)
					: code(c), arg0(a0), arg1(a1), arg2(a2) {
				}
				ByteCommand(ByteOpCode c, CommandArgument a0, CommandArgument a1, CommandArgument a2, uint32_t flags)
					: code(c), arg0(a0), arg1(a1), arg2(a2),flags(flags) {
				}
				ByteCommand(): code(ByteOpCode::NOP), arg0(0, CommandSource::Immediate), arg1(0, CommandSource::Immediate), arg2(0, CommandSource::Immediate) {}
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

				size_t AllocateFreeGeneral() {
					for (size_t i = 0; i < general_count; ++i) {
						if (!IsUsed(i))
						{
							SetUsed(i);
							return i;
						}
					}
					return InvalidRegister;
				}

				//size_t FindFreeAccumulator() const {
				//	for (size_t i = general_count; i < general_count + accum_count; ++i) {
				//		if (!IsUsed(i)) return i;
				//	}
				//	return InvalidRegister;
				//}

				//CommandSource GetRegType(size_t index) const {
				//	if (index < general_count) return RGeneral;
				//	if (index < general_count + accum_count) return RAccumulator;
				//	return RSpecific;  
				//}
			};

			using PrimitiveAnalogs = CompilationInfo::Types::PrimitiveAnalogs;

			enum class ValueSource {
				Register,		//Register (General + Accumulator + Special)
				Pointer,       // Pointer in register (data is register index)
				StaticAddress,		//static memory address 
				DynamicAddress,		//address got by dereference (data is register index)
				Constant,      // Reference in constant pool
				Symbol,        // Symbol link (for linking)
				Immediate,		//Immediate value (e.g size)
			};

			struct ValueFrame
			{	
				//ValueType value_type = ValueType::Pointer;
				ValueSource source;
				size_t data;

				//Dynamic - can be edited by operations

				//Costil eto (dynamic/static) - costil. We need to replace it on more cool type model in the future 

				PrimitiveAnalogs dynamic_primitive_type = PrimitiveAnalogs::UInt;
				size_t dynamic_data_size = 0;		// Size of value: pointer, simple data and another

				//Static are constant
				PrimitiveAnalogs static_primitive_type = PrimitiveAnalogs::UInt;
				size_t static_data_size = 0;		// Size of typed value

				CompilationInfo::Values::ValueInfo value_info;	//dynamic

				CompilationInfo::CompilationState* cs_observer;

				ValueFrame(ValueSource source,size_t data, PrimitiveAnalogs type, size_t size, CompilationInfo::CompilationState* state) : source(source), data(data), dynamic_primitive_type(type), dynamic_data_size(size), static_primitive_type(type), static_data_size(size), cs_observer(state) {}

				static ValueFrame Invalid()
				{
					return ValueFrame(ValueSource::Immediate,0, PrimitiveAnalogs::UInt, 0, nullptr);
				}
				void SetValueInfo(CompilationInfo::Values::ValueInfo vinfo)
				{
					value_info = vinfo;
					if (vinfo.isPointer()) 
					{
						static_primitive_type = PrimitiveAnalogs::UInt;
						dynamic_primitive_type = PrimitiveAnalogs::UInt;
						static_data_size = POINTER_SIZE;
						dynamic_data_size = POINTER_SIZE;
					}
					else 
					{
						auto desc = GetTypeDesc(vinfo.type_id, cs_observer);
						dynamic_primitive_type = desc->primitive_analog;
						static_primitive_type = desc->primitive_analog;
						static_data_size = desc->GetAlignedSize();
						dynamic_data_size = desc->GetAlignedSize();
					}
				}

				void Dereference()
				{
					if (value_info.pointers_depth > 0) value_info.pointers_depth--;
					if (value_info.pointers_depth == 0)
					{
						value_info.flags &= ~CompilationInfo::Values::ValueFlags::Pointer;
						value_info.flags &= ~CompilationInfo::Values::ValueFlags::ConstPointer;
						source = ValueSource::DynamicAddress;
						dynamic_primitive_type = GetTypeDesc(value_info.type_id, cs_observer)->primitive_analog;
						dynamic_data_size = GetTypeDesc(value_info.type_id, cs_observer)->GetAlignedSize();
					}

				}
				void GetPointer() 
				{
					value_info.pointers_depth++;
					value_info.flags |= CompilationInfo::Values::ValueFlags::Pointer;
					source = ValueSource::Pointer;
					dynamic_primitive_type = PrimitiveAnalogs::UInt;
					dynamic_data_size = POINTER_SIZE;
				}
				static CompilationInfo::Types::TypeDescription* GetTypeDesc(CompilationInfo::Types::TypeID id, CompilationInfo::CompilationState* state) 
				{
					return state->GetGST().GetType(id);
				}




			};

			using BCommandsArray = Definitions::ChunkArray<ByteCommand>;

			struct StackFrame 
			{
				size_t start_address = 0;
				size_t size = 0;
				bool local_offsets = false;
				StackFrame(size_t start_address): start_address(start_address) {}
				size_t NextFreeAddress() { return start_address + size; }
			};



			struct ByteTranslationState
			{

			public:

				uint64_t pseudo_ip = 0;
				RegistersTable registers_table;
				int64_t current_depth = 0; //Program starts by OpenVisibleScope and ends by CloseVisibleScope, but we need start depth = 0
				std::stack<ValueFrame> value_stack{};         //Stack for operations
				std::stack<StackFrame> frame_stack{};    //When we create variable add it size to frame_size stack;

				Diagnostics::DeclaringPlace declaring_place;
				BCommandsArray result;

				ByteTranslationConfig config;

				std::shared_ptr<Pseudo::PseudoTranslationState> pseudo_state_observer;

				
				CompilationInfo::CompilationState* cs_observer;

				std::unordered_map<size_t, bool> dvicm;//declared_variables_in_current_module


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
				inline void PushFrame(StackFrame frame)
				{
					frame_stack.push(frame);

				}
				inline bool PopFrame() 
				{
					if (frame_stack.empty()) return false;
					else frame_stack.pop();
					return true;
				}

				Diagnostics::DeclaringPlace GetDeclaringPlace() 
				{
					return declaring_place;
				}

			};
		}
	}
}