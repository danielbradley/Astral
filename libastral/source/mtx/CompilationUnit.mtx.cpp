
.		Compilation Unit

!
~!include/astral/CompilationUnit.h~
#ifndef ASTRAL_COMPILATIONUNIT_H
#define ASTRAL_COMPILATIONUNIT_H

#include <astral.h>
#include <astral.ast/AST.h>
#include <astral.tokenizer.h>
#include <openxds.io.h>
#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>
#include <openxds.exceptions/NoSuchElementException.h>

using astral::ast::AST;
using astral::tokenizer::SourceToken;
using openxds::adt::IDictionary;
using openxds::adt::IEntry;
using openxds::adt::IPosition;
using openxds::adt::ITree;
using openxds::base::String;
~

~include/astral/CompilationUnit.h~
namespace astral {
~
!

The /CompilationUnit/ class inherits directly from the /openxds::Object/ class.
	
~include/astral/CompilationUnit.h~
class CompilationUnit : openxds::Object {
~



..		Private members

It has the following private members - some of which are accessible via accessor methods.

~include/astral/CompilationUnit.h~
private:
	AST*                                            ast; 
	String*                                    location;
	String*                                 packageName;
	String*                                   className;
	String*                                      fqName;
	String*                                extendsClass;

	Declaration*                            declaration;
	ImportsList*                            importsList;
	  EnumsList*                              enumsList;
	MembersList*                            membersList;
	MethodsList*                            methodsList;
	
	IDictionary<IPosition<SourceToken> >*       members; // Scheduled for termination.
	IDictionary<String>*                  importedTypes;
	IDictionary<Method>*                  methodObjects;
~



..		Public Constructor declarations

Usually, /CompilationUnit/ instances would be instantiated by the /CodeBase/ class, however they may directly instantiated by simply passing the location of Java source file to the /CompilationUnit/ constructor.

~include/astral/CompilationUnit.h~
public:
	                CompilationUnit( const char* location );
	virtual        ~CompilationUnit();
~



..		Public Methods

~include/astral/CompilationUnit.h~
public:
	virtual void         initialise();
	virtual void resetImportedTypes( const SymbolDB& symbolDB );
	virtual void               save();
~



..		Public accessors

~include/astral/CompilationUnit.h~
public:
	virtual               AST&               getAST()       { return *this->ast;          }
	virtual const         AST&               getAST() const { return *this->ast;          }
	virtual       Declaration&       getDeclaration()       { return *this->declaration;  }
	virtual const Declaration&       getDeclaration() const { return *this->declaration;  }
	virtual         EnumsList&         getEnumsList()       { return *this->enumsList;    }
	virtual const   EnumsList&         getEnumsList() const { return *this->enumsList;    }
	virtual       ImportsList&       getImportsList()       { return *this->importsList;  }
	virtual const ImportsList&       getImportsList() const { return *this->importsList;  }
	virtual       MembersList&       getMembersList()       { return *this->membersList;  }
	virtual const MembersList&       getMembersList() const { return *this->membersList;  }
	virtual       MethodsList&       getMethodsList()       { return *this->methodsList;  }
	virtual const MethodsList&       getMethodsList() const { return *this->methodsList;  }
	virtual const      String&         getNamespace() const { return *this->packageName;  }
	virtual const      String&              getName() const { return *this->className;    }
	virtual const      String&            getFQName() const { return *this->fqName;       }
	virtual const      String&        getSuperclass() const { return *this->extendsClass; }
	virtual const      String&          getLocation() const { return *this->location;     }

	virtual       IDictionary<IPosition<SourceToken> >&    getMembers() const { return *this->members; }
~



..		Protected const method declarations

~include/astral/CompilationUnit.h~
public:

	virtual void                       registerSymbols( IDictionary<const IEntry<CompilationUnit> >& symbols, const IEntry<CompilationUnit>& e ) const;
	virtual void                     deregisterSymbols( IDictionary<const IEntry<CompilationUnit> >& symbols ) const;

	virtual       String*         resolveFQTypeOfName( const char* name, const VariableScopes& scopes ) const;
	virtual       String*         resolveFQTypeOfType( const char* type ) const;
	virtual       String* resolveFQTypeOfPlatformType( const char* type ) const;
	virtual       String*           resolveTypeOfName( const char* name, const VariableScopes& scopes ) const;
	virtual       String*           resolveMemberType( const char* name ) const;
	virtual       String*           resolveMethodType( const char* methodCall ) const;
	
	virtual       MethodSignature* matchingMethodSignature( const char* methodName, const char* parameters ) const;
	
