#include "astral/VariableScopes.h"

#include <astral/Name.h>
#include <astral/Type.h>
#include <astral.tokenizer/SourceToken.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/String.h>
#include <openxds.base/StringTokenizer.h>
#include <openxds.exceptions/NoSuchElementException.h>

using namespace astral;
using namespace astral::tokenizer;
using namespace openxds;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::exceptions;

VariableScopes::VariableScopes( const ITree<SourceToken>& ast ) : ast( ast )
{
	this->scopes = new Sequence<IDictionary<String> >();
}

VariableScopes::~VariableScopes()
{
	delete this->scopes;
}

void
VariableScopes::reset()
{
	delete this->scopes;
	this->scopes = new Sequence<IDictionary<String> >();
}

void
VariableScopes::addScope()
{
	this->scopes->addLast( new Dictionary<String>() );
}

void
VariableScopes::removeInnermostScope()
{
	if ( ! this->scopes->isEmpty() )
	{
		delete this->scopes->removeLast();
	}
}

void
VariableScopes::addMethodParameters( const IPosition<SourceToken>& p )
{
	String* type = NULL;
	String* name = NULL;

	const IPIterator<SourceToken>* it = this->ast.children( p );
	while ( it->hasNext() )
	{
		const IPosition<SourceToken>* pos = it->next();
		{
			switch ( pos->getElement().getTokenType() )
			{
			case SourceToken::PARAMETERS:
				this->addMethodParameters( *pos );
				break;
			case SourceToken::PARAMETER:
				this->addMethodParameters( *pos );
				break;
			case SourceToken::TYPE:
				if ( type ) delete type;
				type = new String( pos->getElement().getValue() );
				break;
			case SourceToken::VARIABLE:
				if ( name ) delete name;
				
				if ( type )
				{
					name = new String( pos->getElement().getValue() );
					this->scopes->getLast().insert( name->getChars(), new String( *type ) );
					if ( type ) delete type;
					if ( name ) delete name;

					type = NULL;
					name = NULL;
				}
				break;
			default:
				break;
			}
		}
		delete pos;
	}
	delete it;
	
	if ( type ) delete type;
	if ( name ) delete name;
}

void
VariableScopes::addVariableDeclaration( const IPosition<SourceToken>& p )
{
	String* type = new String();
	String* name = new String();
	{
		bool loop = true;
		const IPIterator<SourceToken>* it = this->ast.children( p );
		while ( loop && it->hasNext() )
		{
			const IPosition<SourceToken>* pos = it->next();
			{
				switch ( pos->getElement().getTokenType() )
				{
				case SourceToken::TYPE:
					delete type;
					type = new String( pos->getElement().getValue() );
					break;
				case SourceToken::VARIABLE:
					delete name;
					name = new String( pos->getElement().getValue() );
					loop = false;
					break;
				default:
					break;
				}
			}
			delete pos;
		}
		delete it;
		
		bool strip_parameterization = false;
		
		if ( strip_parameterization )
		{
			StringTokenizer st( *type );
			st.setDelimiter( '<' );
			if ( st.hasMoreTokens() )
			{
				delete type; type = st.nextToken();
			}
		}
		
		if ( ! this->scopes->isEmpty() )
		{
			//fprintf( stderr, "addDeclarationToScope: this->scopes->getLast().insert( %s, %s );\n", name->getChars(), type->getChars() );
		
			this->scopes->getLast().insert( name->getChars(), new String( *type ) );
		}
	}
	delete type;
	delete name;
}

String*
VariableScopes::searchForTypeOfName( const char* name ) const
{
	String* type = null;

	long nr = this->scopes->size() - 1;
	while ( 0 <= nr )
	{
		try
		{
			const IEntry<String>* e = this->scopes->get( (int) nr ).find( name );
			{
				type = new String( e->getValue() );
			}
			delete e;
			nr = -1;
		}
		catch ( NoSuchElementException* ex )
		{
			delete ex; // Not found, try next scope.
		}
		nr--;
	}

	return type;
}

//String*
//VariableScopes::searchForTypeOfName( const openxds::base::String& name ) const
//{
//	return this->searchForTypeOfName( name.getChars() );
//}

Type*
VariableScopes::searchForTypeOfName( const Name& name ) const
{
	Type* ret = null;
	{
		String* type = this->searchForTypeOfName( name.getValue().getChars() );
		if ( type )
		{
			ret = new Type( *type );
		}
		delete type;
	}
	return ret;
}


