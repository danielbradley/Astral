#ifndef ASTRAL_ASTRAL_H
#define ASTRAL_ASTRAL_H

#include "astral.h"
#include <openxds.base.h>
#include <openxds.adt.h>
#include <openxds/Object.h>

namespace astral {
	
class Astral : openxds::Object
{
private:
	/**
	 *	This dictionary maps filenames to parsed CompilationUnit objects.
	 */
	openxds::adt::IDictionary<CompilationUnit>* files;

	SymbolDB* symbolDB;

public:
	Astral();
	virtual ~Astral();

	/**
	 *	This method causes a CompilationUnit to be parsed from a source file.
	 */ 
	virtual       void               addBaseDirectory( const openxds::base::String& path );
	virtual       void                  addSourceFile( const openxds::base::String& path );

	virtual       openxds::adt::IDictionary<CompilationUnit>&           getFiles()                                                     { return *this->files; }
	virtual const openxds::adt::IDictionary<CompilationUnit>&           getFiles()                                               const { return *this->files; }
	virtual const CompilationUnit&                            getCompilationUnit( const openxds::base::String& methodSignature ) const;
	virtual const CompilationUnit&                            getCompilationUnit( const MethodSignature& aMethodSignature ) const;

protected:

	virtual       openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& getCompilationUnits();
	virtual const openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& getCompilationUnits() const;

	virtual       SymbolDB& getSymbolDB()       { return *this->symbolDB; }
	virtual const SymbolDB& getSymbolDB() const { return *this->symbolDB; }

};

};

#endif