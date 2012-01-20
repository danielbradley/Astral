#ifndef ASTRAL_ADVANCEDHTMLPRINTTOUR_H
#define ASTRAL_ADVANCEDHTMLPRINTTOUR_H

#include "astral.h"
#include "astral.ast.h"
#include <astral.tokenizer.h>
#include <astral.tokenizer/SourceToken.h>

#include <openxds.io.h>
#include <openxds.adt.std/GeneralTour.h>
#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>

#include <cstdio>
#include <cstring>

using namespace astral::tokenizer;

namespace astral {

class AdvancedHTMLPrintTour : public openxds::adt::std::GeneralTour<SourceToken>
{
public:
	int depth;
	openxds::io::PrintWriter& writer;
	const CompilationUnit& cu;
	const CodeBase& codebase;
	
	VariableScopes* scopes;
	openxds::base::String* lastType;
	openxds::adt::ISequence<openxds::base::String>* methodCallTypes;

	AdvancedHTMLPrintTour(       openxds::adt::ITree<astral::tokenizer::SourceToken>& tree,
	                       const CompilationUnit& aCu,
						   const CodeBase& codebase,
								 openxds::io::PrintWriter& aWriter );
	virtual ~AdvancedHTMLPrintTour();

	virtual void  visitPreorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );
	virtual void visitPostorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );
	virtual void  visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );
};
	
};

#endif
