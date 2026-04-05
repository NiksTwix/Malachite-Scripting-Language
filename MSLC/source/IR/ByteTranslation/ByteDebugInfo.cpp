#include "..\..\..\include\IR\ByteTranslation\ByteDebugInfo.hpp"



namespace MSLC {
	namespace IntermediateRepresentation
	{
		namespace Byte
		{
			CommandStringSerializer::CommandStringSerializer()
			{
				bopts_table_ = {
					{ByteOpCode::NOP,"NOP"},

                    {ByteOpCode::COMP_FLG, "COMP_FLG"},
                    {ByteOpCode::ADDR, "ADDR"},
                    {ByteOpCode::SUBR, "SUBR"},
                    {ByteOpCode::DIVR, "DIVR"},
                    {ByteOpCode::MULR, "MULR"},
                    {ByteOpCode::NEGR, "NEGR"},
                    {ByteOpCode::EXPR, "EXPR"},

                    {ByteOpCode::ADDI, "ADDI"},
                    {ByteOpCode::SUBI, "SUBI"},
                    {ByteOpCode::DIVI, "DIVI"},
                    {ByteOpCode::MULI, "MULI"},
                    {ByteOpCode::NEGI, "NEGI"},
                    {ByteOpCode::MODI, "MODI"},
                    {ByteOpCode::EXPI, "EXPI"},

                    {ByteOpCode::ADDU, "ADDU"},
                    {ByteOpCode::SUBU, "SUBU"},
                    {ByteOpCode::DIVU, "DIVU"},
                    {ByteOpCode::MULU, "MULU"},
                    {ByteOpCode::MODU, "MODU"},
                    {ByteOpCode::EXPU, "EXPU"},

                    {ByteOpCode::AND, "AND"},
                    {ByteOpCode::OR, "OR"},
                    {ByteOpCode::NOT, "NOT"},
                    {ByteOpCode::BIT_OR, "BIT_OR"},
                    {ByteOpCode::BIT_NOT, "BIT_NOT"},
                    {ByteOpCode::BIT_AND, "BIT_AND"},
                    {ByteOpCode::BIT_OFFSET_LEFT, "BIT_OFFSET_LEFT"},
                    {ByteOpCode::BIT_OFFSET_RIGHT, "BIT_OFFSET_RIGHT"},

                    {ByteOpCode::EQUALI,"EQUALI"},
                    {ByteOpCode::NOT_EQUALI,"NOT_EQUALI"},
                    {ByteOpCode::GREATERI,"GREATERI"},
                    {ByteOpCode::LESSI,"LESSI"},
                    {ByteOpCode::EGREATERI,"EGREATERI"},
                    {ByteOpCode::ELESSI,"ELESSI"},

                    {ByteOpCode::EQUALU,"EQUALU"},
                    {ByteOpCode::NOT_EQUALU,"NOT_EQUALU"},
                    {ByteOpCode::GREATERU,"GREATERU"},
                    {ByteOpCode::LESSU,"LESSU"},
                    {ByteOpCode::EGREATERU,"EGREATERU"},
                    {ByteOpCode::ELESSU,"ELESSU"},

                    {ByteOpCode::EQUALR,"EQUALR"},
                    {ByteOpCode::NOT_EQUALR,"NOT_EQUALR"},
                    {ByteOpCode::GREATERR,"GREATERR"},
                    {ByteOpCode::LESSR,"LESSR"},
                    {ByteOpCode::EGREATERR,"EGREATERR"},
                    {ByteOpCode::ELESSR,"ELESSR"},

                    {ByteOpCode::MOVRR, "MOVRR"},
                    {ByteOpCode::MOVRI, "MOVRI"},
                    {ByteOpCode::STACK_UP, "PUSH"},
                    {ByteOpCode::STACK_DOWN, "POP"},

                    {ByteOpCode::LEA_STATIC, "LEA_STATIC"},
                    {ByteOpCode::LEA_DYNAMIC, "LEA_DYNAMIC"},

                    {ByteOpCode::LOAD_DYNAMIC, "LOAD_DYNAMIC"},
                    {ByteOpCode::STORE_DYNAMIC, "STORE_DYNAMIC"},

    
                    {ByteOpCode::LOAD_CONST_STATIC, "LOAD_CONST_STATIC"},
                    {ByteOpCode::LEA_CONST, "LOAD_CONST_DYNAMIC"},

                    {ByteOpCode::LOAD_STATIC, "LOAD_STATIC"},
                    {ByteOpCode::STORE_STATIC, "STORE_STATIC"},

                    {ByteOpCode::ALLOC, "ALLOC"},
                    {ByteOpCode::FREE, "FREE"},

                    {ByteOpCode::JMP, "JMP"},
                    {ByteOpCode::JMP_LABEL, "JMP_LABEL" },
                    {ByteOpCode::JMPCV, "JMPCV"},
                    {ByteOpCode::JMPCNV, "JMPCNV"},
                    {ByteOpCode::CALL, "CALL"},
                    {ByteOpCode::RET, "RET"},
                    {ByteOpCode::EXT, "EXT"},


                    {ByteOpCode::TC_ITR, "TC_ITR"},
                    {ByteOpCode::TC_RTI, "TC_RTI"},
                    {ByteOpCode::TC_UTR, "TC_UTR"},
                    {ByteOpCode::TC_UTI, "TC_UTI"},
                    {ByteOpCode::TC_RTU, "TC_RTU"},
                    {ByteOpCode::TC_ITU, "TC_ITU"},
                    {ByteOpCode::SPEC_CALL, "SPEC_CALL" },
                };

                sts_table_ = {
                    {CommandSource::Constant, "C" },
                    {CommandSource::Register, "R" },
                    {CommandSource::Immediate, "I" },
                    {CommandSource::Symbol, "S" },
                    {CommandSource::MemoryAddress, "M" },
                }; 

                cfts_table_ =
                {
                    {ByteCommand::Flag::UnhandledSymbol, "US" },
                };
			}

			std::string CommandStringSerializer::Serialize(ByteCommand& command)
			{
                std::stringstream ss;

                ss << command.source_line << "(p.i " << command.pseudo_op_index << ")" << ":" << bopts_table_[command.code] << "|(" << sts_table_[command.arg0.type] << ") " << command.arg0.data << "|(" << sts_table_[command.arg1.type] << ") " << command.arg1.data << "|(" << sts_table_[command.arg2.type] << ") " << command.arg2.data;
                if (command.flags != 0 ) ss << "| Flags:";
                if (command.flags & ByteCommand::Flag::UnhandledSymbol) ss << cfts_table_[ByteCommand::Flag::UnhandledSymbol];


				return ss.str();
			}
		}
	}
}