	virtual       String* resolveMethodCallParametersToTypes( const String& parameters, const VariableScopes& scopes ) const;
	virtual       String*        resolveMethodCallReturnType( const CodeBase& codebase, const ITree<SourceToken>& tree, const IPosition<SourceToken>& methodcall, const VariableScopes& scopes, const String& invocationClass ) const;
	virtual       String*     resolveMethodCallArgumentTypes( const CodeBase& codebase, const ITree<SourceToken>& tree, const IPosition<SourceToken>& methodcall, const VariableScopes& scopes ) const;
	virtual       String*             recurseMethodArguments( const CodeBase& codebase, const ITree<SourceToken>& tree, const IPosition<SourceToken>& methodcall, const VariableScopes& scopes ) const;
	virtual       String*              recurseMethodArgument( const CodeBase& codebase, const ITree<SourceToken>& tree, const IPosition<SourceToken>& methodcall, const VariableScopes& scopes ) const;

	virtual       String*                     generaliseType(       String* aType ) const;


	virtual long calculateOffset( const IPosition<SourceToken>& p ) const;
~

!
~include/astral/CompilationUnit.h~
};
~
!

!
~include/astral/CompilationUnit.h~
};

#endif
~
!

!
~!source/cplusplus/CompilationUnit.cpp~
#include <astral/AdvancedHTMLPrintTour.h>
#include <astral/Codebase.h>
#include <astral/CompilationUnit.h>
#include <astral/Declaration.h>
#include <astral/Enum.h>
#include <astral/EnumsList.h>
#include <astral/Import.h>
#include <astral/ImportsList.h>
#include <astral/Member.h>
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
~
!

!
~source/cplusplus/CompilationUnit.cpp~
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
~
!

!
~source/cplusplus/CompilationUnit.cpp~
static String* searchForNameInLocalScopes( const char* name, const ISequence<IDictionary<String > >& scopes );
~
!



..		Constructors

~source/cplusplus/CompilationUnit.cpp~
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
	this->enumsList       = new   EnumsList( *this );
	this->membersList     = new MembersList( *this );
	this->methodsList     = new MethodsList( *this );
	
	this->importedTypes   = new Dictionary<String>();
	this->methodObjects   = new Dictionary<Method>();
}
~



..		Destructor

~source/cplusplus/CompilationUnit.cpp~
CompilationUnit::~CompilationUnit()
{
	delete this->location;
	delete this->members;
	delete this->ast;
	delete this->packageName;
	delete this->className;
	delete this->extendsClass;

	delete this->declaration;
	delete this->importsList;
	delete this->enumsList;
	delete this->membersList;
	delete this->methodsList;

	delete this->importedTypes;
	delete this->methodObjects;
}
~



..		Methods


...			Initialise

The initialise method first causes the /astral::AST/ class to parse its source file;
then performs a number of tree traversals.
The first traversal uses a /PackageDiscoveryTour/ to determine the package, class, and super class names;
and also populates the /imports/ list.
The second traversal locates method definitions.
Finally, the third tour locates member declarations.

~source/cplusplus/CompilationUnit.cpp~
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
~


...			Method: resetImportedTypes

~source/cplusplus/CompilationUnit.cpp~
void
CompilationUnit::resetImportedTypes( const SymbolDB& symbolDB )
{
	delete this->importedTypes;
	this->importedTypes = symbolDB.importedTypes( this->getImportsList(), this->getNamespace() );
}
~


...			Method: registerSymobls

Parameters
|
/symbols/, a dictionary that maps fully-qualified symbols to an entry that stores a reference to the /CompilationUnit/ they are defined within;

/e/, an entry that stores a reference to a /CompilationUnit/.
|

Note
|
The /registerSymbols/ method is called from the /SymbolDB/ class to register the symbols defined within the /CompilationUnit/.
The parameter /e/ is an entry of the /CodeBase/ /files/ dictionary.
Therefore, given a fully-qualified symbol string identifier it is possible to locate the /CompilationUnit/ that a symbol is defined within. 
|

Explanation
|
For each method and member an entry is added to the symbols dictionary.
The /StringBuffer/ is used to create a fully-qualified string idenfier;
while the /StringTokenizer/ is used to prevent the idenfier from including the member type or method return type.
|

Implementation

~source/cplusplus/CompilationUnit.cpp~
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
~

...			Method: deregisterSymobls

Parameters
|
/symbols/, a dictionary that maps fully-qualified symbols to an entry that stores a reference to the /CompilationUnit/ they are defined within.
|


Implementation

