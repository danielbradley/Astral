#ifndef ASTRAL_AST_MEMBERDISCOVERYTOUR_H
#define ASTRAL_AST_MEMBERDISCOVERYTOUR_H

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

namespace astral {
	namespace tours {

class MemberDiscoveryTour : public openxds::adt::std::GeneralTour<SourceToken>
{
private:
	openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >& members;

public:
	MemberDiscoveryTour( openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >& aDictionary )
	: openxds::adt::std::GeneralTour<SourceToken>( tree ), members( aDictionary )
	{
	}

	virtual void  visitPreorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );
	
private:
	virtual void addMemberToDirectory( openxds::adt::IPosition<SourceToken>& p );
};
	
};};

#endif
