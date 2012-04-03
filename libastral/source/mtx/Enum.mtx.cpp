
~!include/astral/Enum.h~
#ifndef ASTRAL_ENUM_H
#define ASTRAL_ENUM_H

#include <astral.h>
#include <astral.ast.h>
#include <astral.tokenizer.h>
#include <openxds/Object.h>
#include <openxds.adt.h>
#include <openxds.base.h>

using astral::ast::AST;
using astral::tokenizer::SourceToken;
using openxds::adt::IDictionary;
using openxds::adt::IPosition;
using openxds::base::String;

namespace astral {

class Enum : openxds::Object
{
private:
	CompilationUnit&                 cu;
	IPosition<SourceToken>*           p;
	AST*                        enumAST;
	String*                     content;
	String*                   className;
	String*                 declaration;
	String*                        name;
	IDictionary<String>*         fields;

public:
	         Enum( CompilationUnit& cu );
	         Enum( CompilationUnit& cu, IPosition<SourceToken>& p );
	virtual ~Enum();

	virtual String*             determineContent();
	virtual void reparseEnum( const String& content );
	virtual void  removeEnum();
	virtual void setContent( const String& content );

	virtual void addField( const String& key );

	
	virtual const String&             getContent() const { return *this->content;     }
	virtual const String&           getClassName() const { return *this->className;   }
	virtual const String&         getDeclaration() const { return *this->content;     } // Kludge.
	virtual const String&                getName() const { return *this->name;        }
	virtual       IDictionary<String>& getFields()       { return *this->fields;      }
	virtual const IDictionary<String>& getFields() const { return *this->fields;      }

private:
	virtual String*   determineClassName();
	virtual String* determineDeclaration();
	virtual void               parseEnum();
	virtual void          parseEnumBlock( IPosition<SourceToken>& pBlock );
	virtual void      parseEnumStatement( IPosition<SourceToken>& pStatement );

	virtual void              initialise();
};

};

#endif
~

~!source/cplusplus/Enum.cpp~
#include "astral/CompilationUnit.h"
#include "astral/Enum.h"

#include <astral.ast/ASTHelper.h>
#include <astral.tours/PrintSourceTour.h>
#include <astral.tokenizer/SourceToken.h>

#include <openxds.adt/IDictionary.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.io/IOBuffer.h>
#include <openxds.io/OutputStream.h>
#include <openxds.io/PrintWriter.h>

using namespace astral;
using namespace astral::ast;
using namespace astral::tours;

using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::io;

Enum::Enum( CompilationUnit& cu ) : cu( cu )
{
	this->p           = NULL;
	this->enumAST     = new AST();

	this->content     = new String();

	this->className   = new String();
	this->declaration = new String();
	this->fields      = new Dictionary<String>();
	this->name        = new String();
}

Enum::Enum( CompilationUnit& cu, IPosition<SourceToken>& p ) : cu( cu )
{
	this->p           = p.copy();
	this->enumAST     = cu.getAST().copySubtree( p );

	this->content     = new String();

	this->className   = new String();
	this->declaration = new String();
	this->fields      = new Dictionary<String>();
	this->name        = new String();
	
	this->initialise();
}

Enum::~Enum()
{
	delete this->p;
	delete this->enumAST;
	delete this->content;
	delete this->className;
	delete this->declaration;
	delete this->fields;
	delete this->name;
}

void
Enum::initialise()
{
	delete this->content;
	       this->content     = determineContent(); 
	delete this->className;
	       this->className   = determineClassName();
	delete this->declaration;
	       this->declaration = determineDeclaration();

	this->parseEnum();
}

String*
Enum::determineContent()
{
	IOBuffer        buffer;
	OutputStream    os( buffer );
	PrintWriter     writer( os );
	PrintSourceTour pst( this->enumAST->getTree(), writer );

	pst.doGeneralTour();

	return buffer.toString();
}

void
Enum::reparseEnum( const String& content )
{
	this->setContent( content );

	delete this->enumAST;
	       this->enumAST = new AST();
		   this->enumAST->parseString( *this->content );

	if ( this->enumAST->isValid() )
	{
		CompilationUnit& cu  = this->cu;
		AST&             ast = cu.getAST();

		ASTHelper helper( ast );

		if ( this->p )
		{
			helper.replaceEnumAST( *this->p, *this->enumAST );
		}
		else
		{
			this->p = helper.insertEnumAST( *this->enumAST );
		}
		
		this->cu.save();
		
		this->initialise();
	}
}

