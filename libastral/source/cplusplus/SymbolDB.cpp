#include "astral/CompilationUnit.h"
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
	this->symbols        = new Dictionary<const IEntry<CompilationUnit> >();
	this->name2namespace = new Dictionary<String>();
	this->namespace2name = new Dictionary<String>();
}

SymbolDB::~SymbolDB()
{
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
		delete this->name2namespace->remove( this->name2namespace->find( nm ) );
		delete this->namespace2name->remove( this->namespace2name->find( ns ) );
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}

	//cu.deregisterSymbols( *this->symbols, anEntry );
}

IDictionary<String>*
SymbolDB::importedTypes( const IList<String>& imports ) const
{
	IDictionary<String>* imported_types = new Dictionary<String>();
	{
		const IPIterator<String>* it = imports.positions();
		while ( it->hasNext() )
		{
			const IPosition<String>* p = it->next();
			{
				const char* nspace = p->getElement().getChars(); 
				//fprintf( stdout, "Astral::importTypes: Looking for classes matching: %s\n", nspace );
				
				IEIterator<String>* ie = this->namespace2name->findAll( nspace );
				while ( ie->hasNext() )
				{
					IEntry<String>* e = ie->next();
					{
						const String& value = e->getValue();
						delete imported_types->insert( value.getChars(), new String( nspace ) );
						//fprintf( stdout, "Astral::importTypes: %s\n", value.getChars() );
					}
					delete e;
				}
				delete ie;
			}
			delete p;
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
SymbolDB::importedSymbols( const IList<String>& imports ) const
{
	IDictionary<const IEntry<CompilationUnit> >* subset = new Dictionary<const IEntry<CompilationUnit> >();
	{
		const IPIterator<String>* it = imports.positions();
		while ( it->hasNext() )
		{
			const IPosition<String>* p = it->next();
			{
				const String& imprt = p->getElement();
				
				copySymbolsFor( *subset, *this->symbols, imprt );
			}
			delete p;
		}
		delete it;
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

