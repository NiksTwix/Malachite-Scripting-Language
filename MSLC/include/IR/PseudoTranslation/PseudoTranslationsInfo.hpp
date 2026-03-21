#pragma once
#include "..\..\Definitions\ValueContainer.hpp"
#include <unordered_map> 

#include "..\..\Definitions\ChunkArray.hpp"
#include "..\..\CompilationInfo\CompilationState.hpp"
#include "..\..\AST\ASTBuilder.hpp"

namespace MSLC 
{
	namespace IntermediateRepresentation
	{




		namespace Pseudo
		{
			enum class PseudoOpCode : uint16_t
			{
				Nop,

				//Arythmetic
				ST_AL,
				Add,
				Subtract,
				Divide,
				Multiply,
				Mod,
				Negative,

				PrefixIncrement,
				PrefixDecrement,
				PostfixDecrement,
				PostfixIncrement,


				Exponentiate,

				BitOffsetRight,
				BitOffsetLeft,

				Greater,
				Less,
				GreaterEqual,
				LessEqual,
				Equal,
				NotEqual,

				And,
				Or,
				Not,
				BitAnd,
				BitOr,
				BitNot,

				ED_AL,

				ST_MEM,
				Assign,		//arg0 = op_code of complex assignment (as example += - Add)
				AssignR,	//R - returns. Accumulate value of assignment in register, also pushs in value_stack (of ByteTranslationState) Value(OperationResult,reg_number)

				Push,
				Pop,
				//Work with pointers
				GetPointer,
				Dereference,

				GetFieldByValue,
				GetFieldByPointer,


				ED_MEM,

				//Logic



				ST_DECL,
				DeclareVariable,	//arg - symbol
				DefineFunction,
				ED_DEC,

				ST_USING,
				Use,			//variable
				UseConstant,	//constant_id
				UseType,
				UseField,
				ED_USING,
				Call,
				CreateArray,	//count of previous elements in stack

				PushFrame,
				PopFrame,
			};


			enum PseudoOperationFlags : uint8_t
			{
				None = 0,
				Binary = 1 << 0,
				Unary = 1 << 1,
				Assignment = 1 << 2,
			};

			struct PseudoOperation
			{
				size_t arg_0;
				size_t arg_1;
				size_t arg_2;
				uint32_t debug_line;
				PseudoOpCode op_code;
				uint8_t flags;

				PseudoOperation(PseudoOpCode code, size_t arg0, size_t arg1, size_t arg2, uint32_t line, uint8_t flags) : arg_0(arg0), arg_1(arg1), arg_2(arg2), debug_line(line), op_code(code), flags(flags)
				{
				}
				PseudoOperation(PseudoOpCode code, size_t arg0, size_t arg1, size_t arg2, uint32_t line) : arg_0(arg0), arg_1(arg1), arg_2(arg2), debug_line(line), op_code(code), flags(PseudoOperationFlags::None)
				{
				}
				PseudoOperation(PseudoOpCode code, size_t arg0, size_t arg1,  uint32_t line) : arg_0(arg0), arg_1(arg1), arg_2(0), debug_line(line), op_code(code), flags(PseudoOperationFlags::None)
				{
				}
				PseudoOperation(PseudoOpCode code, size_t arg0, uint32_t line) : arg_0(arg0), arg_1(0), arg_2(0), debug_line(line), op_code(code), flags(PseudoOperationFlags::None)
				{
				}
				PseudoOperation(PseudoOpCode code, uint32_t line) : arg_0(0), arg_1(0), arg_2(0), debug_line(line), op_code(code), flags(PseudoOperationFlags::None)
				{
				}
				PseudoOperation() = default;
			};

			using POperationArray = Definitions::ChunkArray<PseudoOperation>;

			

			struct PseudoTranslationState 
			{
				POperationArray pseudo_code = POperationArray();
				CompilationInfo::CompilationState* cs_observer = nullptr;
			};




			struct OperatorInfo 
			{
				enum class Type { Undefined, Unary,Binary,Meta, Assignment,Declaration, FieldAccess };
				enum class Associativity 
				{
					LeftRight,	// +,/,*, and etc
					RightLeft	//assignments
				};	
				int8_t priority;
				Type type;
				PseudoOpCode op_code;
				Associativity associativity = Associativity::LeftRight;
				OperatorInfo() : priority(-128), type(Type::Undefined), op_code(PseudoOpCode::Nop) {}
				OperatorInfo(int8_t priority, Type type, PseudoOpCode op_code) : priority(priority), type(type), op_code(op_code) {}
				OperatorInfo(int8_t priority, Type type, PseudoOpCode op_code, Associativity associativity) : priority(priority), type(type), op_code(op_code), associativity(associativity){}
				OperatorInfo(int8_t priority, Type type) : priority(priority), type(type), op_code(PseudoOpCode::Nop) {}
			};

