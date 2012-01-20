#include <astral/AdvancedHTMLPrintTour.h>
#include <astral/Codebase.h>
#include <astral/CompilationUnit.h>
#include <astral/MethodSignature.h>
#include <astral/SymbolDB.h>
#include <astral/VariableScopes.h>
#include <astral.ast/AST.h>
#include <astral.tours/HTMLPrintTour.h>
#include <astral.tours/MemberDiscoveryTour.h>
#include <astral.tours/MethodDiscoveryTour.h>
#include <astral.tours/PackageDiscoveryTour.h>
#include <astral.tours/PrintSourceTour.h>
#include <astral.tokenizer/SourceToken.h>
#include <openxds.io/PrintWriter.h>
#include <openxds.io/OutputStream.h>
#include <openxds.io/IOBuffer.h>
#include <openxds.adt/IEIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.base/StringTokenizer.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/Math.h>

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

static String* searchForNameInLocalScopes( const char* name, const ISequence<IDictionary<String > >& scopes );
static String*                  translate(       String* aString );
static long              determineMinTabs( const String& aString );

CompilationUnit::CompilationUnit( const char* location )
{
	this->location     = new String( location );
	this->imports      = new Sequence<String>();
	this->methods      = new Dictionary<IPosition<SourceToken> >();
	this->members      = new Dictionary<IPosition<SourceToken> >();
	this->ast          = new AST( location );
	this->packageName  = NULL;
	this->className    = NULL;
	this->extendsClass = NULL;
	
	this->importedTypes = new Dictionary<String>();
}

CompilationUnit::~CompilationUnit()
{
	delete this->methods;
	delete this->members;
	delete this->ast;
	delete this->packageName;
	delete this->className;
	delete this->extendsClass;
	
	delete this->importedTypes;
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
		this->fqName       = new FormattedString( "%s.%s", this->packageName->getChars(), this->className->getChars() );
		this->extendsClass = pdt.getExtendsClass().asString();

		MethodDiscoveryTour mdt1( this->ast->getTree(), *this->methods );
		mdt1.doGeneralTour( *root );
		
		MemberDiscoveryTour mdt2( this->ast->getTree(), *this->members );
		mdt2.doGeneralTour( *root );
		
		this->imports->insertLast( new String( this->getNamespace() ) );
	}
	delete root;
}

void
CompilationUnit::resetImportedTypes( const SymbolDB& symbolDB )
{
	delete this->importedTypes;
	this->importedTypes = symbolDB.importedTypes( this->getImports() );
}

void
CompilationUnit::registerSymbols( IDictionary<const IEntry<CompilationUnit> >& symbols, const IEntry<CompilationUnit>& e ) const
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

					//fprintf( stdout, "CU::registerSymbols: symbols.insert( \"%s\", e )\n", sb.getChars() );

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

String*
CompilationUnit::resolveFQTypeOfName( const char* name, const VariableScopes& scopes ) const
{
	String* type = this->resolveTypeOfName( name, scopes );

	if ( ! type->contentEquals( "" ) )
	{
		const char* _type = type->getChars();

		String* fq_type = this->resolveFQTypeOfType( _type );
		if ( ! fq_type->contentEquals( "" ) )
		{
			delete type;
			type = new String( *fq_type );
		}
		delete fq_type;
	}
	
	//fprintf( stderr, "CompilationUnit::resolveFQTypeOfName( %s, scopes ) | %s\n", name, type->getChars() );
	
	return type;
}

String*
CompilationUnit::resolveFQTypeOfType( const char* type ) const
{
	StringBuffer sb;

	try
	{
		const IEntry<String>* e = this->importedTypes->find( type );
		{
			sb.append( e->getValue() );
			sb.append( "." );
			sb.append( type );
		}
		delete e;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}

	return sb.asString();
}

String*
CompilationUnit::resolveTypeOfName( const char* name, const VariableScopes& scopes ) const
{
	String* type = NULL;

	String _name( name );
	if ( _name.contentEquals( "this" ) )
	{
		type = new String( this->getName() );
	}
	else
	{
		type = scopes.searchForTypeOfName( name );
		if ( type->contentEquals( "" ) )
		{
			delete type;
			type = this->resolveMemberType( name );
		}
	}

	//fprintf( stderr, "CompilationUnit::resolveTypeOfName( %s, scopes ) | %s\n", name, type->getChars() );

	return type;
}

