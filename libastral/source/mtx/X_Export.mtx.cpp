

~!include/astral/Export.h~
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
~

~!source/cplusplus/Export.cpp~
#include "astral/AdvancedHTMLPrintTour.h"
#include "astral/Export.h"
#include "astral/Import.h"
#include "astral/ImportsList.h"

#include "astral/CodeBase.h"
#include "astral/CompilationUnit.h"
#include "astral/SymbolDB.h"

#include <astral.ast/AST.h>
#include "astral.tours/HTMLPrintTour.h"
#include <astral.tokenizer/SourceToken.h>

#include <openxds.adt/IDictionary.h>
#include <openxds.adt/IEIterator.h>
#include <openxds.adt/IEntry.h>
#include <openxds.adt/IIterator.h>
#include <openxds.adt/IList.h>
#include <openxds.adt/ITree.h>
#include <openxds.base/Environment.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.io/File.h>
#include <openxds.io/FileOutputStream.h>
#include <openxds.io/PrintWriter.h>
#include <openxds.io/Path.h>

using namespace astral;
using namespace astral::ast;
using namespace astral::tokenizer;
using namespace openxds::adt;
using namespace openxds::base;
using namespace openxds::io;

static PrintWriter* createPrintWriter( const String& pathname );

void
Export::printExposedSymbols( const CodeBase& cb )
{
	IEIterator<const IEntry<CompilationUnit> >* ie = cb.getSymbolDB().getSymbols().entries();
	while ( ie->hasNext() )
	{
		IEntry<const IEntry<CompilationUnit> >* entry = ie->next();
		{
			//fprintf( stdout, "%s\n", entry->getKey() );
		}
		delete entry;
	}
	delete ie;
}

