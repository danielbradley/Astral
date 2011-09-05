#include "astral.tours/MethodDiscoveryTour.h"

#include <astral.tokenizer/SourceToken.h>
#include <openxds.adt/IEntry.h>
#include <openxds.base/StringBuffer.h>

using namespace astral::ast;
using namespace astral::tokenizer;
using namespace astral::tours;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;

void
MethodDiscoveryTour::visitPreorder( IPosition<SourceToken>& p, Result& r )
{
	SourceToken& token = p.getElement();
	
	switch ( token.getTokenType() )
	{
	case SourceToken::METHOD:
		if ( !this->methodP )
		{
			this->methodP = p.copy();
		
			delete this->current;
			this->current = new StringBuffer();
//			this->current->append( *this->packageName );
//			this->current->append( '.' );
//			this->current->append( *this->className );
		}
		break;
	case SourceToken::PARAMETERS:
		this->inParameters = true;
		break;
	case SourceToken::PACKAGE:
		this->setPackageName( p );
		break;
	}
}

void
MethodDiscoveryTour::visitPostorder( IPosition<SourceToken>& p, Result& r )
{
	SourceToken& token = p.getElement();

	switch ( token.getTokenType() )
	{
	case SourceToken::PARAMETERS:
		if ( this->methodP )
		{
			if ( ',' == this->current->charAt( this->current->getLength() - 1 ) )
			{
				this->current->removeLast();
			}
			this->current->append( ')' );
			this->current->append( '|' );
			this->current->append( *this->type );
			
			delete this->methods.insert( this->current->getChars(), this->methodP->copy() );
			delete this->current;
			this->current = new StringBuffer();
			delete this->type;
			this->type = new String();

			delete this->methodP;
			this->methodP = NULL;
		}
		this->inParameters = false;
		break;
	}
}

void
MethodDiscoveryTour::visitExternal( IPosition<SourceToken>& p, Result& r )
{
	SourceToken& token = p.getElement();
	
	switch ( token.getTokenType() )
	{
	case SourceToken::CLASSNAME:
		delete this->className;
		this->className = token.getValue().asString();
		break;
	case SourceToken::METHODNAME:
		this->current->append( token.getValue() );
		this->current->append( '(' );
		break;
	case SourceToken::TYPE:
		if ( this->inParameters )
		{
			this->current->append( token.getValue() );
			this->current->append( ',' );
		} else {
			delete this->type;
			this->type = token.getValue().asString();
		}
		break;
	}
}

void
MethodDiscoveryTour::setPackageName( IPosition<SourceToken>& p )
{
	StringBuffer sb;

	IPIterator<SourceToken>* it = this->tree.children( p );
	while ( it->hasNext() )
	{
		IPosition<SourceToken>* p1 = it->next();
		{
			switch ( p1->getElement().getTokenType() )
			{
			case SourceToken::NAME:
				sb.append( p1->getElement().getValue() );
				break;
			case SourceToken::SELECTOR:
				sb.append( p1->getElement().getValue() );
				break;
			}
		}
		delete p1;
	}
	delete it;

	delete this->packageName;
	this->packageName = sb.asString();
}


