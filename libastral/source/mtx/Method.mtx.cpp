.	Method

The /Method/ class is used to extract a copy of a method from the /CodeBase/,
which can then be modified and resynced back with the /CodeBase/.




..		Lifecycle

A reference to a /Method/ object is retrieved from a /CompilationUnit/ object,
which was itself retrieved from a /CodeBase/ object.
The /CodeBase/ instantiates and stores one /Method/ object for each method.
Therefore requests to /getMethod/ will return a reference to the same object.
Race-conditions should therefore be avoided.

Typically, user-code will want to retrieve the method content as text for editing,
then will want to resync the modified method content back into the /codebase/.

Retrieving the text will usually be performed by performing a custom tree traversal as
this will allow the user code to markup tokens based on their type.
The code below shows how the ? tree traversal is used to retrieve plain text as a String.

~
String* retrieveMethodContent( CompilationUnit& cu, MethodSignature& sig )
{
	Method& method = cu.getMethod( sig );

	return perform_traversal( method.getMethodAST() );
}
~

Periodically, the code might want to be reparsed.
The code below uses /reparseMethod/ to parse the content into a new abstract syntax tree.
If there are any problems parsing the code a /ParsingException/ is thrown.

Typically, after reparsing the content, the AST will be retraversed (as above) if reannotated
text is required.

~
bool reparseContent( CompilationUnit& cu, MethodSignature& sig, const String& content )
{
	bool status = false;

	Method& method = cu.getMethod( sig );
	try
	{
		method.reparseMethod( content );
		status = true;
	}
	catch ( Exception* ex )
	{
		delete ex;
	}
	return status;
}
~

Eventaully, the edited content should be synced back to the /CodeBase/.

~
bool resync( CompilationUnit& cu, MethodSignature& sig, const String& content )
{
	bool status = false;

	Method& method = cu.getMethod( sig );
	try
	{
		method.reparseMethod( content );
		method.sync();
		status = true;
	}
	catch ( ParsingException* ex )
	{
		delete ex;
	}
	return status;
}
~

In some cases, it is desired to revert the method content back to that in the /CodeBase/.

~
String* revertAndRetrieveMethodContent( CompilationUnit& cu, MethodSignature& sig )
{
	Method& method = cu.getMethod( sig );
	method.revert();

	return perform_traversal( method.getMethodAST() );
}
~


..		Header

~!include/astral/Method.h~
#ifndef ASTRAL_METHOD_H
#define ASTRAL_METHOD_H
~




...			Dependencies

~include/astral/Method.h~
#include "astral.h"
#include <astral.ast.h>
#include <openxds/Object.h>
#include <openxds.base.h>
~




...			Namespace

~include/astral/Method.h~
namespace astral {
~




...			Class definition

~include/astral/Method.h~
class Method : openxds::Object {
~




....			Friends

The /Method/ class is only instantiated and destroyed from within the /CompilationUnit/ class.

~include/astral/Method.h~
friend class CompilationUnit;
~




....			Members

~include/astral/Method.h~
private: CompilationUnit&         cu;
private: astral::ast::AST* methodAST;
private: MethodSignature*  signature;
private: bool               modified;
~




....			Protected Constructors

~include/astral/Method.h~
protected:
	 Method( astral::ast::AST* methodAST, CompilationUnit& cu, const MethodSignature& aMethodSignature );
~




....			Public destructors

~include/astral/Method.h~
public:
	~Method();
~




....			Public const methods

~include/astral/Method.h~
public:
	virtual       void reparseMethod( const openxds::base::String& content );
	virtual       void          sync();
	virtual       void        revert();

	virtual       astral::ast::AST& getMethodAST() { return *this->methodAST; }
~




....			Public const methods

~include/astral/Method.h~
	virtual const astral::ast::AST& getMethodAST() const { return *this->methodAST; }
	virtual       bool                isModified() const { return this->modified; }
~




~include/astral/Method.h~
};};

#endif
~




..		Implementation

~!source/cplusplus/Method.cpp~
#include "astral/CompilationUnit.h"
#include "astral/Method.h"
#include "astral/MethodSignature.h"
#include <astral.ast/AST.h>
#include <astral.tokenizer/SourceToken.h>
#include <openxds.adt/IDictionary.h>
#include <openxds.adt/IEntry.h>
#include <openxds.adt/IPosition.h>
#include <openxds.base/String.h>
#include <openxds/Exception.h>
~



...			Namespaces used

~source/cplusplus/Method.cpp~
using namespace astral;
using namespace astral::ast;
using namespace astral::tokenizer;
using namespace openxds;
using namespace openxds::adt;
using namespace openxds::base;
~




~source/cplusplus/Method.cpp~
Method::Method( AST* methodAST, CompilationUnit& cu, const MethodSignature& aMethodSignature ) : cu( cu )
{
	this->methodAST = methodAST;
	this->signature = new MethodSignature( aMethodSignature );
	this->modified  = false;
}
~




~source/cplusplus/Method.cpp~
Method::~Method()
{
	delete this->methodAST;
	delete this->signature;
}
~




~source/cplusplus/Method.cpp~
void
Method::reparseMethod( const String& content )
{
	AST* pAST = new AST();
	try
	{
		pAST->parseString( content );
		delete this->methodAST;
		this->methodAST = pAST;
		this->modified = true;
	}
	catch ( Exception* ex )
	{
		delete pAST;
		throw;
	}
}
~




~source/cplusplus/Method.cpp~
void
Method::sync()
{
	const char* method_key = this->signature->getMethodKey().getChars();
	IEntry<IPosition<SourceToken> >* e = this->cu.getMethods().find( method_key );
	{
		this->cu.getAST().replaceSubtree( e->getValue(), *this->methodAST );
		this->cu.save();
		this->modified = false;
	}
	delete e;
}
~




~source/cplusplus/Method.cpp~
void
Method::revert()
{
	//
}
~

