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

	/**
	 *	This dictionary maps symbols exposed by CompilationUnit objects to
	 *	their entry in the "files" dictionary.
	 */
	openxds::adt::IDictionary<openxds::adt::IEntry<CompilationUnit> >* symbols;
	openxds::adt::IDictionary<openxds::base::String>* name2namespace;
	openxds::adt::IDictionary<openxds::base::String>* namespace2name;

public:
	Astral();
	virtual ~Astral();

	/**
	 *	This method causes a CompilationUnit to be parsed from a source file.
	 */ 
	virtual void add( const openxds::base::String& path );

	/**
	 *	Returns a dictionary that contains a subset of the symbols in the
	 *  "symbols" dictionary, representing those symbols available to the
	 *  specified imports.
	 */
	openxds::adt::IDictionary<openxds::adt::IEntry<CompilationUnit> >* processImports( const openxds::adt::IList<openxds::base::String>& imports ) const;

	/**
	 *	Returns a name to namespace mapping for those types imported.
	 */
	openxds::adt::IDictionary<openxds::base::String>* importedTypes( const openxds::adt::IList<openxds::base::String>& imports ) const;

	/**
	 *	This method searches the symbol dictionary for symbols imported by a compilation unit.
	 */

//	openxds::adt::IList<openxds::base::String>& getAllNamespaces();
//	const IList*    getNamespaces( const openxds::base::String& nspace ) const;
//	const IList*       getClasses( const openxds::base::String& nspace ) const;
//	const IList*       getMethods( const openxds::base::String& cls )    const;

protected:
	openxds::adt::IDictionary<CompilationUnit>&                          getFiles() const { return *this->files; }
	openxds::adt::IDictionary<openxds::adt::IEntry<CompilationUnit> >& getSymbols() const { return *this->symbols; }
	openxds::adt::IDictionary<openxds::base::String>&                     getN2NS() const { return *this->name2namespace; }
	openxds::adt::IDictionary<openxds::base::String>&                     getNS2N() const { return *this->namespace2name; }

};

};

#endif