
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
	astral::ast::AST* ast;
	openxds::base::String* location;
	openxds::base::String* packageName;
	openxds::base::String* className;
	openxds::base::String* fqName;
	openxds::base::String* extendsClass;
	openxds::adt::IList<openxds::base::String>* imports;
	openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >* methods;
	openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >* members;

	openxds::adt::IDictionary<openxds::base::String>* importedTypes;
	
	openxds::adt::IDictionary<Method>* methodObjects;
~

The abstract syntax tree is accessible via the /ast/ member.
The /AST/ class provides one useful method /getTree/, which is used to retrieve an /openxds::adt::ITree/ with nodes of type /astral::tokenizer::SourceToken/.
For more information on the structure and data included in the abstract syntax tree refer to the /Astral AST/ documentation, see:

~html~
<a href='http://astral.crossadaptive.com/documentation/astral/ast'>Astral AST</a>
~

The Java files package name, class name, and superclass, are stored in the members /packageName/, /className/, and /extendsClass/.

A list of the "imports" is stored in the /imports/ list, while two dictionaries - /methods/ and /members/ - map method and member names to their locations in the abstract syntax tree.

..		Public Constructor declarations

Usually, /CompilationUnit/ instances would be instantiated by the /CodeBase/ class, however they may directly instantiated by simply passing the location of Java source file to the /CompilationUnit/ constructor.

~include/astral/CompilationUnit.h~
public:
	         CompilationUnit( const char* location );
	virtual ~CompilationUnit();
	virtual void initialise();
~


..		Insertion of new methods

~include/astral/CompilationUnit.h~
	virtual bool insertNewMethod( const char* methodKey, const astral::ast::AST& aMethodAST );
~

..		Public const method declarations

The following methods provide public (const) access to the class's members.

~include/astral/CompilationUnit.h~
public:
	virtual       void                                   resetImportedTypes( const SymbolDB& symbolDB );

	virtual const   astral::ast::AST&                                getAST() const { return *this->ast;          }
	virtual       openxds::base::String&                       getNamespace() const { return *this->packageName;  }
	virtual       openxds::base::String&                            getName() const { return *this->className;    }
	virtual       openxds::base::String&                          getFQName() const { return *this->fqName;       }
	virtual       openxds::base::String&                      getSuperclass() const { return *this->extendsClass; }
	virtual       openxds::adt::IList<openxds::base::String>&    getImports() const { return *this->imports;      }

	virtual openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >&
		getMethods() const { return *this->methods; }
	virtual openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >&
		getMembers() const { return *this->members; }
~

..		Protected const method declarations

The following members are used by other classes in the Astral library.


~include/astral/CompilationUnit.h~
public:

	virtual void                       registerSymbols( openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& symbols, const openxds::adt::IEntry<CompilationUnit>& e ) const;
	virtual void                     deregisterSymbols( openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& symbols ) const;

	virtual openxds::base::String* resolveFQTypeOfName( const char* name, const VariableScopes& scopes ) const;
	virtual openxds::base::String* resolveFQTypeOfType( const char* type ) const;
	virtual openxds::base::String*   resolveTypeOfName( const char* name, const VariableScopes& scopes ) const;
	virtual openxds::base::String*   resolveMemberType( const char* name ) const;
	virtual openxds::base::String*   resolveMethodType( const char* name ) const;
	
	virtual openxds::base::String*   resolveMethodCallParametersToTypes( const openxds::base::String& parameters, const VariableScopes& scopes ) const;


	virtual openxds::base::String*      resolveMethodCallReturnType( const CodeBase& codebase, const openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, const openxds::adt::IPosition<astral::tokenizer::SourceToken>& methodcall, const VariableScopes& scopes, const openxds::base::String& invocationClass ) const;
	virtual openxds::base::String*   resolveMethodCallArgumentTypes( const CodeBase& codebase, const openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, const openxds::adt::IPosition<astral::tokenizer::SourceToken>& methodcall, const VariableScopes& scopes ) const;
	virtual openxds::base::String*           recurseMethodArguments( const CodeBase& codebase, const openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, const openxds::adt::IPosition<astral::tokenizer::SourceToken>& methodcall, const VariableScopes& scopes ) const;
	virtual openxds::base::String*            recurseMethodArgument( const CodeBase& codebase, const openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, const openxds::adt::IPosition<astral::tokenizer::SourceToken>& methodcall, const VariableScopes& scopes ) const;

	virtual openxds::base::String* retrieveMethodContent( const MethodSignature& aMethodSignature ) const;

