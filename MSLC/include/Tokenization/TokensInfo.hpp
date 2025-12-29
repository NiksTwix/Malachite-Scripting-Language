#pragma once
#include "..\Definitions\ValueContainer.hpp"
#include "..\Syntax\StringContants.hpp"

#include <unordered_map>

namespace MSLC
{
	namespace Tokenization 
	{
		enum class TokenType 
		{
			UNDEFINED = 0,
			IDENTIFIER,
			OPERATOR, 
			LITERAL,   //false/true
			DELIMITER,
			TYPE_MARKER,
			KEYWORD,
			COMPILATION_LABEL,	//Operation's end and etc

			//Attributes
			ATTRIBUTE,
			ATTRIBUTE_PARAM,	//get/set (in @property),
			ATTRIBUTE_ARG,		//public,protected,private

			//Directives
			PREPROCESSOR_DIRECTIVE
		};
	
		enum class CompilationLabel : uint8_t
		{
			OPERATION_END = 0,  //End of operation


			OPEN_VISIBLE_SCOPE,    //Start of variable scope
			CLOSE_VISIBLE_SCOPE,      //End of variable scope
			FUNCTION_CALL,  //function(params)
			OFFSET_ACCESS,  //array[index]

			FIELD_ACCESS,   //For class.field
			METHOD_CALL,     //For class.method(params)

			NODES_GROUP,

		};

		struct Token 
		{
			//std::string source_value;
			Definitions::ValueContainer value;
			TokenType type;
			size_t line;
			size_t module_id;

			Token(Definitions::ValueContainer val_con, TokenType type, size_t line = 0, size_t module_id = 0) : value(val_con), type(type), line(line), module_id(module_id) {}
			Token(TokenType type) : value((uint64_t)0), type(type), line(0), module_id(0) {}
			Token() : value((uint64_t)0), type(TokenType::UNDEFINED), line(0), module_id(0) {}
			Token(TokenType type, size_t line = 0, size_t module_id = 0) : value((uint64_t)0), type(type), line(line), module_id(module_id) {}
		};


		class TokensTypeTable 
		{
			std::unordered_map<std::string, TokenType> table;
			
