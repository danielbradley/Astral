#ifndef ASTRAL_TOURS_FINDLASTTOKENTOUR_H
#define ASTRAL_TOURS_FINDLASTTOKENTOUR_H

#include "astral.ast.h"
#include <astral.tokenizer.h>
#include <astral.tokenizer/SourceToken.h>

#include <openxds.adt/ITree.h>

#include <openxds.adt.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/IDictionary.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/GeneralTour.h>
#include <openxds.base.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds/Object.h>

#include <cstdio>
#include <cstring>

using namespace astral::tokenizer;
using openxds::adt::IPosition;
using openxds::adt::ISequence;
using openxds::adt::ITree;
using openxds::adt::std::GeneralTour;

namespace astral {
	namespace tours {

class FindLastTokenTour : public GeneralTour<SourceToken>
{
private:
	IPosition<SourceToken>* parent;
	IPosition<SourceToken>* prev;
	IPosition<SourceToken>* last;

	SourceToken::TokenType  parentType;
	SourceToken::TokenType  prevType;
	SourceToken::TokenType  type;

public:
	FindLastTokenTour( ITree<SourceToken>& tree, SourceToken::TokenType type )
	: GeneralTour<SourceToken>( tree )
	{
		this->parent = NULL;
		this->prev   = NULL;
		this->last   = NULL;

		this->type   = type;

		switch ( type )
		{
		case SourceToken::IMPORT:
			this->parentType = SourceToken::FILE;
			this->prevType   = SourceToken::PACKAGE;
			break;

		case SourceToken::CLASS:
			this->parentType = SourceToken::FILE;
			this->prevType   = SourceToken::IMPORT;
			break;

		case SourceToken::ENUM:
			this->parentType = SourceToken::FILE;
			this->prevType   = SourceToken::IMPORT;
			break;
		
		case SourceToken::MEMBER:
			this->parentType = SourceToken::BLOCK;
			this->prevType   = SourceToken::OTHER;
			break;

		case SourceToken::METHOD:
			this->parentType = SourceToken::BLOCK;
			this->prevType   = SourceToken::MEMBER;
			break;

		default:
			break;
		}

	}

	virtual ~FindLastTokenTour()
	{
		delete this->parent;
		delete this->prev;
		delete this->last;
	}

	virtual void setParent( IPosition<SourceToken>& p )
	{
		delete this->parent;
		       this->parent = p.copy();
	}

	virtual void setPrev( IPosition<SourceToken>& p )
	{
		delete this->prev;
		       this->prev = p.copy();
	}
	
	virtual void setLast( IPosition<SourceToken>& p )
	{
		delete this->last;
		       this->last = p.copy();
	}
	
	virtual void  visitPreorder( IPosition<SourceToken>& p, openxds::adt::std::Result& r )
	{
		SourceToken::TokenType type = p.getElement().getTokenType();
		
		if ( !this->parent && (this->parentType == type) )
		{
			this->setParent( p );
		}
		else if ( this->prevType == type )
		{
			this->setPrev( p );
		}
		else if ( this->type == type )
		{
			this->setLast( p );
		}
	}
	
	virtual void visitPostorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r ) {}

	virtual void  visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
	{
		if ( this->parent && !this->last )
		{
			switch ( this->parentType )
			{
			case SourceToken::BLOCK:
				if ( p.getElement().getTokenType() == SourceToken::STARTBLOCK )
				{
					this->setLast( p );
				}
				break;
			default:
				break;
			}
		}
	}

	virtual IPosition<SourceToken>* copyLastTokenPosition()
	{
		if ( this->last )
		{
			return this->last->copy();
		}
		else if ( this->prev )
		{
			return this->prev->copy();
		}
		else
		{
			return NULL;
		}
	}
};
	
};};

#endif
