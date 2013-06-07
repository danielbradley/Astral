#ifndef ASTRAL_AST_METHODDISCOVERYTOUR_H
#define ASTRAL_AST_METHODDISCOVERYTOUR_H

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

class MethodDiscoveryTour : public openxds::adt::std::GeneralTour<SourceToken>
{
private:
	openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >& methods;
	openxds::base::String* packageName;
	openxds::base::String* className;
	openxds::base::String* type;
	openxds::base::StringBuffer* current;
	openxds::adt::IPosition<astral::tokenizer::SourceToken>* methodP;
	bool inParameters;
	int   methodDepth;

public:
	MethodDiscoveryTour( openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >& mthds )
	: openxds::adt::std::GeneralTour<SourceToken>( tree ), methods( mthds )
	{
		this->packageName  = new openxds::base::String();
		this->className    = new openxds::base::String();
		this->type         = new openxds::base::String();
		this->current      = new openxds::base::StringBuffer();
		this->methodP      = NULL;
		this->inParameters = false;
		this->methodDepth  = 0;
	}

	~MethodDiscoveryTour()
	{
		delete this->packageName;
		delete this->className;
		delete this->type;
		delete this->current;
		delete this->methodP;
	}
	
	virtual void  visitPreorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );
	virtual void visitPostorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );
	virtual void  visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );
	
private:
	virtual void setPackageName( openxds::adt::IPosition<SourceToken>& p );
};
	
};};

#endif