~source/cplusplus/CompilationUnit.cpp~
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
~

...			Method: resolveFQTypeOfName

~
String* resolveFQTypeOfName( const char* name, const VariableScopes& scopes ) const;
~

Parameters
|
/name/, that needs to be resolved to a fully qualified type.
|

Returns
|
The fully qualified (including package) type of /name/.
|

Implementation

~source/cplusplus/CompilationUnit.cpp~
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
~

...			Method: resolveFQTypeOfType

~
String* resolveFQTypeOfType( const char* type ) const;
~

Parameters
|
/type/, that needs to be resolved to a fully qualified type.
|

Returns
|
The fully qualified (including package) type of /type/.
|

Note
|
Compilation units do not know what types they have imported, only the commands used to import types, e.g. "java.util.*".
Therefore, user-code should generate the /importedTypes/ dictionary by calling:
~
IDictionary<String>* imported_types = codebase.getSymbolDB().importedTypes( compilation_unit.getImports() );
~

This returns a dictionary that resolves to fully-qualified types all of the types imported.
This only needs to be done once for each /CompilationUnit/, after the /CodeBase/ has been populated.

In the future this may be automatically carried out, but for now this does not happend to maintain the conceptual clarity of the CompilationUnit class.
|

Implementation

~source/cplusplus/CompilationUnit.cpp~
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
~

~source/cplusplus/CompilationUnit.cpp~
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
~

...			Method: resolveTypeOfName

~
String* CompilationUnit::resolveTypeOfName( const char* name, const VariableScopes& scopes ) const;
~

Parameters
|
/name/, of a member, or variable of this /CompilationUnit/.

/scopes/, a sequence of dictionaries which represent variables declared in block scopes.
|

Returns
|
The unqualified type of the /name/, e.g. "int", "String", if the /name/ exists; otherwise an empty string.
|

Implementation

~source/cplusplus/CompilationUnit.cpp~
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
~

...			Method: resolveMemberType

~
String* CompilationUnit::resolveMemberType( const char* name ) const
~

Parameters
|
/name/, of a member of this /CompilationUnit/.
|

Returns
|
The unqualified type of the member, e.g. "int", "String", if the member exists; otherwise an empty string.
|

Implementation

~source/cplusplus/CompilationUnit.cpp~
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
~



...			Method: resolveMethodType

~
String* CompilationUnit::resolveMethodType( const char* name ) const
~

Parameters
|
/name/, of a method of this /CompilationUnit/.
|

Returns
|
The unqualified return type of the method, e.g. "int", "String", if the method exists; otherwise an empty string.
|

Implementation

~source/cplusplus/CompilationUnit.cpp~
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
~


~source/cplusplus/CompilationUnit.cpp~
MethodSignature*
CompilationUnit::matchingMethodSignature( const char* methodName, const char* parameters ) const
{
	MethodSignature* signature = NULL;

	MethodCall methodCall( methodName, parameters );
	const IIterator<String>* it = methodCall.generateVariations().elements();
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
	return signature;
}
~



...			Method: retrieveMethodContent

~
String* CompilationUnit::resolveMethodCallParametersToTypes( const String& parameters, const VariableScopes& scopes ) const;
~

Parameters
|
/parameters/, passed into a method;

/scopes/, containg declared parameters and variables of the method.
|

Returns
|
A new string instance that contains a comma separated list of TYPES corresponding to the passed parameters.
|

Implementation

~source/cplusplus/CompilationUnit.cpp~
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
~
	


~source/cplusplus/CompilationUnit.cpp~
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
~

~source/cplusplus/CompilationUnit.cpp~
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
~

~source/cplusplus/CompilationUnit.cpp~
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
~

~source/cplusplus/CompilationUnit.cpp~
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
					//invocation_class = generaliseType( this->resolveTypeOfName( value, scopes ) );
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
~

~source/cplusplus/CompilationUnit.cpp~
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
~

~source/cplusplus/CompilationUnit.cpp~
//Method&
//CompilationUnit::getMethod( const MethodSignature& aMethodSignature )
//throw (NoSuchElementException*)
//{
//	return this->methodsList->getMethod( aMethodSignature );
//}
~

~source/cplusplus/CompilationUnit.cpp~
//const Method&
//CompilationUnit::getMethod( const MethodSignature& aMethodSignature ) const
//throw (NoSuchElementException*)
//{
//	return const_cast<CompilationUnit*>( this )->getMethod( aMethodSignature );
//}
~

~source/cplusplus/CompilationUnit.cpp~
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
~


~source/cplusplus/CompilationUnit.cpp~
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
~
