.	Member

A /Member/ object is used to represent a member declaration from an AST.
It stores

~!include/astral/Member.h~
#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>
#include <astral.h>
#include <astral.ast.h>
#include <astral.tokenizer.h>
~

~include/astral/Member.h~
using astral::ast::AST;
using astral::tokenizer::SourceToken;
using openxds::adt::IPosition;
using openxds::base::String;
~

~include/astral/Member.h~
namespace astral {
~




~include/astral/Member.h~
class Member : openxds::Object
{
private:
	CompilationUnit&                  cu;
	IPosition<SourceToken>*     position;
	AST*                       memberAST;
	String*                       member;
	String*                         name;

public:
	 Member( CompilationUnit& aCompilationUnit );
	 Member( CompilationUnit& aCompilationUnit, IPosition<SourceToken>& aPosition );
	~Member();

	void                setMember( const String& aValue );
	void            reparseMember( const String& aValue );
	void             removeMember();

	const String&         getName() { return *this->name;     }
	const String&       getMember() { return *this->member;   }

	bool                  isValid() const;

private:
	void initialise();
};
~

~include/astral/Member.h~
}; // end namespace
~




~!source/cplusplus/Member.cpp~
#include "astral/CompilationUnit.h"
#include "astral/Member.h"
#include "astral.ast/AST.h"
#include "astral.ast/ASTHelper.h"
#include "astral.tokenizer/SourceToken.h"

#include <openxds/Exception.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
~

~source/cplusplus/Member.cpp~
using namespace astral;
using namespace astral::ast;
using namespace astral::tokenizer;
using namespace openxds;
using namespace openxds::adt;
using namespace openxds::base;
~

~source/cplusplus/Member.cpp~
Member::Member( CompilationUnit& aCompilationUnit ) : cu( aCompilationUnit )
{
	this->position  = NULL;
	this->memberAST = new AST();
	this->member    = new String();
	this->name      = new String();
}

Member::Member( CompilationUnit& aCompilationUnit, IPosition<SourceToken>& aPosition ) : cu( aCompilationUnit )
{
	this->position  = aPosition.copy();
	this->memberAST = aCompilationUnit.getAST().copySubtree( aPosition );
	this->member    = new String();
	this->name      = new String();
	
	this->initialise();
}

Member::~Member()
{
	delete this->position;
	delete this->memberAST;
	delete this->member;
	delete this->name;
}
~




~source/cplusplus/Member.cpp~
//
//	Replicated in Import.mtx.cpp
//
static void traverse( StringBuffer& sb, ITree<SourceToken>& tree, IPIterator<SourceToken>& it )
{
	while ( it.hasNext() )
	{
		IPosition<SourceToken>* p = it.next();
		if ( tree.isExternal( *p ) )
		{
			sb.append( p->getElement().getValue() );
		}
		else
		{
			IPIterator<SourceToken>* it2 = tree.children( *p );
			{
				traverse( sb, tree, *it2 );
			}
			delete it2;
		}
		delete p;
	}
}

void
Member::initialise()
{
	ITree<SourceToken>&      tree = this->cu.getAST().getTree();
	IPIterator<SourceToken>* it   = tree.children( *this->position );
	{
		StringBuffer sb;

		bool loop = true;
		while ( loop && it->hasNext() )
		{
			IPosition<SourceToken>* p = it->next();
			{
				const SourceToken& token = p->getElement();
				switch ( token.getTokenType() )
				{
				case SourceToken::VARIABLE:
					delete this->name;
					this->name = new String( token.getValue() );
					loop = false;
					//	Intentional fall-through.
				default:
					sb.append( token.getValue() );
				}
			}
			delete p;
		}

		traverse( sb, tree, *it );
		this->member = sb.asString();
	}
	delete it;
}
~

~source/cplusplus/Member.cpp~
void
Member::setMember( const String& aValue )
{
	StringBuffer sb;
	sb.append( aValue );
	sb.append( '\n' );

	delete this->member;
	this->member = sb.asString();
}
~

~source/cplusplus/Member.cpp~
void
Member::reparseMember( const String& aValue )
{
	this->setMember( aValue );

	delete this->memberAST;
	this->memberAST = new AST();

	this->memberAST->parseString( aValue );
	if ( this->memberAST->isValid() )
	{
		CompilationUnit& cu  = this->cu;
		AST&             ast = cu.getAST(); 
		ASTHelper helper( ast );

		if ( this->position )
		{
			helper.replaceMemberAST( *this->position, *this->memberAST );
		}
		else
		{
			this->position = helper.insertMemberAST( *this->memberAST );
		}
		
		this->cu.save();

		this->initialise();
	}
}
~

~source/cplusplus/Member.cpp~
void
Member::removeMember()
{
	ASTHelper helper( this->cu.getAST() );
	
	helper.deleteSubtree( this->position );
	this->position = NULL;

	this->cu.save();
}
~

~source/cplusplus/Member.cpp~
bool
Member::isValid() const
{
	return this->memberAST->isValid();
}
~




