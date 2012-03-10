#include <astral/AdvancedHTMLPrintTour.h>
#include <astral/Codebase.h>
#include <astral/CompilationUnit.h>
#include <astral/Declaration.h>
#include <astral/Import.h>
#include <astral/ImportsList.h>
#include <astral/Member.h>
#include <astral/MembersList.h>
#include <astral/Method.h>
#include <astral/MethodsList.h>
#include <astral/MethodSignature.h>
#include <astral/PlatformTypes.h>
#include <astral/SymbolDB.h>
#include <astral/VariableScopes.h>
#include <astral.ast/AST.h>
#include <astral.ast/ASTHelper.h>
#include <astral.tours/FindLastTokenTour.h>
#include <astral.tours/HTMLPrintTour.h>
#include <astral.tours/MemberDiscoveryTour.h>
#include <astral.tours/MethodDiscoveryTour.h>
#include <astral.tours/PackageDiscoveryTour.h>
#include <astral.tours/PrintSourceTour.h>
#include <astral.tokenizer/SourceToken.h>
#include <openxds.io/File.h>
#include <openxds.io/FileOutputStream.h>
#include <openxds.io/IO.h>
#include <openxds.io/Path.h>
#include <openxds.io/PrintWriter.h>
#include <openxds.io/OutputStream.h>
#include <openxds.io/IOBuffer.h>
#include <openxds.adt/IEIterator.h>
#include <openxds.adt/IIterator.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.base/StringTokenizer.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/Math.h>

#include <stdio.h>

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

CompilationUnit::CompilationUnit( const char* location )
{
	this->location        = new String( location );
	this->members         = new Dictionary<IPosition<SourceToken> >();
	this->ast             = new AST();
	this->packageName     = NULL;
	this->className       = NULL;
	this->extendsClass    = NULL;
	
	this->declaration     = new Declaration( *this );
	this->importsList     = new ImportsList( *this );
	this->membersList     = new MembersList( *this );
	this->methodsList     = new MethodsList( *this );
	
	this->importedTypes   = new Dictionary<String>();
	this->methodObjects   = new Dictionary<Method>();
}

CompilationUnit::~CompilationUnit()
{
	delete this->importsList;
	delete this->membersList;
	delete this->methodsList;

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
	this->ast->parseFile( this->location->getChars() );
	
	IPosition<SourceToken>* root = this->ast->getTree().root();
	{
		IList<String>*                        imports          = new Sequence<String>();
		IList<IPosition<SourceToken> >*       import_positions = new Sequence<IPosition<SourceToken> >();
		{
			PackageDiscoveryTour pdt( this->ast->getTree(), *imports, *import_positions );
			pdt.doGeneralTour( *root );

			this->packageName  = pdt.getPackageName().asString();
			this->className    = pdt.getClassName().asString();
			this->fqName       = new FormattedString( "%s.%s", this->packageName->getChars(), this->className->getChars() );
			this->extendsClass = pdt.getExtendsClass().asString();

			FindLastTokenTour fltt( this->ast->getTree(), SourceToken::CLASS );
			fltt.doGeneralTour( *root );

			MethodDiscoveryTour mdt1( this->ast->getTree(), this->methodsList->getMethodPositions() );
			mdt1.doGeneralTour( *root );
			
			MemberDiscoveryTour mdt2( this->ast->getTree(), *this->members );
			mdt2.doGeneralTour( *root );

			this->declaration->initialise( fltt.copyLastTokenPosition() );
			this->importsList->initialise( *import_positions );
			this->membersList->initialise( *this->members );

			//this->initialiseImportObjects();
			//this->imports->insertLast( new String( this->getNamespace() ) );
		}
		delete imports;
		delete import_positions;
	}
	delete root;
}

void
CompilationUnit::resetImportedTypes( const SymbolDB& symbolDB )
{
	delete this->importedTypes;
	this->importedTypes = symbolDB.importedTypes( this->getImportsList(), this->getNamespace() );
}

void
CompilationUnit::registerSymbols( IDictionary<const IEntry<CompilationUnit> >& symbols, const IEntry<CompilationUnit>& e ) const
{
	IEIterator<IPosition<SourceToken> >* ie = this->methodsList->getMethodPositions().entries();
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


	IIterator<Member>* it = this->membersList->values();
	while ( it->hasNext() )
	{
		Member& member = it->next();
		
		FormattedString member_key( "%s.%s.%s", this->getNamespace().getChars(), this->getName().getChars(), member.getName().getChars() );
		
		delete symbols.insert( member_key.getChars(), e.copy() );
	}
	delete it;
}

