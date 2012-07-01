#include <astral/AdvancedHTMLPrintTour.h>
#include <astral/Codebase.h>
#include <astral/CompilationUnit.h>
#include <astral/Declaration.h>
#include <astral/Enum.h>
#include <astral/EnumsList.h>
#include <astral/Import.h>
#include <astral/ImportsList.h>
#include <astral/Member.h>
#include <astral/MemberSignature.h>
#include <astral/MembersList.h>
#include <astral/Method.h>
#include <astral/MethodCall.h>
#include <astral/MethodsList.h>
#include <astral/MethodSignature.h>
#include <astral/PlatformTypes.h>
#include <astral/SymbolDB.h>
#include <astral/VariableScopes.h>
#include <astral.ast/AST.h>
#include <astral.ast/ASTHelper.h>
#include <astral.tours/EnumDiscoveryTour.h>
#include <astral.tours/FindLastTokenTour.h>
#include <astral.tours/HTMLPrintTour.h>
#include <astral.tours/MemberDiscoveryTour.h>
#include <astral.tours/MethodDiscoveryTour.h>
#include <astral.tours/PackageDiscoveryTour.h>
#include <astral.tours/PrintSourceTour.h>
#include <astral.tokenizer/JavaTokenizer.h>
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
#include <openxds.adt/IMap.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt.std/Map.h>
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

CompilationUnit::CompilationUnit( const CodeBase& codebase, const char* location ) : codebase( codebase )
{
	this->location        = new String( location );
	this->members         = new Dictionary<IPosition<SourceToken> >();
	this->ast             = new AST();
	this->packageName     = NULL;
	this->className       = NULL;
	this->genericName     = NULL;
	this->extendsClass    = NULL;
	
	this->declaration     = new Declaration( *this );
	this->importsList     = new ImportsList( *this );
	this->enumsList       = new   EnumsList( *this );
	this->membersList     = new MembersList( *this );
	this->methodsList     = new MethodsList( *this );
	
	this->importedTypes   = new Dictionary<String>();
	this->methodObjects   = new Dictionary<Method>();
}

CompilationUnit::~CompilationUnit()
{
	delete this->location;
	delete this->members;
	delete this->ast;
	delete this->packageName;
	delete this->className;
	delete this->genericName;
	delete this->extendsClass;

	delete this->declaration;
	delete this->importsList;
	delete this->enumsList;
	delete this->membersList;
	delete this->methodsList;

	delete this->importedTypes;
	delete this->methodObjects;
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
			this->genericName  = pdt.getGenericName().asString();
			this->fqName       = new FormattedString( "%s.%s", this->packageName->getChars(), this->className->getChars() );
			this->extendsClass = pdt.getExtendsClass().asString();

			EnumDiscoveryTour edt( this->ast->getTree(), this->enumsList->getEnumPositions() );
			edt.doGeneralTour( *root );

			FindLastTokenTour fltt( this->ast->getTree(), SourceToken::CLASS );
			fltt.doGeneralTour( *root );

			MethodDiscoveryTour mdt1( this->ast->getTree(), this->methodsList->getMethodPositions() );
			mdt1.doGeneralTour( *root );
			
			MemberDiscoveryTour mdt2( this->ast->getTree(), *this->members );
			mdt2.doGeneralTour( *root );

			this->declaration->initialise( fltt.copyLastTokenPosition() );
			this->importsList->initialise( *import_positions );
			this->enumsList->initialise();
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
	const char* nspace = this->getNamespace().getChars();
	const char* cls    = this->getName().getChars();

	IEIterator<IPosition<SourceToken> >* ie = this->methodsList->getMethodPositions().entries();
	{
		while ( ie->hasNext() )
		{
			IEntry<IPosition<SourceToken> >* entry = ie->next();
			{
				StringTokenizer st( entry->getKey() );
				st.setDelimiter( '|' );
				String* name = st.nextToken();
				{
					FormattedString key( "%s.%s.%s", nspace, cls, name->getChars() );

					//fprintf( stdout, "CU::registerSymbols: symbols.insert( \"%s\", e )\n", sb.getChars() );
					delete symbols.insert( key.getChars(), e.copy() );
				}
				delete name;
			}
			delete entry;
		}
	}
	delete ie;

	{
		IIterator<Member>* it = this->membersList->values();
		while ( it->hasNext() )
		{
			Member& member = it->next();
			
			FormattedString member_key( "%s.%s.%s", nspace, cls, member.getName().getChars() );
			delete symbols.insert( member_key.getChars(), e.copy() );
		}
		delete it;
	}
	{
		IIterator<Enum>* it = this->enumsList->getEnums().values();
		while ( it->hasNext() )
		{
			Enum& anEnum = it->next();
			
			const String& cls = anEnum.getClassName();
			if ( cls.getLength() )
			{
				FormattedString enum_key( "%s.%s.%s", nspace, cls.getChars(), anEnum.getName().getChars() );
				delete symbols.insert( enum_key.getChars(), e.copy() );
			}
			else
			{
				FormattedString enum_key( "%s.%s", nspace, anEnum.getName().getChars() );
				delete symbols.insert( enum_key.getChars(), e.copy() );
			}
		}
		delete it;
	}
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
	String* fq_type = new String();
	{
		String* type = this->resolveTypeOfName( name, scopes );
		if ( type->getLength() )
		{
			ClassSignature class_signature( *type );
			const char* _type = class_signature.getClassName().getChars();

			String* tmp = this->resolveFQTypeOfType( _type );
			if ( tmp->getLength() )
			{
				ClassSignature fq_sig( *tmp );
				const char* _nspace = fq_sig.getNamespace().getChars();
				const char* _gClass = class_signature.getGenericClass().getChars();
				
				delete fq_type; fq_type = new FormattedString( "%s.%s", _nspace, _gClass );
			}
			delete type; type = this->resolveFQTypeOfType( _type );
		}
		delete type;
	}
	return fq_type;
}

