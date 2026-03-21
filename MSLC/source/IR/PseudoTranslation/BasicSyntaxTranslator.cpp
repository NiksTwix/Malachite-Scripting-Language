#include "..\..\..\include\IR\PseudoTranslation\BasicSyntaxTranslator.hpp"
namespace MSLC
{
	namespace IntermediateRepresentation
	{
		namespace Pseudo
		{
			void BasicSyntaxTranslator::HandleBasicSyntax(AST::ASTNode& node, PseudoTranslationState& pts, recursive_handler rhandler)
			{
				switch (node.type)
				{
				case AST::ASTNodeType::None: break;
				case AST::ASTNodeType::Namespace: break;
				case AST::ASTNodeType::NodeGroup: break;		//IfCondition, maybe switch block 
				case AST::ASTNodeType::ForCycle: break;
				case AST::ASTNodeType::WhileCycle: break;
				case AST::ASTNodeType::Function: break;
				case AST::ASTNodeType::ObjectTemplate: break;
				case AST::ASTNodeType::LowLevelCodeBlock: 
					//opcst.HandleLowLevelCode(node, pts, rhandler);
					break;
				default:
					break;
				}
			}
		}
	}
}