			class OperatorsTable 
			{
				std::unordered_map<std::string, OperatorInfo> m_operators = 
				{
					//Level 12: data access and declaration
					{"::", OperatorInfo(12,OperatorInfo::Type::Binary, PseudoOpCode::Nop)},//Access to the namespace/static field/method
					
					{":", OperatorInfo(12,OperatorInfo::Type::Declaration, PseudoOpCode::Nop)},//Declaration of variable

					{".", OperatorInfo(12,OperatorInfo::Type::Binary, PseudoOpCode::GetFieldByValue)}, {"->",OperatorInfo(12,OperatorInfo::Type::Binary, PseudoOpCode::GetFieldByPointer)},

					// Level 11: Meta
					{std::string(Keywords::w_new), OperatorInfo(11,OperatorInfo::Type::Meta)},
					{std::string(Keywords::w_to),OperatorInfo(11,OperatorInfo::Type::Meta)},

					// Level 10: branchs (separately handling)
					{"(", OperatorInfo(-1,OperatorInfo::Type::Undefined)}, {")",OperatorInfo(-1,OperatorInfo::Type::Undefined)},{"[",OperatorInfo(-1,OperatorInfo::Type::Undefined)}, {"]", OperatorInfo(-1,OperatorInfo::Type::Undefined)},  // special cases

					// Level 9: unary operators
					{"!u", OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::Not)}, {"~u",OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::BitNot)},  {"-u",OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::Negative)},  // unary + and -, also bit NOT/ u-unary {"+u", OperatorInfo(8,OperatorInfo::Type::Unary, PseudoOpCode::Not)},

					{"*u", OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::Dereference)},{"&u",OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::GetPointer)},   //Work with pointers
					{"--u", OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::PrefixDecrement)},
					{"++u", OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::PrefixIncrement)},
					{"u++", OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::PostfixIncrement)},
					{"u--", OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::PostfixDecrement)},
					// Level 9: Exponetiate

					{"^", OperatorInfo(8,OperatorInfo::Type::Binary,PseudoOpCode::Exponentiate)},

					// Level 7: multiplicative
					
					{"*", OperatorInfo(7,OperatorInfo::Type::Binary, PseudoOpCode::Multiply)}, {"/",OperatorInfo(7,OperatorInfo::Type::Binary, PseudoOpCode::Divide)}, {"%", OperatorInfo(7,OperatorInfo::Type::Binary, PseudoOpCode::Mod)},

					// Level 6: additive 
					{"+", OperatorInfo(6,OperatorInfo::Type::Binary, PseudoOpCode::Add)}, {"-", OperatorInfo(6,OperatorInfo::Type::Binary, PseudoOpCode::Subtract)},            // binary + and -

					// Level 5

					
					// Level 4: bit offset

					{"<<", OperatorInfo(4,OperatorInfo::Type::Binary,PseudoOpCode::BitOffsetLeft)}, {">>", OperatorInfo(4,OperatorInfo::Type::Binary,PseudoOpCode::BitOffsetRight)},

					// Level 3: comparings
					{"<", OperatorInfo(3,OperatorInfo::Type::Binary,PseudoOpCode::Less)}, {"<=", OperatorInfo(3,OperatorInfo::Type::Binary,PseudoOpCode::LessEqual)}, {">", OperatorInfo(3,OperatorInfo::Type::Binary,PseudoOpCode::Greater)}, {">=", OperatorInfo(3,OperatorInfo::Type::Binary,PseudoOpCode::GreaterEqual)},

					// Level 2: equalities
					{"==",OperatorInfo(2,OperatorInfo::Type::Binary,PseudoOpCode::Equal)}, {"!=", OperatorInfo(2,OperatorInfo::Type::Binary,PseudoOpCode::NotEqual)},

					// Level 1: bit OR/XOR
					{"&", OperatorInfo(1,OperatorInfo::Type::Binary,PseudoOpCode::BitAnd)}, {"|", OperatorInfo(1,OperatorInfo::Type::Binary,PseudoOpCode::BitOr)}, 

					// Level 0: logic AND/OR
					{"&&", OperatorInfo(0,OperatorInfo::Type::Binary,PseudoOpCode::And)}, {"||", OperatorInfo(0,OperatorInfo::Type::Binary,PseudoOpCode::Or)},

					// Level -1: assignments (the lowest priority)
					{"=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Nop, OperatorInfo::Associativity::RightLeft)}, 
					{"+=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Add, OperatorInfo::Associativity::RightLeft)},
					{"-=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Subtract, OperatorInfo::Associativity::RightLeft)},
					{"*=",OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Multiply, OperatorInfo::Associativity::RightLeft)},
					{"/=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Divide, OperatorInfo::Associativity::RightLeft)}, 
					{"%=",OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Mod, OperatorInfo::Associativity::RightLeft)},
					{"&=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::BitAnd, OperatorInfo::Associativity::RightLeft)},
					{"|=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::BitOr, OperatorInfo::Associativity::RightLeft)},
					{"<<=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::BitOffsetLeft, OperatorInfo::Associativity::RightLeft)},
					{">>=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::BitOffsetRight, OperatorInfo::Associativity::RightLeft)},
					{"&&=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::And, OperatorInfo::Associativity::RightLeft)},
					{"||=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Or, OperatorInfo::Associativity::RightLeft)},
					{"~=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::BitNot, OperatorInfo::Associativity::RightLeft)},
					{"^=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Exponentiate, OperatorInfo::Associativity::RightLeft)}
				
				};

			public:
				OperatorInfo GetInfo(const std::string& operator_token) 
				{
					if (m_operators.count(operator_token)) return m_operators[operator_token];
					else return OperatorInfo();
				}
				bool Has(const std::string& operator_token) { return m_operators.count(operator_token); }

				static OperatorsTable& Get() 
				{
					static OperatorsTable table;
					return table;
				}

			};

			using recursive_handler = std::function<void(AST::ASTNode&, PseudoTranslationState&)>;
		}
	}
}