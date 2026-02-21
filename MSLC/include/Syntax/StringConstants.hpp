#pragma once 
#include <string_view>


namespace MSLC 
{
	namespace Keywords 
	{
		constexpr std::string_view 	w_if        = "if";
		constexpr std::string_view 	w_elif      = "elif";
		constexpr std::string_view 	w_else      = "else";
		constexpr std::string_view 	w_while     = "while";
		constexpr std::string_view 	w_for       = "for";
		constexpr std::string_view 	w_in		= "in";
		constexpr std::string_view 	w_continue  = "continue";
		constexpr std::string_view 	w_break     = "break";
		constexpr std::string_view 	w_func      = "func";
		constexpr std::string_view 	w_return    = "return";
		constexpr std::string_view 	w_op_code   = "op_code";
		constexpr std::string_view 	w_lite		= "lite";
		constexpr std::string_view 	w_kernel	= "kernel";
		constexpr std::string_view 	w_object_template	= "object_template";
		constexpr std::string_view 	w_alias     = "alias";
		constexpr std::string_view 	w_new       = "new";
		constexpr std::string_view 	w_delete    = "delete";
		constexpr std::string_view 	w_namespace = "namespace";

		constexpr std::string_view 	w_constructor = "constructor";
		constexpr std::string_view 	w_destructor = "destructor";
		constexpr std::string_view 	w_operator = "operator";

		constexpr std::string_view 	w_to = "to";
	}
	namespace TypeMarkers
	{
		constexpr std::string_view 	w_real		= "real";
		constexpr std::string_view 	w_integer	= "int";
		constexpr std::string_view 	w_unsigned	= "uint";
		constexpr std::string_view 	w_character = "char";
		constexpr std::string_view 	w_boolean	= "bool";
		constexpr std::string_view 	w_void		= "void";

		constexpr std::string_view 	w_const = "const";
		constexpr std::string_view 	w_gc = "gc";
		constexpr std::string_view 	w_ptr = "ptr";
		constexpr std::string_view 	w_ref = "ref";
		constexpr std::string_view 	w_var = "var";
	}
	namespace Literals
	{
		constexpr std::string_view w_true	= "true";
		constexpr std::string_view w_false = "false";

	}
	namespace Attributes	//@ + attribute name
	{
		constexpr std::string_view 	w_access_mode  = "access_mode";

		constexpr std::string_view 	w_override = "override";
		constexpr std::string_view 	w_static = "static";
		constexpr std::string_view 	w_virtual = "virtual";
		constexpr std::string_view 	w_inline = "inline";

		constexpr std::string_view 	w_obj_property = "property";
		
		constexpr std::string_view 	w_func_inheritance = "finherit";
		constexpr std::string_view 	w_func_block = "fblock";
		constexpr std::string_view 	w_func_block_override = "fblock_override";
	}
	namespace AttributeParams
	{
		constexpr std::string_view 	w_obj_property_set_mode = "set";
		constexpr std::string_view 	w_obj_property_get_mode = "get";
	}
	namespace AttributeArgs
	{
		//For attributes
		constexpr std::string_view w_public = "public";
		constexpr std::string_view w_private = "private";
		constexpr std::string_view w_protected = "protected";
	}
	namespace Directives	//# + directive name
	{
		constexpr std::string_view 	w_import = "import";

		constexpr std::string_view 	w_define_const = "define_const";
		constexpr std::string_view 	w_define_insertion = "define_insertion";
	}
} 