//	virtual void printHTML( openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& symbols,
//	                        openxds::adt::IDictionary<openxds::base::String>& importedTypes,
//							openxds::io::PrintWriter& writer ) const;
//	
//	virtual openxds::base::String* toXML( openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& iSymbols, openxds::adt::IDictionary<openxds::base::String>& iTypes ) const;
//
//	
//	virtual void printMethods() const;
//	virtual void printMembers() const;

	virtual       Method& getMethod( const MethodSignature& aMethodSignature )       throw (openxds::exceptions::NoSuchElementException*);
	virtual const Method& getMethod( const MethodSignature& aMethodSignature ) const throw (openxds::exceptions::NoSuchElementException*);

	virtual const openxds::base::String& getLocation() const { return *this->location; }
	
	virtual void save();
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
#include <astral/Method.h>
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
#include <openxds.io/File.h>
#include <openxds.io/FileOutputStream.h>
#include <openxds.io/IO.h>
#include <openxds.io/Path.h>
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
static String*                  translate(       String* aString );
static long              determineMinTabs( const String& aString );
~
!

..		Constructors

...			CompilationUnit

~
CompilationUnit::CompilationUnit( const char* location )
~

Parameters
|
/location/, of a Java source file.
|

Returns
|
a new instance of /CompilationUnit/.
|

Implementation

~source/cplusplus/CompilationUnit.cpp~
CompilationUnit::CompilationUnit( const char* location )
{
	this->location     = new String( location );
	this->imports      = new Sequence<String>();
	this->methods      = new Dictionary<IPosition<SourceToken> >();
	this->members      = new Dictionary<IPosition<SourceToken> >();
	this->ast          = new AST();
	this->packageName  = NULL;
	this->className    = NULL;
	this->extendsClass = NULL;
	
	this->importedTypes = new Dictionary<String>();
	this->methodObjects = new Dictionary<Method>();
}
~

..		Destructor

~
CompilationUnit::~CompilationUnit()
~

Implementation

~source/cplusplus/CompilationUnit.cpp~
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
~

..		Methods

...			Initialise

~
CompilationUnit::initialise()
~

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
~

...			Method: insertNewMethod

~
void CompilationUnit::insertNewMethod( const char* methodKey, const AST& aMethodAST );
~

Parameters
|
/methodKey/, the methodKey that identifies this method;

/aMethodAST/, that represents the method to be added to the AST of the /CompilationUnit/.
|

Causes
|
/aMethodAST/ to be added to /ast/ as a new method.
|

~source/cplusplus/CompilationUnit.cpp~
bool
CompilationUnit::insertNewMethod( const char* methodKey, const AST& aMethodAST )
{
	bool status = false;

	fprintf( stderr, "CompilationUnit::insertNewMethod\n" );

	IPosition<SourceToken>* r = aMethodAST.getTree().root();
	{
		ITree<SourceToken>* method_ast_copy = aMethodAST.getTree().copyAsTree( *r );
		{
			IIterator<IPosition<SourceToken> >* it = this->getMethods().values();
			{
				IPosition<SourceToken>* last_method = NULL;

				while ( it->hasNext() )
				{
					IPosition<SourceToken>& p = it->next();
					SourceToken&            t = p.getElement();
					
					switch ( t.getTokenType() )
					{
					case SourceToken::METHOD:
						last_method = &p;
						break;
					default:
						break;
					}
				}

				if ( last_method )
				{
					fprintf( stderr, "\t Found last method\n" );

					ITree<SourceToken>&     t = this->ast->getTree();

					IPosition<SourceToken>* class_block = t.parent( *last_method );
					{
						long insertion_index = t.nrOfChild( *last_method ) + 1;

						fprintf( stderr, "\t Inserting at index: %li\n", insertion_index );

						delete t.insertChildAt( *class_block, new SourceToken( SourceToken::NEWLINE, new String( "\n" ) ), insertion_index++ );
						delete t.insertChildAt( *class_block, new SourceToken( SourceToken::TAB,     new String( "\t" ) ), insertion_index++ );
						
						IPosition<SourceToken>* method_ast_root = method_ast_copy->root();
						{
							IPosition<SourceToken>* n = t.insertChildAt( *class_block, new SourceToken( SourceToken::METHOD, new String( "" ) ), insertion_index );
							t.swapSubtrees( *n, *method_ast_copy, *method_ast_root );
							this->methods->insert( methodKey, n );
						}
						delete method_ast_root;
					}
					delete class_block;

					status = true;
				}
			}
			delete it;
		}
		//delete method_ast_copy;
	}
	delete r;

	return status;
}
~

