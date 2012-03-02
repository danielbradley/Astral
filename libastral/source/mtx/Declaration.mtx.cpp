.	Declaration

~!include/astral/Declaration.h~
#include "astral.h"

#include <astral.ast.h>
#include <astral.tokenizer.h>
#include <openxds.adt.h>
#include <openxds/Object.h>
~

~include/astral/Declaration.h~
using astral::CompilationUnit;
using astral::ast::AST;
using astral::tokenizer::SourceToken;
using openxds::adt::IPosition;
using openxds::adt::ISequence;
using openxds::base::String;
~

~include/astral/Declaration.h~
namespace astral {
~

~include/astral/Declaration.h~
class Declaration : openxds::Object
{
private:
	CompilationUnit&                                   cu;

	IPosition<SourceToken>*                      position;
	String*                                       content;

protected:	
	void        setContent( String* aString );

public:
	 Declaration( CompilationUnit& aCompilationUnit );
	~Declaration();
	
	void initialise( IPosition<SourceToken>* p );
	
	void reparseContent( const String& content );
	
	const String& getContent() const;
	
private:
	void removeExistingDeclaration();
};
~

~include/astral/Declaration.h~
}; // end namespace astral.
~





~!source/cplusplus/Declaration.cpp~
#include "astral/CompilationUnit.h"
#include "astral/Declaration.h"

#include <astral.ast/AST.h>
#include <astral.tokenizer/SourceToken.h>

#include <openxds.adt/IPIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
~


~source/cplusplus/Declaration.cpp~
using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
~

~source/cplusplus/Declaration.cpp~
void
Declaration::setContent( String* aString )
{
	delete this->content;
	       this->content = aString;
}
~

~source/cplusplus/Declaration.cpp~
Declaration::Declaration( CompilationUnit& aCompilationUnit ) : cu( aCompilationUnit )
{
	this->content           = new String();
}
~

~source/cplusplus/Declaration.cpp~
Declaration::~Declaration()
{
	delete this->content;
}
~

~source/cplusplus/Declaration.cpp~
void
Declaration::initialise( IPosition<SourceToken>* aPosition )
{
	StringBuffer sb;

	this->position = aPosition;
	
	     ITree<SourceToken>& tree = this->cu.getAST().getTree();
	IPIterator<SourceToken>* it   = tree.children( *this->position );
	bool loop = true;
	while ( loop && it->hasNext() )
	{
		IPosition<SourceToken>* p = it->next();
		{
			SourceToken& token = p->getElement();
			
			switch ( token.getTokenType() )
			{
			case SourceToken::BLOCK:
				loop = false;
				break;
				
			default:
				break;
			}
			
			if ( loop ) sb.append( token.getValue() );
		}
		delete p;
	}
	delete it;

	String* tmp = sb.asString();
	{
		this->setContent( tmp->trim() );
	}
	delete tmp;
	
	fprintf( stdout, "Declaration::initialise(): %s\n", this->content->getChars() ); 	
}
~

~source/cplusplus/Declaration.cpp~
void
Declaration::reparseContent( const String& content )
{
	this->setContent( new String( content ) );
	this->removeExistingDeclaration();
	
	String* statement = new FormattedString( "%s\n;", content.getChars() );
	{
		AST* declarationAST = new AST();
		{
			declarationAST->parseString( *statement );

			ITree<SourceToken>&     t    = this->cu.getAST().getTree();
			ITree<SourceToken>&     tree = declarationAST->getTree();
			IPosition<SourceToken>* root = tree.root();
			IPosition<SourceToken>* parent = tree.child( *root, 0 );
			{
				IPIterator<SourceToken>* it = tree.children( *parent );
				bool loop = true;
				long i = 0;
				while ( loop && it->hasNext() )
				{
					IPosition<SourceToken>* p = it->next();
					{
						SourceToken& token = p->getElement();
						if ( SourceToken::STOP == token.getTokenType() )
						{
							loop = false;
						}
						else
						{
							t.insertChildAt( *this->position, new SourceToken( token ), i++ );
						}
					}
					delete p;
				}
				delete it;
			}
			delete root;
			delete parent;
		}
		delete declarationAST;
	}
	delete statement;
	
	this->cu.save();
}
~

~source/cplusplus/Declaration.cpp~
const String&
Declaration::getContent() const
{
	return *this->content;
}
~

~source/cplusplus/Declaration.cpp~
void
Declaration::removeExistingDeclaration()
{
	ITree<SourceToken>& tree = this->cu.getAST().getTree();

	while ( tree.isInternal( *this->position ) )
	{
		IPosition<SourceToken>* p = tree.child( *this->position, 0 );
		if ( SourceToken::BLOCK == p->getElement().getTokenType() )
		{
			delete p;
			break;
		}
		else
		{
			delete tree.remove( p );
		}
	}
}
~
