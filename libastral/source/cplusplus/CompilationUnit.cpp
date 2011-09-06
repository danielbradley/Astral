#include <astral/AdvancedHTMLPrintTour.h>
#include <astral/CompilationUnit.h>
#include <astral.ast/AST.h>
#include <astral.tours/HTMLPrintTour.h>
#include <astral.tours/MemberDiscoveryTour.h>
#include <astral.tours/MethodDiscoveryTour.h>
#include <astral.tours/PackageDiscoveryTour.h>
#include <astral.tokenizer/SourceToken.h>
#include <openxds.io/PrintWriter.h>
#include <openxds.adt/IEIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.base/StringTokenizer.h>
#include <openxds.base/FormattedString.h>

using namespace astral;
using namespace astral::ast;
using namespace astral::tours;
using namespace astral::tokenizer;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::exceptions;
using namespace openxds::io;
using namespace openxds::io::exceptions;

CompilationUnit::CompilationUnit( const char* location )
{
	this->imports      = new Sequence<String>();
	this->methods      = new Dictionary<IPosition<SourceToken> >();
	this->members      = new Dictionary<IPosition<SourceToken> >();
	this->ast          = new AST( location );
	this->packageName  = NULL;
	this->className    = NULL;
	this->extendsClass = NULL;
}

CompilationUnit::~CompilationUnit()
{
	delete this->methods;
	delete this->members;
	delete this->ast;
	delete this->packageName;
	delete this->className;
	delete this->extendsClass;
}

void
CompilationUnit::initialise()
{
	this->ast->parseFile();

	IPosition<SourceToken>* root = this->ast->getTree().root();
	{
		PackageDiscoveryTour pdt( this->ast->getTree(), *this->imports );
		pdt.doGeneralTour( *root );
		this->packageName  = pdt.getPackageName().asString();
		this->className    = pdt.getClassName().asString();
		this->extendsClass = pdt.getExtendsClass().asString();

		MethodDiscoveryTour mdt1( this->ast->getTree(), *this->methods );
		mdt1.doGeneralTour( *root );
		
		MemberDiscoveryTour mdt2( this->ast->getTree(), *this->members );
		mdt2.doGeneralTour( *root );
	}
	delete root;
}

void
CompilationUnit::registerSymbols( IDictionary<IEntry<CompilationUnit> >& symbols, const IEntry<CompilationUnit>& e ) const
{
	IEIterator<IPosition<SourceToken> >* ie = this->methods->entries();
	{
		while ( ie->hasNext() )
		{
			IEntry<IPosition<SourceToken> >* entry = ie->next();
			{
				const char* key = entry->getKey();
				StringTokenizer st( key );
				st.setDelimiter( '|' );
				String* name = st.nextToken();
				{
					StringBuffer sb;
					sb.append( this->getNamespace() );
					sb.append( '.' );
					sb.append( this->getName() );
					sb.append( '.' );
					sb.append( *name );

					delete symbols.insert( sb.getChars(), e.copy() );
				}
				delete name;
			}
			delete entry;
		}
	}
	delete ie;

	ie = this->members->entries();
	{
		while ( ie->hasNext() )
		{
			IEntry<IPosition<SourceToken> >* entry = ie->next();
			{
				StringTokenizer st( entry->getKey() );
				st.setDelimiter( '|' );
				if ( st.hasMoreTokens() )
				{
					String* name = st.nextToken();
					{
						StringBuffer sb;
						sb.append( this->getNamespace() );
						sb.append( '.' );
						sb.append( this->getName() );
						sb.append( '.' );
						sb.append( *name );
					
						delete symbols.insert( sb.getChars(), e.copy() );
					}
					delete name;
				}
			}
			delete entry;
		}
	}
	delete ie;
}

IDictionary<IPosition<SourceToken> >&
CompilationUnit::getMethods() const
{
	return *this->methods;
}

IDictionary<IPosition<SourceToken> >&
CompilationUnit::getMembers() const
{
	return *this->members;
}

String*
CompilationUnit::resolveMemberType( const char* name ) const
{
	String* ret = new String( "" );
	try
	{
		IEntry<IPosition<SourceToken> >* e = this->members->startsWith( name );
		{
			StringTokenizer st( e->getKey() );
			st.setDelimiter( '|' );
			if ( st.hasMoreTokens() ) delete st.nextToken();
			if ( st.hasMoreTokens() )
			{
				delete ret;
				ret = st.nextToken();
			}
		}
		delete e;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}
	return ret;
}

String*
CompilationUnit::resolveMethodType( const char* name ) const
{
	String* ret = new String( "" );
	try
	{
		IEntry<IPosition<SourceToken> >* e = this->methods->startsWith( name );
		{
			StringTokenizer st( e->getKey() );
			st.setDelimiter( '|' );
			if ( st.hasMoreTokens() ) delete st.nextToken();
			if ( st.hasMoreTokens() )
			{
				delete ret;
				ret = st.nextToken();
			}
		}
		delete e;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}
	return ret;
}

String&
CompilationUnit::getNamespace() const
{
	return *this->packageName;
}

String&
CompilationUnit::getName() const
{
	return *this->className;
}

String&
CompilationUnit::getSuperclass() const
{
	return *this->extendsClass;
}

IList<String>&
CompilationUnit::getImports() const
{
	return *this->imports;
}

void
CompilationUnit::printHTML( IDictionary<IEntry<CompilationUnit> >& symbols, IDictionary<String>& importedTypes, PrintWriter& writer ) const
{
	ITree<SourceToken>&     ast  = this->ast->getTree();
	IPosition<SourceToken>* root = ast.root();
	{
		AdvancedHTMLPrintTour* print_tour = new AdvancedHTMLPrintTour( ast, *this, symbols, importedTypes, writer );
		print_tour->doGeneralTour( *root );
		delete print_tour;
	}
	delete root;
}

String*
CompilationUnit::toXML( IDictionary<IEntry<CompilationUnit> >& iSymbols, IDictionary<String>& iTypes )
{
	StringBuffer sb;
	{
		const char* packageName  = this->packageName->getChars();
		const char* className    = this->className->getChars();
		const char* extendsClass = this->extendsClass->getChars();
	
		FormattedString tag( "\n<compilationUnit package='%s' class='%s' extends='%s'>\n", packageName, className, extendsClass );
		sb.append( tag );
		{
			{
				IPIterator<String>* it = this->imports->positions();
				{
					while ( it->hasNext() )
					{
						IPosition<String>* p = it->next();
						{
							FormattedString import( "<import value='%s' />\n", p->getElement().getChars() );
							sb.append( import );
						}
						delete p;
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
				IEIterator<IEntry<CompilationUnit> >* ie = iSymbols.entries();
				{
					while ( ie->hasNext() )
					{
						IEntry<IEntry<CompilationUnit> >* e = ie->next();
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
				IEIterator<IPosition<SourceToken> >* ie = this->methods->entries();
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
				IEIterator<IPosition<SourceToken> >* ie = this->members->entries();
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
CompilationUnit::printMethods() const
{
	IEIterator<IPosition<SourceToken> >* ie = this->methods->entries();
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

void
CompilationUnit::printMembers() const
{}