//openxds::base::String* searchForNameInLocalScopes( const char* name, const VariableScopes& scopes )
//{
//	String* type = new String();
//
//	long nr = scopes.size() - 1;
//	while ( 0 <= nr )
//	{
//		try
//		{
//			const IEntry<String>* e = scopes.get( (int) nr ).find( name );
//			{
//				delete type;
//				type = new String( e->getValue() );
//			}
//			delete e;
//			nr = -1;
//		}
//		catch ( NoSuchElementException* ex )
//		{
//			delete ex;
//		}
//		nr--;
//	}
//
//	return type;
//}

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

String*
CompilationUnit::resolveMethodCallParametersToTypes( const String& parameters, const VariableScopes& scopes ) const
{
	StringBuffer sb;
	{
		StringTokenizer st( parameters );
		st.setDelimiter( ',' );
		
		while ( st.hasMoreTokens() )
		{
			String* token = st.nextToken();
			{
				if ( token->contentEquals( "VALUE" ) || token->contentEquals( "QUOTE" ) || token->contentEquals( "CHAR" ) )
				{
					sb.append( *token );
					sb.append( ',' );
				}
				else
				{
					const char* name = token->getChars();
					String* type = this->resolveTypeOfName( name, scopes );
					{
						sb.append( *type );
						sb.append( ',' );
					}
					delete type;
				}
			}
			delete token;
		}
	}
	sb.removeLast();

//	fprintf( stderr, "CompilationUnit::resolveMethodCallParametersToTypes( \"%s\", scopes ) | \"%s\"\n", parameters.getChars(), sb.getChars() );

	return sb.asString();
}

String*
CompilationUnit::retrieveMethodContent( const MethodSignature& aMethodSignature ) const
{
	String* ret = NULL;
	try
	{
		const char* _method = aMethodSignature.getMethodKey().getChars();
		const IEntry<IPosition<SourceToken> >* e_method = this->methods->find( _method );
		{
			const ITree<SourceToken>&     ast      = this->ast->getTree();
			const IPosition<SourceToken>& p_method = e_method->getValue();
			{
				PrintWriter* writer = new PrintWriter( new OutputStream( new IOBuffer() ) );
				PrintSourceTour* print_tour = new PrintSourceTour( ast, *writer );
				{
					print_tour->doGeneralTour( p_method );
					ret = translate( dynamic_cast<IOBuffer&>( writer->getOutputStream().getIOEndPoint() ).toString() );
				}
				delete print_tour;
				delete writer;
			}
		}
		delete e_method;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
		ret = new String();
	}

	return ret;
}

static String* translate( String* aString )
{
	long min_tabs = determineMinTabs( *aString );
	
	StringBuffer sb;
	{
		      long  t      = 0;
		      long  max    = aString->getLength();
		const char* string = aString->getChars();
		for ( long i=0; i < max; i++ )
		{
			char ch = string[i];
			switch ( ch )
			{
			case '\n':
				sb.append( ch );
				t = min_tabs;
				while ( t && ('\t' == string[i+1]) )
				{
					t--;
					i++;
				}
				break;
			case '\t':
				if ( 0 != i ) sb.append( ch );
				break;
			default:
				sb.append( ch );
			}
		}
	}
	delete aString;
	return sb.asString();
}

static long determineMinTabs( const String& aString )
{
	long min_tabs = 0;
	long tabs     = 0;

	long max      = aString.getLength();
	for ( long i=0; i < max; i++ )
	{
		char ch = aString.charAt( i );
		switch ( ch )
		{
		case '\t':
			tabs++;
			break;
		case '\n':
			min_tabs = Math::min( min_tabs, tabs );
			break;
		}
	}
	return Math::max( 1, min_tabs );
}

