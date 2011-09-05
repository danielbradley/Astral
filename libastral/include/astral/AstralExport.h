#ifndef ASTRAL_ASTRALEXPORT_H
#define ASTRAL_ASTRALEXPORT_H

#include "astral/Astral.h"
#include "astral.h"
#include <openxds.base.h>
#include <openxds.adt.h>
#include <openxds/Object.h>

namespace astral {
	
class AstralExport : public Astral
{
public:
	AstralExport();
	virtual ~AstralExport();

	virtual void printExposedSymbols() const;

	virtual void        exportHTMLTo( const char* directory ) const;
	virtual void        exportHTMLTo( const char* directory, const CompilationUnit& cu ) const;
	virtual void               toXML() const;
};

};

#endif