			void Init() 
			{
				table =
				{
					{std::string(Keywords::w_if),TokenType::KEYWORD},
					{std::string(Keywords::w_elif),TokenType::KEYWORD},
					{std::string(Keywords::w_else),TokenType::KEYWORD},
					{std::string(Keywords::w_while),TokenType::KEYWORD},
					{std::string(Keywords::w_for),TokenType::KEYWORD},
					{std::string(Keywords::w_in),TokenType::KEYWORD},
					{std::string(Keywords::w_continue),TokenType::KEYWORD},
					{std::string(Keywords::w_break),TokenType::KEYWORD},
					{std::string(Keywords::w_func),TokenType::KEYWORD},
					{std::string(Keywords::w_return),TokenType::KEYWORD},
					{std::string(Keywords::w_op_code),TokenType::KEYWORD},
					{std::string(Keywords::w_lite),TokenType::KEYWORD},
					{std::string(Keywords::w_kernel),TokenType::KEYWORD},
					{std::string(Keywords::w_object_template),TokenType::KEYWORD},
					{std::string(Keywords::w_alias),TokenType::KEYWORD},
					{std::string(Keywords::w_new),TokenType::KEYWORD},
					{std::string(Keywords::w_delete),TokenType::KEYWORD},
					{std::string(Keywords::w_namespace),TokenType::KEYWORD},
					{std::string(Keywords::w_const),TokenType::KEYWORD},
					{std::string(BasicTypeMarkers::w_real),TokenType::TYPE_MARKER},
					{std::string(BasicTypeMarkers::w_integer),TokenType::TYPE_MARKER},
					{std::string(BasicTypeMarkers::w_unsigned),TokenType::TYPE_MARKER},
					{std::string(BasicTypeMarkers::w_character),TokenType::TYPE_MARKER},
					{std::string(BasicTypeMarkers::w_boolean),TokenType::TYPE_MARKER},
					{std::string(BasicTypeMarkers::w_void),TokenType::TYPE_MARKER},
					{std::string(Literals::w_true),TokenType::LITERAL},
					{std::string(Literals::w_false),TokenType::LITERAL},
					{std::string(Attributes::w_access_mode),TokenType::ATTRIBUTE},
					{std::string(Attributes::w_override),TokenType::ATTRIBUTE},
					{std::string(Attributes::w_static),TokenType::ATTRIBUTE},
					{std::string(Attributes::w_virtual),TokenType::ATTRIBUTE},
					{std::string(Attributes::w_inline),TokenType::ATTRIBUTE},
					{std::string(Attributes::w_obj_property),TokenType::ATTRIBUTE},
					{std::string(Attributes::w_stack_only),TokenType::ATTRIBUTE},
					{std::string(Attributes::w_func_inheritance),TokenType::ATTRIBUTE},
					{std::string(Attributes::w_func_block),TokenType::ATTRIBUTE},
					{std::string(Attributes::w_func_block_override),TokenType::ATTRIBUTE},
					{std::string(AttributeParams::w_obj_property_set_mode),TokenType::ATTRIBUTE_PARAM},
					{std::string(AttributeParams::w_obj_property_get_mode),TokenType::ATTRIBUTE_PARAM},
					{std::string(AttributeArgs::w_public),TokenType::ATTRIBUTE_ARG},
					{std::string(AttributeArgs::w_private),TokenType::ATTRIBUTE_ARG},
					{std::string(AttributeArgs::w_protected),TokenType::ATTRIBUTE_ARG},
					{std::string(Directives::w_import),TokenType::PREPROCESSOR_DIRECTIVE},
					{std::string(Directives::w_define_const), TokenType::PREPROCESSOR_DIRECTIVE},
					{"(", TokenType::DELIMITER },
					{")", TokenType::DELIMITER},
					{"{", TokenType::DELIMITER},
					{"}", TokenType::DELIMITER},
					{"[", TokenType::DELIMITER},
					{"]", TokenType::DELIMITER},
					{":", TokenType::DELIMITER},    //delimiter for inheritance
					{",", TokenType::DELIMITER},
					{".", TokenType::DELIMITER},
					{";",TokenType::DELIMITER},
					{"+", TokenType::OPERATOR},
					{"-", TokenType::OPERATOR},
					{"+u", TokenType::OPERATOR},    //u-unary
					{"-u", TokenType::OPERATOR},
					//Pointers
					{"&u", TokenType::OPERATOR},
					{"`u", TokenType::OPERATOR},

					{"`", TokenType::OPERATOR},
					//Pointers end

					{"/", TokenType::OPERATOR},
					{"*", TokenType::OPERATOR},
					{"%", TokenType::OPERATOR},
					{"=", TokenType::OPERATOR},
					{"+=", TokenType::OPERATOR},
					{"-=", TokenType::OPERATOR},
					{"/=", TokenType::OPERATOR},
					{"*=", TokenType::OPERATOR},
					{"==", TokenType::OPERATOR},
					{"!=", TokenType::OPERATOR},
					{"!", TokenType::OPERATOR},     // 
					{"!u", TokenType::OPERATOR},    //  Its result of lexer
					{">",TokenType::OPERATOR},
					{"<",TokenType::OPERATOR},
					{">=",TokenType::OPERATOR},
					{"<=",TokenType::OPERATOR},
					{"->",TokenType::OPERATOR},     //Operator of returned function's type or implication (in the future)
					{">>",TokenType::OPERATOR},
					{"<<",TokenType::OPERATOR},
					{">>=",TokenType::OPERATOR},
					{"<<=",TokenType::OPERATOR},
					{"&&",TokenType::OPERATOR},
					{"||",TokenType::OPERATOR},
					{"&&=",TokenType::OPERATOR},
					{"||=",TokenType::OPERATOR},
					{"&",TokenType::OPERATOR},
					{"~",TokenType::OPERATOR},
					{"|",TokenType::OPERATOR},
					{"&=",TokenType::OPERATOR},
					{"|=",TokenType::OPERATOR},
					{"~=",TokenType::OPERATOR},
				};
			}

			TokensTypeTable()
			{
				Init();
			}

		public:
			static TokensTypeTable& Get() 
			{
				static TokensTypeTable table;
				return table;
			}

			TokenType GetTokenType(const std::string& str_token)
			{
				auto it = table.find(str_token);
				return it != table.end() ? it->second : TokenType::UNDEFINED;
			}
		};

	}

}