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
	virtual void add( const openxds::base::String& path );

protected:
	virtual const SymbolDB& getSymbolDB() const { return *this->symbolDB; }

	openxds::adt::IDictionary<CompilationUnit>& getFiles() const { return *this->files; }
};

};

#endif