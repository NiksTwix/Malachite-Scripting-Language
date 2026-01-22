#pragma once
#include "..\..\Definitions\ValueContainer.hpp"
#include <unordered_map> 

#include "..\..\Definitions\ChunkArray.hpp"
#include "..\..\CompilationInfo\CompilationState.hpp"


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
				Add,
				Subtract,
				Divide,
				Multiply,
				Mod,
				Negative,

				Exponentiate,

				BitOffsetRight,
				BitOffsetLeft,

				//Work with pointers
				GetPointer,
				GetValue,

				GetFieldByValue,
				GetFieldByPointer,

				//Logic
				
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
				
			};

			struct PseudoOperation
			{
				size_t arg_0;
				size_t arg_1;
				size_t arg_2;
				int debug_line;
				PseudoOpCode op_code;
				uint8_t flags;
			};

			using POperationArray = Definitions::ChunkArray<PseudoOperation>;

			

			struct PseudoTranslationState 
			{
				POperationArray pseudo_code{};
				CompilationInfo::CompilationState* cs_observer = nullptr;
			};




			struct OperatorInfo 
			{
				enum class Type { Undefined, Unary,Binary,Meta, Assignment };
				int8_t priority;
				Type type;
				PseudoOpCode op_code;
				OperatorInfo() : priority(-128), type(Type::Undefined), op_code(PseudoOpCode::Nop) {}
				OperatorInfo(int8_t priority, Type type, PseudoOpCode op_code) : priority(priority), type(type), op_code(op_code) {}
				OperatorInfo(int8_t priority, Type type) : priority(priority), type(type), op_code(PseudoOpCode::Nop) {}
			};

			class OperatorsTable 
			{
				std::unordered_map<std::string, OperatorInfo> m_operators = 
				{
					//Level 12: data access
					{".", OperatorInfo(12,OperatorInfo::Type::Binary, PseudoOpCode::GetFieldByValue)}, {"->",OperatorInfo(12,OperatorInfo::Type::Binary, PseudoOpCode::GetFieldByPointer)},

					// Level 11: Meta
					{std::string(Keywords::w_new), OperatorInfo(11,OperatorInfo::Type::Meta)},
					{std::string(Keywords::w_to),OperatorInfo(11,OperatorInfo::Type::Meta)},

					// Level 10: branchs (separately handling)
					{"(", OperatorInfo(-1,OperatorInfo::Type::Undefined)}, {")",OperatorInfo(-1,OperatorInfo::Type::Undefined)},{"[",OperatorInfo(-1,OperatorInfo::Type::Undefined)}, {"]", OperatorInfo(-1,OperatorInfo::Type::Undefined)},  // special cases

					// Level 9: unary operators
					{"!u", OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::Not)}, {"~u",OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::BitNot)},  {"-u",OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::Negative)},  // unary + and -, also bit NOT/ u-unary {"+u", OperatorInfo(8,OperatorInfo::Type::Unary, PseudoOpCode::Not)},

					{"*u", OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::GetValue)},{"&u",OperatorInfo(9,OperatorInfo::Type::Unary, PseudoOpCode::GetPointer)},   //Work with pointers
					
					
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
					{"=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Nop)}, {"+=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Add)}, {"-=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Subtract)}, 
					{"*=",OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Multiply)},
					{"/=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Divide)}, {"%=",OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Mod)}, {"&=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::BitAnd)}, {"|=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::BitOr)},
					{"<<=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::BitOffsetLeft)}, {">>=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::BitOffsetRight)}, {"&&=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::And)}, {"||=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Or)},
					{"~=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::BitNot)},{"^=", OperatorInfo(-1,OperatorInfo::Type::Assignment,PseudoOpCode::Exponentiate)}
				
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


		}
	}
}