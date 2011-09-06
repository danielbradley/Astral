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
	const openxds::adt::IDictionary<openxds::adt::IEntry<CompilationUnit> >& symbols;
	const openxds::adt::IDictionary<openxds::base::String>& importedTypes;
	
	openxds::adt::ISequence<openxds::adt::IDictionary<openxds::base::String> >* scopes;
	openxds::base::String* lastType;
	openxds::adt::ISequence<openxds::base::String>* methodCallTypes;

	AdvancedHTMLPrintTour(       openxds::adt::ITree<astral::tokenizer::SourceToken>& tree,
	                       const CompilationUnit& aCu,
	                       const openxds::adt::IDictionary<openxds::adt::IEntry<CompilationUnit> >& symbols,
	                       const openxds::adt::IDictionary<openxds::base::String>& importedTypes,
								 openxds::io::PrintWriter& aWriter );
	virtual ~AdvancedHTMLPrintTour();

	virtual void  visitPreorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );
	virtual void visitPostorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );
	virtual void  visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );

	virtual void addDeclarationToScope( openxds::adt::IPosition<SourceToken>& p );


	virtual openxds::base::String*             resolveLink( const char* name, const openxds::base::String& lastType );
	virtual openxds::base::String*       resolveTypeOfName( const char* name );
	virtual openxds::base::String*      resolveTypeIfLocal( const char* name );
	virtual openxds::base::String*           resolveFQType( const char* type );

	virtual openxds::base::String*       resolveMethodCall( const char* name, const openxds::base::String& lastType, const openxds::base::String& parameters );
	virtual openxds::base::String*  resolveInvocationClass( const openxds::base::String& lastType );
	virtual openxds::base::String*  resolveMethodSignature( const char* invokee, const char* name, const char* parameters );
	virtual openxds::base::String*    findMethodParameters( openxds::adt::IPosition<SourceToken>& p );
	virtual openxds::base::String*  resolveMethodCallParametersToTypes( const openxds::base::String& parameters );
};
	
};

#endif
