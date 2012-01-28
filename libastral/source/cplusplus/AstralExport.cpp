
#include "astral/AstralExport.h"
#include "astral/CompilationUnit.h"
#include "astral/Export.h"
#include "astral/SymbolDB.h"

#include <openxds.io/FileOutputStream.h>
#include <openxds.io/File.h>
#include <openxds.io/Path.h>
#include <openxds.io/PrintWriter.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt/IEntry.h>
#include <openxds.adt/IList.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.base/Environment.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/String.h>

using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::io;

static PrintWriter* createPrintWriter( const String& pathname );

XAstralExport::XAstralExport()
{}

XAstralExport::~XAstralExport()
{}

void
XAstralExport::printExposedSymbols() const
{
	IEIterator<const IEntry<CompilationUnit> >* ie = this->getSymbolDB().getSymbols().entries();
	while ( ie->hasNext() )
	{
		IEntry<const IEntry<CompilationUnit> >* entry = ie->next();
		{
			fprintf( stdout, "%s\n", entry->getKey() );
		}
		delete entry;
	}
	delete ie;
}

static void top( PrintWriter& printer )
{
	String* exe_dir = Environment::executableDirectory();
	{
		FormattedString css( "%s/../share/css/java.css", exe_dir->getChars() );
	
		printer.print( "<html>\n" );
		printer.print( "<head>\n" );
		printer.printf( "<link rel='stylesheet' type='text/css' href='%s'>\n", css.getChars() );
		printer.print( "</head>\n" );
		printer.print( "<body>\n" );
	}
	delete exe_dir;
}

static void bottom( PrintWriter& printer )
{
	printer.print( "</html>" );
}

void
XAstralExport::exportHTMLTo( const char* directory ) const
{
	const IEIterator<CompilationUnit>* ie = this->getFiles().entries();
	{
		while ( ie->hasNext() )
		{
			const IEntry<CompilationUnit>* entry = ie->next();
			{
				//const char* filename = entry->getKey();
				const CompilationUnit& cu  = entry->getValue();
				this->exportHTMLTo( directory, cu );
			}
			delete entry;
		}
	}
	delete ie;
}

void
XAstralExport::exportHTMLTo( const char* directory, const CompilationUnit& cu ) const
{
	FormattedString pathname( "%s/%s.%s.html", directory, cu.getNamespace().getChars(), cu.getName().getChars() );
	{
		Path path( pathname );
		fprintf( stdout, "%s\n", path.getAbsolute().getChars() );
	}
	PrintWriter* aWriter = createPrintWriter( pathname );
	{
		const IList<String>& imports = cu.getImports();
//		IDictionary<IEntry<CompilationUnit> >* imported_symbols = this->processImports( imports );
//		IDictionary<String>* imported_types = this->importedTypes( imports );

		IDictionary<const IEntry<CompilationUnit> >* imported_symbols = this->getSymbolDB().importedSymbols( imports );
		IDictionary<String>* imported_types = this->getSymbolDB().importedTypes( imports );
		{
			top( *aWriter );
			//Export::printHTML( cu, this, *imported_symbols, *imported_types, *aWriter );
			bottom( *aWriter );
		}
		delete imported_symbols;
		delete imported_types;
	}
	delete aWriter;
}

static PrintWriter* createPrintWriter( const String& pathname )
{
	File* file;
	PrintWriter* aWriter = new PrintWriter( new FileOutputStream( (file = new File( new Path( pathname ) )) ) );
	file->open( "w+" );

	return aWriter;
} 

void
XAstralExport::toXML() const
{
	fprintf( stdout, "<names>\n" );
	{
		IEIterator<String>* ie = this->getSymbolDB().getN2NS().entries();
		while ( ie->hasNext() )
		{
			IEntry<String>* e = ie->next();
			{
				const char*   name = e->getKey();
				const String& ns   = e->getValue();
				fprintf( stdout, "<name name='%s' namespace='%s' />\n", name, ns.getChars() );
			}
			delete e;
		}
		delete ie;
	}
	fprintf( stdout, "</names>\n" );

	fprintf( stdout, "<namespaces>\n" );
	{
		IEIterator<String>* ie = this->getSymbolDB().getNS2N().entries();
		while ( ie->hasNext() )
		{
			IEntry<String>* e = ie->next();
			{
				const char*   ns   = e->getKey();
				const String& name = e->getValue();
				fprintf( stdout, "<namespace namespace='%s' name='%s' />\n", ns, name.getChars() );
			}
			delete e;
		}
		delete ie;
	}
	fprintf( stdout, "</namespaces>\n" );

	fprintf( stdout, "<symbols>\n" );
	{
		IEIterator<const IEntry<CompilationUnit> >* ie = this->getSymbolDB().getSymbols().entries();
		while ( ie->hasNext() )
		{
			IEntry<const IEntry<CompilationUnit> >* e = ie->next();
			{
				const char*   name = e->getKey();
				fprintf( stdout, "<symbol name='%s' />\n", name );
			}
			delete e;
		}
		delete ie;
	}
	fprintf( stdout, "</symbols>\n" );

	const IEIterator<CompilationUnit>* ie = this->getFiles().entries();
	{
		while ( ie->hasNext() )
		{
			const IEntry<CompilationUnit>* entry = ie->next();
			{
				//const char*            filename = entry->getKey();
				const CompilationUnit& cu       = entry->getValue();
				const IList<String>&   imports  = cu.getImports();

				IDictionary<const IEntry<CompilationUnit> >* imported_symbols = this->getSymbolDB().importedSymbols( imports );
				IDictionary<String>* imported_types = this->getSymbolDB().importedTypes( imports );

				String* xml = Export::toXML( cu, *imported_symbols, *imported_types );
				{
					fprintf( stdout, "%s\n", xml->getChars() );
				}
				delete xml;
			}
			delete entry;
		}
	}
	delete ie;
}