void
CompilationUnit::deregisterSymbols( IDictionary<const IEntry<CompilationUnit> >& symbols ) const
{
	IEIterator<const IEntry<CompilationUnit> >* ie = symbols.entries();
	while ( ie->hasNext() )
	{
		IEntry<const IEntry<CompilationUnit> >* e  = ie->next();
		const IEntry<CompilationUnit>&          e2 =  e->getValue();
		const CompilationUnit&                  cu =  e2.getValue();

		if ( this == &cu )
		{
			delete symbols.remove( e );
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
//		if ( ! fq_type->contentEquals( "" ) )
//		{
//			delete type;
//			type = new String( *fq_type );
//		}
		delete type;
		type = fq_type;
	}
//	else
//	{
//		delete type;
//		       type = new String( name );
//	}
	
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
CompilationUnit::resolveFQTypeOfPlatformType( const char* type ) const
{
	PTypes platformTypes;

	if ( platformTypes.hasType( type ) )
	{
		return new String( platformTypes.resolve( type ) );
	}
	else
	{
		return new String();
	}
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
		StringTokenizer st( _name );
		st.setDelimiter( '[' );
		if ( st.hasMoreTokens() )
		{
			String* variable_name = st.nextToken();
			{
				type = scopes.searchForTypeOfName( variable_name->getChars() );
				if ( type->contentEquals( "" ) )
				{
					delete type;
					type = this->resolveMemberType( variable_name->getChars() );
				}

				if ( ! variable_name->contentEquals( name ) )
				{
					StringBuffer sb;
					sb.append( *type );
					if ( ']' == sb.charAt( sb.getLength() - 1 ) ) sb.removeLast();
					if ( '[' == sb.charAt( sb.getLength() - 1 ) ) sb.removeLast();

					delete type;
					type = sb.asString();
				}
			}
			delete variable_name;
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
		IEntry<IPosition<SourceToken> >* e = this->methodsList->getMethodPositions().startsWith( name );
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
	
	#ifdef DEBUG_ASTRAL_COMPILATIONUNIT
	fprintf( stderr, "CU::resolveMethodCallReturnType( codebase, tree, (%s), scopes, %s ) | %s\n", name, invocationClass.getChars(), return_type->getChars() );
	#endif
	
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
	#ifdef DEBUG_ASTRAL_COMPILATIONUNIT
	fprintf( stderr, "CompilationUnit::recurseMethodArguments\n" );
	#endif

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
						#ifdef DEBUG_ASTRAL_COMPILATIONUNIT
						fprintf( stderr, "\t %s\n", argument_type->getChars() );
						#endif

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
					invocation_class = generaliseType( this->resolveTypeOfName( value, scopes ) );
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

String*
CompilationUnit::generaliseType( String* aType ) const
{
	if ( aType->contentEquals( "long"  ) ||
	     aType->contentEquals( "short" ) ||
		 aType->contentEquals( "int"   ) )
	{
		delete aType;
		aType = new String( "INTEGER" );
	}
	return aType;
}

//Method&
//CompilationUnit::getMethod( const MethodSignature& aMethodSignature )
//throw (NoSuchElementException*)
//{
//	return this->methodsList->getMethod( aMethodSignature );
//}

//const Method&
//CompilationUnit::getMethod( const MethodSignature& aMethodSignature ) const
//throw (NoSuchElementException*)
//{
//	return const_cast<CompilationUnit*>( this )->getMethod( aMethodSignature );
//}

void
CompilationUnit::save()
{
	//fprintf( stdout, "CompilationUnit::save() to: %s\n", this->location->getChars() );
	
	Path             target_path( *this->location );
	File             target_file( target_path );
	FileOutputStream         fos( target_file );
	PrintWriter           writer( fos );
	
	if ( target_file.exists() )
	{
		//fprintf( stdout, "\t target file exists\n" );
		{
			PrintSourceTour pst( this->getAST().getTree(), IO::out() );
			IPosition<SourceToken>* root = this->getAST().getTree().root();
			{
				pst.doGeneralTour( *root );
			}
			delete root;
		}
		{
			PrintSourceTour pst( this->getAST().getTree(), writer );
			IPosition<SourceToken>* root = this->getAST().getTree().root();
			{
				pst.doGeneralTour( *root );
			}
			delete root;
		}
	}
}

long
CompilationUnit::calculateOffset( const IPosition<SourceToken>& p ) const
{
	long offset = p.getElement().getOffset();
	if ( this->ast->getTree().hasParent( p ) )
	{
		const IPosition<SourceToken>* parent = this->ast->getTree().parent( p );
		offset += this->calculateOffset( *parent );
		delete parent;
	}
	return offset;
}

