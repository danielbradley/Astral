#ifndef ASTRAL_EXPORT_H
#define ASTRAL_EXPORT_H

#include "astral.h"
#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds.io.h>

namespace astral {

class Export
{
public:
	static void printExposedSymbols( const CodeBase& cb );

	static void toXML( const CodeBase& cb );

	static void exportHTMLTo( const char* directory, const CodeBase& cb );
	static void exportHTMLTo( const char* directory, const CodeBase& cb, const CompilationUnit& cu );
	static void printHTML( const CompilationUnit& cu,
	                       const CodeBase& cb,
	                       openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& symbols,
	                       openxds::adt::IDictionary<openxds::base::String>& importedTypes,
						   openxds::io::PrintWriter& writer );

	static void exportAdvancedHTMLTo( const char* directory, const CodeBase& cb );
	static void exportAdvancedHTMLTo( const char* directory, const CodeBase& cb, const CompilationUnit& cu );



	static void printAdvancedHTML( const CompilationUnit& cu,
	                       const CodeBase& cb,
	                       openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& symbols,
	                       openxds::adt::IDictionary<openxds::base::String>& importedTypes,
						   openxds::io::PrintWriter& writer );
	
	static openxds::base::String* toXML( const CompilationUnit& cu, openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& iSymbols, openxds::adt::IDictionary<openxds::base::String>& iTypes );
	
	static void printMethods( const CompilationUnit& cu );
//	virtual void printMembers( const CompilationUnit& cu );
};

};

#endif

