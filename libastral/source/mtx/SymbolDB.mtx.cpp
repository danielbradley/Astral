-	SymbolDB

!
~!include/astral/SymbolDB.h~
#ifndef ASTRAL_SYMBOLDB_H
#define ASTRAL_SYMBOLDB_H

#include "astral.h"
#include <openxds.base.h>
#include <openxds.adt.h>
#include <openxds/Object.h>
#include <openxds.exceptions/NoSuchElementException.h>

namespace astral {
~
!

.	About

The SymbolDB is used to faciliate the storage and interrogation of symbols that have been parsed from the CompilationUnits.
Doing so allows one object reference to be passed to other objects when necessary.




.	Implementation





..		Class definition

The SymbolDB extends from openxds::Object.

~include/astral/SymbolDB.h~
class SymbolDB : openxds::Object {
~




...			Private members

The class contains the following private members:

The /symbols/ dictionary stores a mapping between symbols (methods and members) and
*CompilationUnit* object entries (stored in the /files/ member of the Astral class).
For example:
|
org.example.demo.Demo.initialise(char,Vector) --> IEntry<CompilationUnit>
|

The /name2namespace/ dictionary stores a mapping between classnames and the namespace they were defined within.
The /namespace2name/ dictionary stores the reverse mapping.
For example:
|
org.example.demo.Demo <--> Demo
|



~include/astral/SymbolDB.h~
private: openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >* classes;
private: openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >* symbols;
private: openxds::adt::IDictionary<openxds::base::String>* name2namespace;
private: openxds::adt::IDictionary<openxds::base::String>* namespace2name;
~




...			Construction

The SymbolDB constructor takes no arguments as it simply instantiates the various
private data-structures.


~include/astral/SymbolDB.h~
public: SymbolDB();
public: virtual ~SymbolDB();
~


The SymbolDB is populated by the Astral class, which instantiates *CompilationUnit* objects,
adds them to its own /files/ dictionary, then passes a reference to the corresponding *IEntry*
to the SymbolDB.

If a file is modified, it should be deregistered, then re-registered.

It is often necessary to determine what subset of the namespace is available to classes
depending on the imports specified by a particular class.
The *importedTypes* method returns a Dictionary which is a subset of the /name2namespace/
dictionary.

A similar subset of the /symbols/ table is also required, when looking up symbols.

~include/astral/SymbolDB.h~
public: void registerCU( const openxds::adt::IEntry<CompilationUnit>& anEntry );
public: void deregisterCU( const openxds::adt::IEntry<CompilationUnit>& anEntry );
public: openxds::adt::IDictionary<openxds::base::String>*
          importedTypes( const ImportsList& imports, const openxds::base::String& defaultNamespace ) const;
public: openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >*
          importedSymbols( const ImportsList& imports ) const;
~





...			Accessors

The following provide other classes with direct access to the data structures.

~include/astral/SymbolDB.h~
public: openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& getClasses()       { return *this->classes; }
public: openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& getSymbols()       { return *this->symbols; }
public: openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& getSymbols() const { return *this->symbols; }
public: openxds::adt::IDictionary<openxds::base::String>&                     getN2NS() const { return *this->name2namespace; }
public: openxds::adt::IDictionary<openxds::base::String>&                     getNS2N() const { return *this->namespace2name; }

public: bool                                                             containsType( const char* fqType ) const;
~

~include/astral/SymbolDB.h~
public: const CompilationUnit& getCompilationUnitForSymbolPrefix( const char* prefix ) const
throw (openxds::exceptions::NoSuchElementException*);

public: const CompilationUnit& getCompilationUnitForSymbol( const char* symbol ) const
throw (openxds::exceptions::NoSuchElementException*);
~


!
~include/astral/SymbolDB.h~
};};
~
!

!
~include/astral/SymbolDB.h~
#endif
~
!

..		Class body

!
~!source/cplusplus/SymbolDB.cpp~
#include "astral/CompilationUnit.h"
#include "astral/Import.h"
#include "astral/ImportsList.h"
#include "astral/SymbolDB.h"
~
!

...			Used classes

The SymbolDB class uses the following data structures:

~source/cplusplus/SymbolDB.cpp~
#include <openxds.adt/IEntry.h>
#include <openxds.adt/IList.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.base/String.h>
~

!
~source/cplusplus/SymbolDB.cpp~
using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::exceptions;
~
!

~source/cplusplus/SymbolDB.cpp~
SymbolDB::SymbolDB()
{
	this->classes        = new Dictionary<const IEntry<CompilationUnit> >();
	this->symbols        = new Dictionary<const IEntry<CompilationUnit> >();
	this->name2namespace = new Dictionary<String>();
	this->namespace2name = new Dictionary<String>();
}
~

~source/cplusplus/SymbolDB.cpp~
SymbolDB::~SymbolDB()
{
	delete this->classes;
	delete this->symbols;
	delete this->name2namespace;
	delete this->namespace2name;
}
~

...			Compilation Unit regisration and deregistration

~source/cplusplus/SymbolDB.cpp~
void
SymbolDB::registerCU( const IEntry<CompilationUnit>& anEntry )
{
	const CompilationUnit& cu = anEntry.getValue();
	
	const char* ns = cu.getNamespace().getChars();
	const char* nm = cu.getName().getChars();

	delete this->classes->insert( cu.getFQName().getChars(), anEntry.copy() );
	delete this->name2namespace->insert( nm, new String( ns ) );
	delete this->namespace2name->insert( ns, new String( nm ) );

	cu.registerSymbols( *this->symbols, anEntry );
}
~

