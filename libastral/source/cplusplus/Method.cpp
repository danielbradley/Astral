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

using namespace astral;
using namespace astral::ast;
using namespace astral::tokenizer;
using namespace openxds;
using namespace openxds::adt;
using namespace openxds::base;

Method::Method( AST* methodAST, CompilationUnit& cu, const MethodSignature& aMethodSignature ) : cu( cu )
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
	}
	catch ( Exception* ex )
	{
		delete pAST;
		throw;
	}
}

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

void
Method::revert()
{
	//
}

