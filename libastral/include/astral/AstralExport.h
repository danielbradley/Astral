#ifndef ASTRAL_ASTRALEXPORT_H
#define ASTRAL_ASTRALEXPORT_H

#include "astral/CodeBase.h"
#include "astral.h"
#include <openxds.base.h>
#include <openxds.adt.h>
#include <openxds/Object.h>

namespace astral {
	
class XAstralExport : public CodeBase
{
public:
	XAstralExport();
	virtual ~XAstralExport();

	virtual void printExposedSymbols() const;

	virtual void        exportHTMLTo( const char* directory ) const;
	virtual void        exportHTMLTo( const char* directory, const CompilationUnit& cu ) const;
	virtual void               toXML() const;
};

};

#endif