String*
CompilationUnit::resolveMethodCallReturnType( const CodeBase& codebase, const ITree<SourceToken>& tree, const IPosition<SourceToken>& methodcall, const VariableScopes& scopes, const String& invocationClass ) const
{
	const char* name = methodcall.getElement().getValue().getChars();

	String* return_type = new String();
	{
		String* arguments = this->resolveMethodCallArgumentTypes( codebase, tree, methodcall, scopes );
		{
			MethodSignature* method_signature = codebase.completeMethodSignature( invocationClass.getChars(), name, arguments->getChars() );
			delete return_type;
			return_type = new String( method_signature->getReturnType() );
		}
		delete arguments;
	}
	
	fprintf( stderr, "CU::resolveMethodCallReturnType( codebase, tree, (%s), scopes, %s ) | %s\n", name, invocationClass.getChars(), return_type->getChars() );
	
	return return_type;
}

String*
CompilationUnit::resolveMethodCallArgumentTypes( const CodeBase& codebase, const ITree<SourceToken>& tree, const IPosition<SourceToken>& methodcall, const VariableScopes& scopes ) const
{
	String* arguments = new String();

	const IPIterator<SourceToken>* it = tree.children( methodcall );
	while ( it->hasNext() )
	{
		const IPosition<SourceToken>* p = it->next();
		{
			switch ( p->getElement().getTokenType() )
			{
			case SourceToken::ARGUMENTS:
				delete arguments;
				arguments = this->recurseMethodArguments( codebase, tree, *p, scopes );
				break;
			default:
				break;
			}
		}
		delete p;
	}
	
	return arguments;
}

String*
CompilationUnit::recurseMethodArguments( const CodeBase& codebase, const ITree<SourceToken>& tree, const IPosition<SourceToken>& arguments, const VariableScopes& scopes ) const
{
	StringBuffer sb;

	const IPIterator<SourceToken>* it = tree.children( arguments );
	while ( it->hasNext() )
	{
		const IPosition<SourceToken>* p = it->next();
		{
			switch ( p->getElement().getTokenType() )
			{
			case SourceToken::ARGUMENT:
				{
					String* argument_type = this->recurseMethodArgument( codebase, tree, *p, scopes );
					{
						sb.append( *argument_type );
						sb.append( "," );
					}
					delete argument_type;
				}
				break;
			default:
				break;
			}
		}
		delete p;
	}
	
	sb.removeLast();
	const_cast<SourceToken&>( arguments.getElement() ).setValue( sb.asString() );
	
	return sb.asString();
}

String*
CompilationUnit::recurseMethodArgument( const CodeBase& codebase, const ITree<SourceToken>& tree, const IPosition<SourceToken>& argument, const VariableScopes& scopes ) const
{
	String* argument_type = new String();
	{
		bool is_array = false;
		String* invocation_class = new String( this->getFQName() );
		const IPIterator<SourceToken>* it = tree.children( argument );
		while ( it->hasNext() )
		{
			const IPosition<SourceToken>* p = it->next();
			{
				const char* value = p->getElement().getValue().getChars();
			
				switch ( p->getElement().getTokenType() )
				{
				case SourceToken::KEYWORD:
					delete invocation_class;
					invocation_class = this->resolveFQTypeOfName( value, scopes );
					break;
				case SourceToken::NAME:
					delete invocation_class;
					delete argument_type;
					invocation_class = this->resolveTypeOfName( value, scopes );
					argument_type    = new String( *invocation_class );
					break;
				case SourceToken::SYMBOL:
					if ( p->getElement().getValue().contentEquals( "[" ) )
					{
						is_array = true;
					}
					break;
				case SourceToken::FLOAT:
				case SourceToken::INTEGER:
					if ( ! is_array )
					{
						delete argument_type;
						argument_type = new String( p->getElement().getTokenTypeString() );
					}
					break;
				case SourceToken::METHODCALL:
					delete argument_type;
					argument_type = this->resolveMethodCallReturnType( codebase, tree, *p, scopes, *invocation_class );
				default:
					break;
				}
			}
			delete p;
		}
		delete it;
		delete invocation_class;
	}

	const_cast<SourceToken&>( argument.getElement() ).setValue( new String( *argument_type ) );
	
	return argument_type;
}