String*
CompilationUnit::resolveFQTypeOfType( const char* type ) const
{
	JavaTokenizer java_tokenizer;

	String* fq_type = new String( type );

	if ( ! java_tokenizer.isPrimitiveType( type ) )
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

		delete fq_type; fq_type = sb.asString();
	}
	return fq_type;
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
				//	e->getKey returns name|type (or name|type<?>)

				MemberSignature member_signature( e->getKey() );
				delete ret; ret = member_signature.getType().asString();
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
CompilationUnit::resolveMethodType( const char* methodCall ) const
{
	String* ret = new String( "" );
	try
	{
		IEntry<IPosition<SourceToken> >* e = this->methodsList->getMethodPositions().startsWith( methodCall );
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

static IMap<String>* generateParameterisationMap( const ClassSignature& specified, const ClassSignature& generic )
{
	const ISequence<String>& pSpecified = specified.getParameters();
	const ISequence<String>& pGeneric   = generic.getParameters();
	
	long len = Math::min( pSpecified.size(), pGeneric.size() );

	IMap<String>* p = new Map<String>();
	{
		for ( long i=0; i < len; i++ )
		{
			//	Example: p->put( "Character", new String( "E" ) );

			const String& spec = pSpecified.get( i );
			const String& gen  = pGeneric.get( i );
		
			p->put( spec.getChars(), new String( gen.getChars() ) );
		}
	}
	return p;
}

MethodSignature*
CompilationUnit::matchingMethodSignatureX( const ClassSignature& fqClassSig, const char* methodName, const char* parameters ) const
{
	MethodSignature* signature = NULL;
	
	IMap<String>* parameterisation = generateParameterisationMap( fqClassSig, ClassSignature( *this->genericName ) );
	{
		MethodCall methodCall( *this, methodName, parameters );
		methodCall.applyParameterisation( *parameterisation );
		
		const ISequence<String>& variations = methodCall.generateVariations();
		const IIterator<String>* it = variations.elements();
		bool loop = true;
		while ( loop && it->hasNext() )
		{
			const String& method_call = it->next();
			String* returnType = resolveMethodType( method_call.getChars() );
			if( returnType->getLength() )
			{
				ClassSignature cls( this->getFQName().getChars() );
				signature = new MethodSignature( cls, FormattedString( "%s|%s", method_call.getChars(), returnType->getChars() ) );
				loop = false;
			}
			delete returnType;
		}
		delete it;
	}
	delete parameterisation;
	
	return signature;
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
			if ( method_signature && method_signature->isValid() )
			{
				delete return_type;
				return_type = new String( method_signature->getReturnType() );
			}
			delete method_signature;
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
			SourceToken::TokenType ttype = p->getElement().getTokenType();
		
			switch ( ttype )
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
					delete argument_type;	
						
					argument_type    = this->resolveTypeOfName( value, scopes );
					invocation_class = this->resolveFQTypeOfName( value, scopes );
					break;

				case SourceToken::NAME:
					delete invocation_class;
					delete argument_type;

					argument_type    = this->resolveTypeOfName( value, scopes );
					invocation_class = this->resolveFQTypeOfType( argument_type->getChars() );
					break;

				case SourceToken::SYMBOL:
					if ( p->getElement().getValue().contentEquals( "[" ) )
					{
						is_array = true;
					}
					break;

				case SourceToken::FLOAT:
					if ( ! is_array )
					{
						delete argument_type;
						argument_type = new String( "float" );
					}
					break;

				case SourceToken::INTEGER:
					if ( ! is_array )
					{
						delete argument_type;
						argument_type = new String( "int" );
					}
					break;
					
				case SourceToken::DOUBLEQUOTE:
					delete argument_type;
					argument_type = new String( "String" );
					break;
					
				case SourceToken::QUOTE:
					delete argument_type;
					argument_type = new String( "char" );
					break;

//
//	Removed because argument resolution is now based on actual types i.e. int etc.
//				
//				case SourceToken::INTEGER:
//					if ( ! is_array )
//					{
//						delete argument_type;
//						argument_type = new String( p->getElement().getTokenTypeString() );
//					}
//					break;

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
		#ifdef DEBUG_COMPILATIONUNIT_SAVE
		{
			PrintSourceTour pst( this->getAST().getTree(), IO::out() );
			IPosition<SourceToken>* root = this->getAST().getTree().root();
			{
				pst.doGeneralTour( *root );
			}
			delete root;
		}
		#endif
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

