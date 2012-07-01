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

static IPosition<SourceToken>* insertPosition( AST& ast, SourceToken::TokenType type );
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
	IPosition<SourceToken>* p = insertPosition( this->ast, SourceToken::IMPORT );
	{
		this->replaceReparsedAST( *p, importAST );
	}
	return p;
}

IPosition<SourceToken>*
ASTHelper::insertClassAST( const AST& ast )
{
	return NULL;
}

IPosition<SourceToken>*
ASTHelper::insertEnumAST( const AST& anEnumAST )
{
	IPosition<SourceToken>* p = insertPosition( this->ast, SourceToken::ENUM );
	{
		this->replaceEnumAST( *p, anEnumAST );
	}
	return p;
}

IPosition<SourceToken>*
ASTHelper::insertMemberAST( const AST& aMemberAST )
{
	IPosition<SourceToken>* p = insertPosition( this->ast, SourceToken::MEMBER );
	{
		this->replaceReparsedAST( *p, aMemberAST );
	}
	return p;
}

IPosition<SourceToken>*
ASTHelper::insertMethodAST( const AST& aMethodAST )
{
	IPosition<SourceToken>* p = insertPosition( this->ast, SourceToken::METHOD );
	{
		this->replaceReparsedAST( *p, aMethodAST );
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

AST*
ASTHelper::retrieveMethodAST( IPosition<SourceToken>& p )
{
	AST* ast = this->ast.copySubtree( p );

	ast->storeIndent();
	
	return ast;
}

void
ASTHelper::replaceImportAST( IPosition<SourceToken>& p, const AST& importAST )
{
	this->replaceReparsedAST( p, importAST );
}

void
ASTHelper::replaceEnumAST( IPosition<SourceToken>& p, const AST& enumAST )
{
	this->replaceReparsedAST( p, enumAST );
}

void
ASTHelper::replaceMemberAST( IPosition<SourceToken>& p, const AST& memberAST )
{
	this->replaceReparsedAST( p, memberAST );
}

void
ASTHelper::replaceMethodAST( IPosition<SourceToken>& p, const AST& methodAST )
{
	this->replaceReparsedAST( p, methodAST );
}

IPosition<SourceToken>* insertPosition( AST& ast, SourceToken::TokenType type )
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
			delete ast.getTree().insertChildAt( *parent, new SourceToken( SourceToken::NEWLINE, new String( "\n" ) ), index );
			p = ast.getTree().insertChildAt( *parent, new SourceToken( type, new String() ), index );
			delete ast.getTree().insertChildAt( *parent, new SourceToken( SourceToken::NEWLINE, new String( "\n" ) ), index );
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

//
//	Private methods
//


/*
 *	A reparsed AST should have a root node of type SourceToken::OTHER,
 *  and child nodes corresponding to the reparsed elements,
 *	e.g. SourceToken::IMPORT, MEMBER, METHOD etc.
 */
void
ASTHelper::replaceReparsedAST( IPosition<SourceToken>& p, const AST& aReparsedAST )
{
	ITree<SourceToken>* t = aReparsedAST.getTree().copyAsTree();
	{
		if ( t->size() )
		{
			IPosition<SourceToken>& r = t->getRoot();
			switch ( r.getElement().getTokenType() )
			{
			case SourceToken::OTHER:
				{
					IPosition<SourceToken>* pReparsed = t->child( r, 0 );
					{
						this->ast.getTree().swapSubtrees( p, *t, *pReparsed );
						this->ast.recount( p );
					}
					delete pReparsed;
				}
				break;

			case SourceToken::IMPORT:
			case SourceToken::MEMBER:
			case SourceToken::METHOD:
				{
					this->ast.getTree().swapSubtrees( p, *t, r );
					this->ast.recount( p );
				}
				break;

			default:
				break;
			}
		}
	}
	delete t;
}

long
ASTHelper::calculateOffset( const IPosition<SourceToken>& p )
{
	long offset = p.getElement().getOffset();
	if ( ast.getTree().hasParent( p ) )
	{
		const IPosition<SourceToken>* parent = ast.getTree().parent( p );
		{
			offset += this->calculateOffset( *parent );
		}
		delete parent;
	}
	return offset;
}
