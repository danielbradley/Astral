#include "astral/CompilationUnit.h"
#include "astral/Import.h"
#include "astral/ImportsList.h"
#include "astral/SymbolDB.h"

#include <openxds.adt/IEntry.h>
#include <openxds.adt/IList.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.base/String.h>

using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::exceptions;

SymbolDB::SymbolDB()
{
	this->classes        = new Dictionary<const IEntry<CompilationUnit> >();
	this->symbols        = new Dictionary<const IEntry<CompilationUnit> >();
	this->name2namespace = new Dictionary<String>();
	this->namespace2name = new Dictionary<String>();
}

SymbolDB::~SymbolDB()
{
	delete this->classes;
	delete this->symbols;
	delete this->name2namespace;
	delete this->namespace2name;
}

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
	Dictionary<String>* imported_types = new Dictionary<String>();
	{
		imported_types->setThrowExceptions( FALSE );
	
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

const CompilationUnit&
SymbolDB::getCompilationUnitForSymbolPrefix( const char* prefix ) const
throw (NoSuchElementException*)
{
	const IEntry<const IEntry<CompilationUnit> >* entry = this->getSymbols().startsWith( prefix );
	const CompilationUnit& cu = entry->getValue().getValue();
	delete entry;
	
	return cu;
}

const CompilationUnit&
SymbolDB::getCompilationUnitForSymbol( const char* symbol ) const
throw (NoSuchElementException*)
{
	const IEntry<const IEntry<CompilationUnit> >* entry = this->getSymbols().find( symbol );
	const CompilationUnit& cu = entry->getValue().getValue();
	delete entry;
	
	return cu;
}

