
#include "astral/Astral.h"
#include "astral/CompilationUnit.h"
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt/IEntry.h>
#include <openxds.adt/IList.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.base/String.h>

using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;

static void copySymbolsFor( IDictionary<IEntry<CompilationUnit> >& subset, const IDictionary<IEntry<CompilationUnit> >& symbols, const String& imprt );

Astral::Astral()
{
	this->files          = new Dictionary<CompilationUnit>();
	this->symbols        = new Dictionary<IEntry<CompilationUnit> >();
	this->name2namespace = new Dictionary<String>();
	this->namespace2name = new Dictionary<String>();
}

Astral::~Astral()
{
	delete this->files;
	delete this->symbols;
	delete this->name2namespace;
	delete this->namespace2name;
}

void
Astral::add( const String& path )
{
	CompilationUnit* cu = new CompilationUnit( path.getChars() );
	cu->initialise();

	const char* package = cu->getNamespace().getChars();
	const char* cls     = cu->getName().getChars();

	{
		delete this->name2namespace->insert( cls, new String( package ) );
		delete this->namespace2name->insert( package, new String( cls ) );
	}

	{
		IEntry<CompilationUnit>* entry = this->files->insert( path.getChars(), cu );
		cu->registerSymbols( *this->symbols, *entry );
		delete entry;
	}
}

IDictionary<IEntry<CompilationUnit> >*
Astral::processImports( const IList<String>& imports ) const
{
	IDictionary<IEntry<CompilationUnit> >* subset = new Dictionary<IEntry<CompilationUnit> >();
	{
		IPIterator<String>* it = imports.positions();
		while ( it->hasNext() )
		{
			IPosition<String>* p = it->next();
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

IDictionary<String>*
Astral::importedTypes( const IList<String>& imports ) const
{
	IDictionary<String>* imported_types = new Dictionary<String>();
	{
		IPIterator<String>* it = imports.positions();
		while ( it->hasNext() )
		{
			IPosition<String>* p = it->next();
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


static void copySymbolsFor( IDictionary<IEntry<CompilationUnit> >& subset, const IDictionary<IEntry<CompilationUnit> >& symbols, const String& imprt )
{
	IEIterator<IEntry<CompilationUnit> >* ie = symbols.entries();
	while ( ie->hasNext() )
	{
		IEntry<IEntry<CompilationUnit> >* e = ie->next();
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

//void
//Astral::toXML( const String& path )
//{
//	CompilationUnit* cu = new CompilationUnit( path.getChars() );
//	cu->initialise();
//
//
//
//
//	String* xml = cu->toXML();
//	fprintf( stdout, "%s\n", xml->getChars() );
//	delete xml;
//	this->files->insert( path.getChars(), cu );
//}
