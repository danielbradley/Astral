#ifndef ASTRAL_TOURS_ENUMDISCOVERYTOUR_H
#define ASTRAL_TOURS_ENUMDISCOVERYTOUR_H

#include "astral.ast.h"
#include <astral.tokenizer.h>
#include <astral.tokenizer/SourceToken.h>

#include <openxds.adt/ITree.h>

#include <openxds.adt.h>
#include <openxds.adt/IDictionary.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ISequence.h>
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
using openxds::adt::ITree;
using openxds::adt::std::GeneralTour;
using openxds::base::String;

namespace astral {
	namespace tours {

class EnumDiscoveryTour : public GeneralTour<SourceToken>
{
private:
	ISequence<IPosition<SourceToken> >& enums;

public:
	EnumDiscoveryTour( ITree<SourceToken>& tree, ISequence<IPosition<SourceToken> >& enums )
	: GeneralTour<SourceToken>( tree ), enums( enums )
	{}

	virtual ~EnumDiscoveryTour()
	{}

	virtual void  visitPreorder( IPosition<SourceToken>& p, openxds::adt::std::Result& r )
	{
		SourceToken::TokenType type = p.getElement().getTokenType();

		switch ( type )
		{
		case SourceToken::ENUM:
			this->enums.insertLast( p.copy() );
			break;
			
		default:
			break;
		}
	}
	
	virtual void visitPostorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r ) {}

	virtual void  visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r ) {}
};
	
};};

#endif