void
Export::toXML( const CodeBase& cb )
{
	fprintf( stdout, "<names>\n" );
	{
		IEIterator<String>* ie = cb.getSymbolDB().getN2NS().entries();
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
		IEIterator<String>* ie = cb.getSymbolDB().getNS2N().entries();
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
		IEIterator<const IEntry<CompilationUnit> >* ie = cb.getSymbolDB().getSymbols().entries();
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

	const IEIterator<CompilationUnit>* ie = cb.getFiles().entries();
	{
		while ( ie->hasNext() )
		{
			const IEntry<CompilationUnit>* entry = ie->next();
			{
				//const char*            filename = entry->getKey();
				const CompilationUnit& cu       = entry->getValue();
				const ImportsList&   imports  = cu.getImportsList();

				IDictionary<const IEntry<CompilationUnit> >* imported_symbols = cb.getSymbolDB().importedSymbols( imports );
				IDictionary<String>* imported_types = cb.getSymbolDB().importedTypes( imports, cu.getNamespace() );

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
Export::exportHTMLTo( const char* directory, const CodeBase& cb )
{
	const IEIterator<CompilationUnit>* ie = cb.getFiles().entries();
	{
		while ( ie->hasNext() )
		{
			const IEntry<CompilationUnit>* entry = ie->next();
			{
				const CompilationUnit& cu  = entry->getValue();
				exportHTMLTo( directory, cb, cu );
			}
			delete entry;
		}
	}
	delete ie;
}

void
Export::exportHTMLTo( const char* directory, const CodeBase& cb, const CompilationUnit& cu )
{
	FormattedString pathname( "%s/%s.%s.html", directory, cu.getNamespace().getChars(), cu.getName().getChars() );
	{
		Path path( pathname );
		fprintf( stdout, "\n%s\n", path.getAbsolute().getChars() );
	}
	PrintWriter* aWriter = createPrintWriter( pathname );
	{
		const ImportsList& imports = cu.getImportsList();

		IDictionary<const IEntry<CompilationUnit> >* imported_symbols = cb.getSymbolDB().importedSymbols( imports );
		IDictionary<String>* imported_types = cb.getSymbolDB().importedTypes( imports, cu.getNamespace() );
		{
			top( *aWriter );
			printHTML( cu, cb, *imported_symbols, *imported_types, *aWriter );
			bottom( *aWriter );
		}
		delete imported_symbols;
		delete imported_types;
	}
	delete aWriter;
}

void
Export::printHTML( const CompilationUnit& cu, const CodeBase& cb, IDictionary<const IEntry<CompilationUnit> >& symbols, IDictionary<String>& importedTypes, PrintWriter& writer )
{
	ITree<SourceToken>&     ast  = cu.getAST().getTree();
	IPosition<SourceToken>* root = ast.root();
	{
		astral::tours::HTMLPrintTour* print_tour = new astral::tours::HTMLPrintTour( ast, writer );
		print_tour->doGeneralTour( *root );
		delete print_tour;
	}
	delete root;
}




void
Export::exportAdvancedHTMLTo( const char* directory, const CodeBase& cb )
{
	const IEIterator<CompilationUnit>* ie = cb.getFiles().entries();
	{
		while ( ie->hasNext() )
		{
			const IEntry<CompilationUnit>* entry = ie->next();
			{
				//const char* filename = entry->getKey();
				const CompilationUnit& cu  = entry->getValue();
				exportAdvancedHTMLTo( directory, cb, cu );
			}
			delete entry;
		}
	}
	delete ie;
}

void
Export::exportAdvancedHTMLTo( const char* directory, const CodeBase& cb, const CompilationUnit& cu )
{
	FormattedString pathname( "%s/%s.%s.html", directory, cu.getNamespace().getChars(), cu.getName().getChars() );
	{
		Path path( pathname );
		fprintf( stdout, "\n%s\n", path.getAbsolute().getChars() );
	}
	PrintWriter* aWriter = createPrintWriter( pathname );
	{
		const ImportsList& imports = cu.getImportsList();
//		IDictionary<IEntry<CompilationUnit> >* imported_symbols = cb.processImports( imports );
//		IDictionary<String>* imported_types = cb.importedTypes( imports );

		IDictionary<const IEntry<CompilationUnit> >* imported_symbols = cb.getSymbolDB().importedSymbols( imports );
		IDictionary<String>* imported_types = cb.getSymbolDB().importedTypes( imports, cu.getNamespace() );
		{
			top( *aWriter );
			printAdvancedHTML( cu, cb, *imported_symbols, *imported_types, *aWriter );
			bottom( *aWriter );
		}
		delete imported_symbols;
		delete imported_types;
	}
	delete aWriter;
}

void
Export::printAdvancedHTML( const CompilationUnit& cu, const CodeBase& cb, IDictionary<const IEntry<CompilationUnit> >& symbols, IDictionary<String>& importedTypes, PrintWriter& writer )
{
	ITree<SourceToken>&     ast  = cu.getAST().getTree();
	IPosition<SourceToken>* root = ast.root();
	{
		AdvancedHTMLPrintTour* print_tour = new AdvancedHTMLPrintTour( ast, cu, cb, writer );
		print_tour->doGeneralTour( *root );
		delete print_tour;
	}
	delete root;
}

openxds::base::String*
Export::toXML( const CompilationUnit& cu, openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& iSymbols, openxds::adt::IDictionary<openxds::base::String>& iTypes )
{
	StringBuffer sb;
	{
		const char* packageName  = cu.getNamespace().getChars();
		const char* className    = cu.getName().getChars();
		const char* extendsClass = cu.getSuperclass().getChars();
	
		FormattedString tag( "\n<compilationUnit package='%s' class='%s' extends='%s'>\n", packageName, className, extendsClass );
		sb.append( tag );
		{
			{
				const IIterator<Import>* it = cu.getImportsList().iterator();
				{
					while ( it->hasNext() )
					{
						const Import& im = it->next();
						FormattedString import( "<import value='%s' />\n", im.getImport().getChars() );
						sb.append( import );
					
//						IPosition<String>* p = it->next();
//						{
//							FormattedString import( "<import value='%s' />\n", p->getElement().getChars() );
//							sb.append( import );
//						}
//						delete p;
					}
				}
				delete it;
			}
			{
				sb.append( "<importedTypes>\n" );
				IEIterator<String>* ie = iTypes.entries();
				{
					while ( ie->hasNext() )
					{
						IEntry<String>* e = ie->next();
						{
							FormattedString iType( "<importedType value='%s.%s' />\n", e->getValue().getChars(), e->getKey() );
							sb.append( iType );
						}
						delete e;
					}
				}
				delete ie;
				sb.append( "</importedTypes>\n" );
			}
			{
				sb.append( "<importedSymbols>\n" );
				IEIterator<const IEntry<CompilationUnit> >* ie = iSymbols.entries();
				{
					while ( ie->hasNext() )
					{
						IEntry<const IEntry<CompilationUnit> >* e = ie->next();
						{
							FormattedString iSymbol( "<importedSymbol value='%s' />\n", e->getKey() );
							sb.append( iSymbol );
						}
						delete e;
					}
				}
				delete ie;
				sb.append( "</importedSymbols>\n" );
			}
			{
				IEIterator<IPosition<SourceToken> >* ie = cu.getMethods().entries();
				{
					while ( ie->hasNext() )
					{
						IEntry<IPosition<SourceToken> >* entry = ie->next();
						{
							FormattedString method( "<method signature='%s' />\n", entry->getKey() );
							sb.append( method );
						}
						delete entry;
					}
				}
				delete ie;
			}
			{
				IEIterator<IPosition<SourceToken> >* ie = cu.getMembers().entries();
				{
					while ( ie->hasNext() )
					{
						IEntry<IPosition<SourceToken> >* entry = ie->next();
						{
							FormattedString member( "<member signature='%s' />\n", entry->getKey() );
							sb.append( member );
						}
						delete entry;
					}
				}
				delete ie;
			}
		}
		sb.append( "</compilationUnit>" );
	}
	return sb.asString();
}

void
Export::printMethods( const CompilationUnit& cu )
{
	IEIterator<IPosition<SourceToken> >* ie = cu.getMethods().entries();
	{
		while ( ie->hasNext() )
		{
			IEntry<IPosition<SourceToken> >* entry = ie->next();
			{
				fprintf( stdout, "%s\n", entry->getKey() );
			}
			delete entry;
		}
	
	}
	delete ie;
}

static PrintWriter* createPrintWriter( const String& pathname )
{
	File* file;
	PrintWriter* aWriter = new PrintWriter( new FileOutputStream( (file = new File( new Path( pathname ) )) ) );
	file->open( "w+" );

	return aWriter;
} 
~