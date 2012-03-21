
~!include/astral/Enum.h~
#ifndef ASTRAL_ENUM_H
#define ASTRAL_ENUM_H

#include <astral.h>
#include <astral.tokenizer.h>
#include <openxds/Object.h>
#include <openxds.adt.h>
#include <openxds.base.h>

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
	String*                   className;
	String*                 declaration;
	String*                        name;
	IDictionary<String>*         fields;

public:
	         Enum( CompilationUnit& cu, IPosition<SourceToken>& p );
	virtual ~Enum();

	virtual void addField( const String& key );
	
	virtual const String&           getClassName() const { return *this->className;   }
	virtual const String&         getDeclaration() const { return *this->declaration; }
	virtual const String&                getName() const { return *this->name;        }
	virtual       IDictionary<String>& getFields()       { return *this->fields;      }
	virtual const IDictionary<String>& getFields() const { return *this->fields;      }

private:
	virtual String*   determineClassName();
	virtual String* determineDeclaration();
	virtual void               parseEnum();
	virtual void          parseEnumBlock( IPosition<SourceToken>& pBlock );
	virtual void      parseEnumStatement( IPosition<SourceToken>& pStatement );

};

};

#endif
~

~!source/cplusplus/Enum.cpp~
#include "astral/CompilationUnit.h"
#include "astral/Enum.h"

#include <astral.tokenizer/SourceToken.h>

#include <openxds.adt/IDictionary.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>

using namespace astral;

using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;

Enum::Enum( CompilationUnit& cu, IPosition<SourceToken>& p ) : cu( cu )
{
	this->p           = p.copy();
	this->className   = determineClassName();
	this->declaration = determineDeclaration();
	this->fields      = new Dictionary<String>();
	this->name        = NULL;

	this->parseEnum();
}

Enum::~Enum()
{
	delete this->p;
	delete this->className;
	delete this->name;
	delete this->fields;
}

void
Enum::addField( const String& key )
{
	this->fields->insert( key.getChars(), new String( key ) );
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
				if ( !this->name ) this->name = new String( token.getValue() );
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
~


