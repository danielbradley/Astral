#include "astral.ast/AST.h"
#include "astral.ast/ASTHelper.h"
#include "astral.tours/FindLastTokenTour.h"
#include "astral.tours/HTMLPrintTour.h"

#include <astral.tokenizer/SourceToken.h>
#include <openxds.base/String.h>
#include <openxds.adt/IDictionary.h>
#include <openxds.adt/IEIterator.h>
#include <openxds.adt/IEntry.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.io/IOBuffer.h>
#include <openxds.io/IEndPoint.h>
#include <openxds.io/OutputStream.h>
#include <openxds.io/PrintWriter.h>

using namespace astral;
using namespace astral::ast;
using namespace astral::tokenizer;
using namespace astral::tours;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::io;

static IPosition<SourceToken>* insertPosition( AST& ast, SourceToken::TokenType type, bool prefixWithTab, bool blankLine );
static IPosition<SourceToken>* findLast( AST& ast, SourceToken::TokenType type );
	
ASTHelper::ASTHelper( AST& anAST ) : ast( anAST )
{}

IPosition<SourceToken>*
ASTHelper::insertPackageAST( const AST& packageAST )
{
	return NULL;
}

IPosition<SourceToken>*
ASTHelper::insertImportAST( const AST& importAST )
{
	IPosition<SourceToken>* p = insertPosition( this->ast, SourceToken::IMPORT, false, false );
	{
		this->ast.replaceSubtree( *p, importAST );
	}
	return p;
}

IPosition<SourceToken>*
ASTHelper::insertClassAST( const AST& ast )
{
	return NULL;
}

IPosition<SourceToken>*
ASTHelper::insertMemberAST( const AST& aMemberAST )
{
	IPosition<SourceToken>* p = insertPosition( this->ast, SourceToken::MEMBER, true, true );
	{
		this->ast.replaceSubtree( *p, aMemberAST );
	}
	return p;
}

IPosition<SourceToken>*
ASTHelper::insertMethodAST( const AST& aMethodAST )
{
	IPosition<SourceToken>* p = insertPosition( this->ast, SourceToken::METHOD, true, true );
	{
		this->ast.replaceSubtree( *p, aMethodAST );
	}
	return p;
}

void
ASTHelper::reorder( IDictionary<IPosition<SourceToken> >& positions )
{
	IDictionary<AST>* dictionary = new SortedDictionary<AST>();
	{
		IEIterator<IPosition<SourceToken> >* ie = positions.entries();
		while ( ie->hasNext() )
		{
			IEntry<IPosition<SourceToken> >* e = ie->next();
			String key( e->getKey() );

			IPosition<SourceToken>* p   = positions.remove( e );
			AST*                    ast = this->removeSubtree( p );

			dictionary->insert( key.getChars(), ast );
		}
		delete ie;
	}
	
	{
		IEIterator<AST>* ie = dictionary->entries();
		while ( ie->hasNext() )
		{
			IEntry<AST>* e   = ie->next();
			String*      k   = new String( e->getKey() );
			AST*         ast = dictionary->remove( e );
			
			IPosition<SourceToken>* p = insertMethodAST( *ast );
			positions.insert( k->getChars(), p );
		}
		delete ie;
	}
	delete dictionary;
}

void
ASTHelper::replaceMethodAST( IPosition<SourceToken>& p, const AST& methodAST )
{
	this->ast.replaceSubtree( p, methodAST );
}

IPosition<SourceToken>* insertPosition( AST& ast, SourceToken::TokenType type, bool prefixWithTab, bool blankLine )
{
	IPosition<SourceToken>* p = NULL;

	IPosition<SourceToken>* last = findLast( ast, type );
	{
		long index = 0;

		IPosition<SourceToken>* parent = NULL;
		{
			if ( SourceToken::BLOCK == last->getElement().getTokenType() )
			{
				parent = last->copy();
			}
			else
			{
				parent = ast.getTree().parent( *last );
				index  = ast.getTree().nrOfChild( *last ) + 1;
			}

			//	Inserted in reverse order at same index.
			p = ast.getTree().insertChildAt( *parent, new SourceToken( type, new String() ), index );
			
			if ( prefixWithTab ) delete ast.getTree().insertChildAt( *parent, new SourceToken( SourceToken::TAB,     new String( "\t" ) ), index );
			if ( blankLine )     delete ast.getTree().insertChildAt( *parent, new SourceToken( SourceToken::NEWLINE, new String( "\n" ) ), index );
		}
		delete parent;
	}
	delete last;
	
	return p;
}


IPosition<SourceToken>* findLast( AST& ast, SourceToken::TokenType type )
{
	IPosition<SourceToken>* p = NULL;
	
	IPosition<SourceToken>* root = ast.getTree().root();
	{
		FindLastTokenTour tour( ast.getTree(), type );
		tour.doGeneralTour( *root );
		p = tour.copyLastTokenPosition();
	}
	delete root;
	
	return p;
}

static void removeTokenPrefix( ITree<SourceToken>& tree, IPosition<SourceToken>& parent, long index, SourceToken::TokenType type )
{
	if ( index < tree.nrChildren( parent ) )
	{
		IPosition<SourceToken>* p = tree.child( parent, index );
		if ( type == p->getElement().getTokenType() )
		{
			delete tree.remove( p );
		}
		else
		{
			delete p;
		}
	}
}

AST*
ASTHelper::removeSubtree( IPosition<SourceToken>* p )
{
	AST* ast = this->ast.copySubtree( *p );
	this->deleteSubtree( p );
	return ast;
}

void
ASTHelper::deleteSubtree( IPosition<SourceToken>* p )
{
	ITree<SourceToken>&     tree   = this->ast.getTree();
	IPosition<SourceToken>* parent = tree.parent( *p );
	{
		long index = tree.nrOfChild( *p );

		removeTokenPrefix( tree, *parent, index + 1, SourceToken::NEWLINE );

		delete tree.removeAsTree( p );

		removeTokenPrefix( tree, *parent, index - 1, SourceToken::TAB     );
		removeTokenPrefix( tree, *parent, index - 2, SourceToken::NEWLINE );
	}
	delete parent;
}

String*
ASTHelper::toHTMLString()
{
	PrintWriter writer( new OutputStream( new IOBuffer() ) );
	HTMLPrintTour tour( this->ast.getTree(), writer );
	tour.doGeneralTour();
	return dynamic_cast<IOBuffer&>( writer.getOutputStream().getIOEndPoint() ).toString();
}

String*
ASTHelper::toHTMLString( IPosition<SourceToken>& p )
{
	PrintWriter writer( new OutputStream( new IOBuffer() ) );
	HTMLPrintTour tour( this->ast.getTree(), writer );
	tour.doGeneralTour( p );
	return dynamic_cast<IOBuffer&>( writer.getOutputStream().getIOEndPoint() ).toString();
}