~source/cplusplus/SymbolDB.cpp~
void
SymbolDB::deregisterCU( const IEntry<CompilationUnit>& anEntry )
{
	const CompilationUnit& cu = anEntry.getValue();
	
	const char* ns = cu.getNamespace().getChars();
	const char* nm = cu.getName().getChars();
	
	try
	{
		IEntry<String>* e = this->name2namespace->find( nm );
		{
			const String& nspace = e->getValue();
			if ( nspace.contentEquals( ns ) )
			{
				delete this->name2namespace->remove( e );
				e = null;
			}
		}
		delete e;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}

	try
	{
		IEntry<String>* e = this->namespace2name->find( ns );
		{
			const String& name = e->getValue();
			if ( name.contentEquals( nm ) )
			{
				delete this->namespace2name->remove( e );
				e = null;
			}
		}
		delete e;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}

	cu.deregisterSymbols( *this->symbols );
}
~

~source/cplusplus/SymbolDB.cpp~
static String* extractNamespace( const String& anImportLine )
{
	long len = anImportLine.getLength();
	if ( '*' == anImportLine.charAt( len-1 ) )
	{
		return anImportLine.substring( 0, len-3 );
	} else {
		return anImportLine.asString();
	}
}

static void addTypes( IDictionary<String>& importedTypes, IEIterator<String>& ie, const String& nspace )
{
	while ( ie.hasNext() )
	{
		IEntry<String>* e = ie.next();
		{
			const String& value = e->getValue();
			delete importedTypes.insert( value.getChars(), new String( nspace ) );
		}
		delete e;
	}
}


IDictionary<String>*
SymbolDB::importedTypes( const ImportsList& imports, const String& defaultNamespace ) const
{
	IDictionary<String>* imported_types = new Dictionary<String>();
	{
		if ( defaultNamespace.getLength() )
		{
			IEIterator<String>* ie = this->namespace2name->findAll( defaultNamespace.getChars() );
			{
				addTypes( *imported_types, *ie, defaultNamespace );
			}
			delete ie;
		}

		const IIterator<Import>* it = imports.iterator();
		while ( it->hasNext() )
		{
			const Import& import = it->next();
			String* nspace = extractNamespace( import.getImport() );
			{
				IEIterator<String>* ie = this->namespace2name->findAll( nspace->getChars() );
				{
					addTypes( *imported_types, *ie, *nspace );
				}
				delete ie;
			}
			delete nspace;
		}
		delete it;
	}
	return imported_types;
}
~

~source/cplusplus/SymbolDB.cpp~
static void copySymbolsFor
(
	      IDictionary<const IEntry<CompilationUnit> >& subset,
    const IDictionary<const IEntry<CompilationUnit> >& symbols,
	const String& imprt
);

IDictionary<const IEntry<CompilationUnit> >*
SymbolDB::importedSymbols( const ImportsList& imports ) const
{
	IDictionary<const IEntry<CompilationUnit> >* subset = new Dictionary<const IEntry<CompilationUnit> >();
	{
		const IIterator<Import>* it = imports.iterator();
		while ( it->hasNext() )
		{
			const Import& import = it->next();
			
			copySymbolsFor( *subset, *this->symbols, import.getImport() );
		}
		delete it;

//		const IPIterator<String>* it = imports.positions();
//		while ( it->hasNext() )
//		{
//			const IPosition<String>* p = it->next();
//			{
//				const String& imprt = p->getElement();
//				
//				copySymbolsFor( *subset, *this->symbols, imprt );
//			}
//			delete p;
//		}
//		delete it;
	}
	return subset;
}
~

~source/cplusplus/SymbolDB.cpp~
static void copySymbolsFor
(
	      IDictionary<const IEntry<CompilationUnit> >& subset,
    const IDictionary<const IEntry<CompilationUnit> >& symbols,
	const String& imprt
)
{
	const IEIterator<const IEntry<CompilationUnit> >* ie = symbols.entries();
	while ( ie->hasNext() )
	{
		const IEntry<const IEntry<CompilationUnit> >* e = ie->next();
		{
			const char* sym = e->getKey();
			//const char* ip  = imprt.getChars();
		
			String symbol( sym );
			if ( symbol.startsWith( imprt ) )
			{
				delete subset.insert( e->getKey(), e->getValue().copy() );
			}
		}
		delete e;
	}
	delete ie;
}
~

~source/cplusplus/SymbolDB.cpp~
bool
SymbolDB::containsType( const char* fqType ) const
{
	bool contains = FALSE;
	try
	{
		IEntry<const IEntry<CompilationUnit> >* e = this->symbols->startsWith( fqType );
		{
			contains = TRUE;
		}
		delete e;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}
	return contains;
}
~


~source/cplusplus/SymbolDB.cpp~
const CompilationUnit&
SymbolDB::getCompilationUnitForSymbolPrefix( const char* prefix ) const
throw (NoSuchElementException*)
{
	const IEntry<const IEntry<CompilationUnit> >* entry = this->getSymbols().startsWith( prefix );
	const CompilationUnit& cu = entry->getValue().getValue();
	delete entry;
	
	return cu;
}
~



~source/cplusplus/SymbolDB.cpp~
const CompilationUnit&
SymbolDB::getCompilationUnitForSymbol( const char* symbol ) const
throw (NoSuchElementException*)
{
	const IEntry<const IEntry<CompilationUnit> >* entry = this->getSymbols().find( symbol );
	const CompilationUnit& cu = entry->getValue().getValue();
	delete entry;
	
	return cu;
}
~