...			Method: resetImportedTypes

~
void CompilationUnit::resetImportedTypes( const SymbolDB& symbolDB );
~

Parameters
|
/symbolDB/, member of /CodeBase/ that federates symbols from all /CompilationUnit/ objects.
|

Causes
|
/importedTypes/, dictionary to be updated.
|

~source/cplusplus/CompilationUnit.cpp~
void
CompilationUnit::resetImportedTypes( const SymbolDB& symbolDB )
{
	delete this->importedTypes;
	this->importedTypes = symbolDB.importedTypes( this->getImports() );
}
~


...			Method: register Symobls

~
void CompilationUnit::registerSymbols( IDictionary<const IEntry<CompilationUnit> >& symbols, const IEntry<CompilationUnit>& e ) const
~

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
~

...			Method: register Symobls

~
void CompilationUnit::deregisterSymbols( IDictionary<const IEntry<CompilationUnit> >& symbols ) const
~

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
	


...			Method: retrieveMethodContent

~
String* CompilationUnit::retrieveMethodContent( const MethodSignature& aMethodSignature ) const
~

Parameters
|
/aMethodSignature/, that identified a method.
|

Returns
|
A new string instance that contains the content of the method including Javadoc comment, declaration, and method body.
|

Implementation

~source/cplusplus/CompilationUnit.cpp~
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
~

...			Hidden (static) methods

The /translate/ and /determineMinTabs/ methods are used to strip undesired tabs from the front of each line of method definitions.


/*
 *	None class helper methods.
 */

~source/cplusplus/CompilationUnit.cpp~
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
~




~source/cplusplus/CompilationUnit.cpp~
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
	
	fprintf( stderr, "CU::resolveMethodCallReturnType( codebase, tree, (%s), scopes, %s ) | %s\n", name, invocationClass.getChars(), return_type->getChars() );
	
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
	fprintf( stderr, "CompilationUnit::recurseMethodArguments\n" );

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
						fprintf( stderr, "\t %s\n", argument_type->getChars() );

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
~

~source/cplusplus/CompilationUnit.cpp~
Method&
CompilationUnit::getMethod( const MethodSignature& aMethodSignature )
throw (NoSuchElementException*)
{
	Method* method = null;
	const char* method_key = aMethodSignature.getMethodKey().getChars();

	try
	{
		IEntry<Method>* e = this->methodObjects->find( method_key );
		{
			method = &e->getValue();
		}
		delete e;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	
		try
		{
			const IEntry<IPosition<SourceToken> >* e = this->methods->find( method_key );
			{
				AST* method_ast = this->ast->copySubtree( e->getValue() );
				method = new Method( *this, aMethodSignature, method_ast );
				this->methodObjects->insert( method_key, method );
			}
			delete e;
		}
		catch ( NoSuchElementException* ex )
		{
			method = new Method( *this, aMethodSignature );
			this->methodObjects->insert( method_key, method );
		}
	}
	
	return *method;
}
~

~source/cplusplus/CompilationUnit.cpp~
const Method&
CompilationUnit::getMethod( const MethodSignature& aMethodSignature ) const
throw (NoSuchElementException*)
{
	return const_cast<CompilationUnit*>( this )->getMethod( aMethodSignature );
}
~

~source/cplusplus/CompilationUnit.cpp~
void
CompilationUnit::save()
{
	fprintf( stdout, "CompilationUnit::save() to: %s\n", this->location->getChars() );
	
	Path             target_path( *this->location );
	File             target_file( target_path );
	FileOutputStream         fos( target_file );
	PrintWriter           writer( fos );
	
	if ( target_file.exists() )
	{
		fprintf( stdout, "\t target file exists\n" );
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
~






