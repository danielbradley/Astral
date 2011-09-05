#ifndef ASTRAL_TOURS_METHODDISCOVERYTOUR_H
#define ASTRAL_TOURS_METHODDISCOVERYTOUR_H

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

class PackageDiscoveryTour : public openxds::adt::std::GeneralTour<SourceToken>
{
private:
	openxds::base::String* packageName;
	openxds::base::String* className;
	openxds::base::String* extendsClass;
	openxds::adt::IList<openxds::base::String>& imports;
	bool capture;
public:
	PackageDiscoveryTour( openxds::adt::ITree<astral::tokenizer::SourceToken>& tree,
	                      openxds::adt::IList<openxds::base::String>& imports );
	virtual ~PackageDiscoveryTour();
	
	virtual void  visitPreorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );
	virtual void visitPostorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );
	virtual void  visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r );

	const openxds::base::String&  getPackageName() const;
	const openxds::base::String&    getClassName() const;
	const openxds::base::String& getExtendsClass() const;
	
private:
	virtual void                     setPackageName( openxds::adt::IPosition<SourceToken>& p );
	virtual void                          addImport( openxds::adt::IPosition<SourceToken>& p );
	virtual openxds::base::String* extractSelection( openxds::adt::IPosition<SourceToken>& p );
};
	
};};

#endif