void
Enum::removeEnum()
{
	if ( this->p )
	{
		CompilationUnit& cu  = this->cu;
		AST&             ast = cu.getAST();

		ASTHelper helper( ast );
		delete helper.removeSubtree( this->p ); this->p = NULL;
	}
}

void
Enum::setContent( const String& content )
{
	StringBuffer sb;
	sb.append( content );
	sb.append( '\n' );
	
	delete this->content;
	this->content = sb.asString();
}

String*
Enum::determineClassName()
{
	String* cls = new String();
	{
		ITree<SourceToken>&     t = this->cu.getAST().getTree();
		IPosition<SourceToken>* p = this->p->copy();
		IPosition<SourceToken>* x = NULL;

		bool loop = true;
		while ( loop && t.hasParent( *p ) )
		{
			x = p;
			p = t.parent( *x );
			
			if ( SourceToken::CLASS == p->getElement().getTokenType() )
			{
				loop = false;

				IPIterator<SourceToken>* it = t.children( *p );
				while ( it->hasNext() )
				{
					IPosition<SourceToken>* pChild = it->next();
					{
						if ( SourceToken::CLASSNAME == pChild->getElement().getTokenType() )
						{
							delete cls;
							cls = new String( pChild->getElement().getValue() );
						}
					}
					delete pChild;
				}
				delete it;
			}
			delete x;
		}
		delete p;
	}
	return cls;
}

String*
Enum::determineDeclaration()
{
	StringBuffer sb;

	IPIterator<SourceToken>* it = this->cu.getAST().getTree().children( *this->p );
	bool loop = true;
	while ( loop && it->hasNext() )
	{
		IPosition<SourceToken>* p = it->next();
		{
			SourceToken& token = p->getElement();
			
			switch ( token.getTokenType() )
			{
			case SourceToken::KEYWORD:
			case SourceToken::NAME:
			case SourceToken::SPACE:
			case SourceToken::WORD:
				sb.append( token.getValue() );
				break;
			
			case SourceToken::BLOCK:
				break;
			
			default:
				break;
			}
		}
		delete p;
	}
	delete it;
	
	return sb.asString();
}

void
Enum::parseEnum()
{
	ITree<SourceToken>&      tree = this->cu.getAST().getTree();
	IPIterator<SourceToken>* it   = tree.children( *this->p );

	while ( it->hasNext() )
	{
		IPosition<SourceToken>* p = it->next();
		{
			SourceToken& token = p->getElement();
			switch ( token.getTokenType() )
			{
			case SourceToken::NAME:
				if ( 0 == this->name->getLength() )
				{
					delete this->name;
					       this->name = new String( token.getValue() );
				}
				break;
				
			case SourceToken::BLOCK:
				this->parseEnumBlock( *p );
				break;
			
			default:
				break;
			}
		}
		delete p;
	}
	delete it;
}

void
Enum::parseEnumBlock( IPosition<SourceToken>& pBlock )
{
	ITree<SourceToken>&      tree = this->cu.getAST().getTree();
	IPIterator<SourceToken>* it   = tree.children( pBlock );
	while ( it->hasNext() )
	{
		IPosition<SourceToken>* p = it->next();
		{
			SourceToken& token = p->getElement();
			switch ( token.getTokenType() )
			{
			case SourceToken::STATEMENT:
				this->parseEnumStatement( *p );
				break;
				
			default:
				break;
			}
		}
		delete p;
	}
	delete it;
}

void
Enum::parseEnumStatement( IPosition<SourceToken>& pStatement )
{
	ITree<SourceToken>&      tree = this->cu.getAST().getTree();
	IPIterator<SourceToken>* it   = tree.children( pStatement );
	while ( it->hasNext() )
	{
		IPosition<SourceToken>* p = it->next();
		{
			SourceToken& token = p->getElement();
			switch ( token.getTokenType() )
			{
			case SourceToken::VARIABLE:
				this->addField( token.getValue() );
				break;
				
			default:
				break;
			}
		}
		delete p;
	}
	delete it;
}

void
Enum::addField( const String& key )
{
	this->fields->insert( key.getChars(), new String( key ) );
}
~


