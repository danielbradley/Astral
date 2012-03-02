#include "astral/CompilationUnit.h"
#include "astral/Method.h"
#include "astral/MethodSignature.h"
#include <astral.ast/AST.h>
#include <astral.tokenizer/SourceToken.h>
#include <openxds.adt/IDictionary.h>
#include <openxds.adt/IEntry.h>
#include <openxds.adt/IPosition.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds/Exception.h>
#include <openxds.exceptions/NoSuchElementException.h>

#include <stdio.h>

using namespace astral;
using namespace astral::ast;
using namespace astral::tokenizer;
using namespace openxds;
using namespace openxds::adt;
using namespace openxds::base;
using namespace openxds::exceptions;

Method::Method( CompilationUnit& cu, const MethodSignature& aMethodSignature ) : cu( cu )
{
	this->methodAST = new AST();
	this->signature = new MethodSignature( aMethodSignature );
	this->modified  = false;
	
	const char* return_type = aMethodSignature.getReturnType().getChars();
	const char* method_call = aMethodSignature.getMethodCall().getChars();
	
	StringBuffer sb;
	sb.append( FormattedString( "public %s %s\n", return_type, method_call ) );
	sb.append( "{\n" );
	sb.append( "}\n" );
	
	this->reparseMethod( sb.getContent() );
}

Method::Method( CompilationUnit& cu, const MethodSignature& aMethodSignature, AST* methodAST ) : cu( cu )
{
	this->methodAST = methodAST;
	this->signature = new MethodSignature( aMethodSignature );
	this->modified  = false;
}

Method::~Method()
{
	delete this->methodAST;
	delete this->signature;
}

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
		
		fprintf( stdout, "Method::reparseMethod: isValid %i\n", this->isValid() );
	}
	catch ( Exception* ex )
	{
		delete pAST;
		throw;
	}
}

bool
Method::sync()
{
	bool can_save = this->isValid();

	//	!!! Danger Will Robinson !!!
	//
	//	For an unknown reason if the Method is synced back to the main AST before being
	//	modified it corrupts the main AST.
	//
	//	TODO	Fix in the future.

	if ( can_save && this->modified )
	{
		const char* method_key = this->signature->getMethodKey().getChars();

		try
		{
			IEntry<IPosition<SourceToken> >* e = this->cu.getMethods().find( method_key );
			{
				this->cu.getAST().replaceSubtree( e->getValue(), *this->methodAST );
				this->cu.save();
				this->modified = false;
			}
			delete e;
		}
		catch ( NoSuchElementException* ex )
		{
			delete ex;

			if ( this->cu.insertNewMethod( method_key, *this->methodAST ) )
			{
				this->cu.save();
				this->modified = false;
			}
		}
	}
	
	return can_save;
}

void
Method::revert()
{
	//
}

bool
Method::isValid() const
{
	return this->methodAST->isValid();
}

