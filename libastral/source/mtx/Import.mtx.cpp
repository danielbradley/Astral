.	Import

This class represents an /import/ statement.
It stores a reference to its source in the AST allow this object to be used to modify or delete the import statement.

~!source/cobalt/Import.cm.cpp~
namespace astral;

use openxds;
use astral;

class Import extends Object
{
	@cu        : CompilationUnit&;
	@position  : IPosition<SourceToken>*;
	@import    : String*;
}

public new ( aCompilationUnit : CompilationUnit& )
{
	@cu = aCompilationUnit;

	@import = new String();
}

public new ( aCompilationUnit : CompilationUnit&, aPosition : IPosition<SourceToken>& )
{
	@cu = aCompilationUnit;

	@position = aPosition.copy();

	this.initialise();
}

public initialise()
{
	foreach ( SourceToken& token; @cu.getAST().getTree().childrenValues( @position ) )
	{
		switch ( token.getTokenType() )
		{
		case SourceToken::NAME:
			@import = new String( token.getValue() );
			break;
		
		default
			break;
		}
	}
}

public setImport( anImportString const String& )
{
	@import = new String( anImportString );
}

public const getImport() : const String&
{
	return @import;
}
~





~!include/astral/Import.h~
#ifndef ASTRAL_IMPORT_H
#define ASTRAL_IMPORT_H
~

~include/astral/Import.h~
#include <astral.h>
#include <astral.ast.h>
#include <astral.tokenizer.h>

#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>
~

~include/astral/Import.h~
using astral::CompilationUnit;
using astral::ast::AST;
using astral::tokenizer::SourceToken;
using openxds::adt::IPosition;
using openxds::base::String;
~

~include/astral/Import.h~
namespace astral {
~

~include/astral/Import.h~
class Import : openxds::Object
{
private:
	CompilationUnit&                cu;
	IPosition<SourceToken>*   position;
	AST*                    partialAST;
	String*                     import;
	String*                    content;

public:
	 Import( CompilationUnit& aCompilationUnit );
	 Import( CompilationUnit& aCompilationUnit, IPosition<SourceToken>& aPosition );
	~Import();

	void     setContent( const String& aString );
	void reparseContent( const String& aString );
	void   removeFromCU();
	
	const String&  getImport() const;
	const String& getContent() const;

	bool isValid() const;

private:
	void initialise();
};
~

~include/astral/Import.h~
}; // end namespace
~

~include/astral/Import.h~
#endif
~




~!source/cplusplus/Import.cpp~
#include "astral/Import.h"

#include <astral/CompilationUnit.h>
#include <astral.ast/AST.h>
#include <astral.ast/ASTHelper.h>
#include <astral.tokenizer/SourceToken.h>

#include <openxds.adt/IPosition.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.adt/ITree.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
~

~source/cplusplus/Import.cpp~
using namespace astral;
using namespace astral::ast;
using namespace astral::tokenizer;
using namespace openxds::adt;
using namespace openxds::base;
~

~source/cplusplus/Import.cpp~
Import::Import( CompilationUnit& aCompilationUnit ) : cu( aCompilationUnit )
{
	this->position   = NULL;
	this->partialAST = new AST();
	this->import     = new String();
	this->content    = new String();
}
~

~source/cplusplus/Import.cpp~
Import::Import( CompilationUnit& aCompilationUnit, IPosition<SourceToken>& aPosition ) : cu( aCompilationUnit )
{
	this->position  = aPosition.copy();
	this->partialAST = aCompilationUnit.getAST().copySubtree( aPosition );
	this->import    = new String();
	this->content   = new String();

	this->initialise();
}
~

~source/cplusplus/Import.cpp~
Import::~Import()
{
	delete this->position;
	delete this->partialAST;
	delete this->import;
	delete this->content;
}
~

~source/cplusplus/Import.cpp~
//
//	Replicated in Member.mtx.cpp
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
Import::initialise()
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
				case SourceToken::NAME:
					delete this->import;
					       this->import = new String( token.getValue() );
					loop = false;
					//	Intentional fall-through.
				default:
					sb.append( token.getValue() );
				}
			}
			delete p;
		}
		
		traverse( sb, tree, *it );
		delete this->content;
		       this->content = sb.asString();
	}
	delete it;
}
~

~source/cplusplus/Import.cpp~
void
Import::setContent( const String& aString )
{
	StringBuffer sb;
	sb.append( aString );
	sb.append( '\n' );

	delete this->content;
	this->content = sb.asString();
}
~

~source/cplusplus/Import.cpp~
void
Import::reparseContent( const String& aValue )
{
	this->setContent( aValue );

	delete this->partialAST;
	this->partialAST = new AST();

	this->partialAST->parseString( *this->content );
	if ( this->partialAST->isValid() )
	{
		CompilationUnit& cu  = this->cu;
		AST&             ast = cu.getAST(); 
		ASTHelper helper( ast );

		if ( this->position )
		{
			helper.replaceImportAST( *this->position, *this->partialAST );
		}
		else
		{

			this->position = helper.insertImportAST( *this->partialAST );
		}
		
		this->cu.save();

		this->initialise();
	}
}
~

~source/cplusplus/Import.cpp~
const String&
Import::getImport() const
{
	return *this->import;
}
~

~source/cplusplus/Import.cpp~
const String&
Import::getContent() const
{
	return *this->content;
}
~

~source/cplusplus/Import.cpp~
void
Import::removeFromCU()
{
	ASTHelper helper( this->cu.getAST() );
	
	helper.deleteSubtree( this->position );
	this->position = NULL;

	this->cu.save();
}
~

~source/cplusplus/Import.cpp~
bool
Import::isValid() const
{
	return this->partialAST->isValid();
}
~
