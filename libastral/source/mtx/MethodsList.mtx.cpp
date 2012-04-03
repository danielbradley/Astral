
~!include/astral/MethodsList.h~
#ifndef ASTRAL_METHODSLIST_H
#define ASTRAL_METHODSLIST_H
~

~include/astral/MethodsList.h~
#include <astral.h>
#include <astral.tokenizer.h>
#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>
#include <openxds.exceptions/NoSuchElementException.h>
~

~include/astral/MethodsList.h~
using astral::Method;
using astral::ast::AST;
using astral::tokenizer::SourceToken;
using openxds::adt::IDictionary;
using openxds::adt::IIterator;
using openxds::adt::IList;
using openxds::adt::IPosition;
using openxds::adt::ISequence;
using openxds::base::String;
using openxds::exceptions::NoSuchElementException;
~

~include/astral/MethodsList.h~
namespace astral {
~

~include/astral/MethodsList.h~
class MethodsList : openxds::Object
{
private:
	CompilationUnit&                                   cu;
	IDictionary<IPosition<SourceToken> >* methodPositions;
	IDictionary<Method>*                          methods;
	String*                                         empty;

public:
	 MethodsList( CompilationUnit& aCompilationUnit );
	~MethodsList();
	
//	bool         saveMethod( const MethodSignature& aMethodSignature );
	ISequence<MethodSignature>* synchroniseMethods( const MethodSignature& aMethodSignature );

	      Method& getMethod( const MethodSignature& aMethodSignature );
	const Method& getMethod( const MethodSignature& aMethodSignature ) const;
	bool     containsMethod( const MethodSignature& aMethodSignature ) const;

		  IDictionary<IPosition<SourceToken> >& getMethodPositions()       { return *this->methodPositions; }
	const IDictionary<IPosition<SourceToken> >& getMethodPositions() const { return *this->methodPositions; }
	
	
private:
	bool                                  removeMethodFromAST( Method& method );
	ISequence<MethodSignature>* extractAndSyncAnyExtraMethods( Method& method );
};
~

~include/astral/MethodsList.h~
}; // end namespace
~

~include/astral/MethodsList.h~
#endif
~




~!source/cplusplus/MethodsList.cpp~
#include "astral/CompilationUnit.h"
#include "astral/Method.h"
#include "astral/MethodsList.h"
#include "astral/MethodSignature.h"

#include <astral.ast/AST.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/String.h>
~

~source/cplusplus/MethodsList.cpp~
using namespace astral;
using namespace astral::ast;
using namespace openxds::adt;
using namespace openxds::adt::std;
~

~source/cplusplus/MethodsList.cpp~
MethodsList::MethodsList( CompilationUnit& aCompilationUnit ) : cu( aCompilationUnit )
{
	this->methodPositions = new SortedDictionary<IPosition<SourceToken> >();
	this->methods         = new Dictionary<Method>();
	this->empty           = new String();
}
~

~source/cplusplus/MethodsList.cpp~
MethodsList::~MethodsList()
{
	delete this->empty;
	delete this->methods;
	delete this->methodPositions;
}
~


..		Retrieving methods from underlying AST



~source/cplusplus/MethodsList.cpp~
Method&
MethodsList::getMethod( const MethodSignature& aMethodSignature )
{
	Method* method = null;
	
	const char* method_key = aMethodSignature.getMethodKey().getChars();
	
	try
	{
		IEntry<Method>* e = this->methods->find( method_key );
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
			IEntry<IPosition<SourceToken> >* e = this->methodPositions->find( method_key );
			{
				method = new Method( this->cu, *this, aMethodSignature, e->getValue() );
				this->methods->insert( method_key, method );
			}
			delete e;
		}
		catch ( NoSuchElementException* ex )
		{
			delete ex;
			method = new Method( this->cu, *this, aMethodSignature );
			this->methods->insert( method_key, method );
		}
	}
	
	return *method;
}
~

~source/cplusplus/MethodsList.cpp~
const Method&
MethodsList::getMethod( const MethodSignature& aMethodSignature ) const
{
	return const_cast<MethodsList*>( this )->getMethod( aMethodSignature );
}
~




~source/cplusplus/MethodsList.cpp~
bool
MethodsList::containsMethod( const MethodSignature& aMethodSignature ) const
{
	return this->methodPositions->containsKey( aMethodSignature.getMethodKey().getChars() );
}
~




Old save method

~source/cplusplus/MethodsList.cpp~
//bool
//MethodsList::saveMethod( const MethodSignature& aMethodSignature )
//{
//	Method& method = this->getMethod( aMethodSignature );
//	return method.oldsync();
//}
~

New save method

~source/cplusplus/MethodsList.cpp~
ISequence<MethodSignature>*
MethodsList::synchroniseMethods( const MethodSignature& aMethodSignature )
{
	ISequence<MethodSignature>* method_signatures = NULL;
	{
		Method& method = this->getMethod( aMethodSignature );

		if ( method.isEmpty() )
		{
			if ( removeMethodFromAST( method ) ) method_signatures = new Sequence<MethodSignature>();
		}
		else if ( method.isModified() )
		{
			method_signatures = extractAndSyncAnyExtraMethods( method );
			if ( method_signatures )
			{
				const ClassSignature& cls = aMethodSignature.getClassSignature();

				if ( method.sync() )
				{
					const char*      method_key = method.getSignature().getMethodKey().getChars();
					MethodSignature* first      = new MethodSignature( cls, method_key );

					method_signatures->addFirst( first );
				}
				else
				{
					delete method_signatures;
					method_signatures = NULL;
				}
			}
		}
	}
	return method_signatures;
}
~


~source/cplusplus/MethodsList.cpp~
bool
MethodsList::removeMethodFromAST( Method& method )
{
	bool status = false;

	const char* method_key = method.getSignature().getMethodKey().getChars();
	if ( (status = method.removeMethod( method_key )) )
	{
		try
		{
			IEntry<IPosition<SourceToken> >* e = this->methodPositions->find( method_key );
			delete this->methodPositions->remove( e );
		}
		catch ( NoSuchElementException* ex )
		{
			delete ex;
		}
		
		try
		{
			IEntry<Method>* e = this->methods->find( method_key );
			delete this->methods->remove( e );
		}
		catch ( NoSuchElementException* ex )
		{
			delete ex;
		}
	}

	return status;
}
~

~source/cplusplus/MethodsList.cpp~
ISequence<MethodSignature>*
MethodsList::extractAndSyncAnyExtraMethods( Method& method )
{
	ISequence<MethodSignature>* method_signatures = new Sequence<MethodSignature>();
	{
		ISequence<Method>* extra_methods = method.extractExtraMethods();
		if ( extra_methods )
		{
			ClassSignature class_signature( this->cu.getFQName() );
			while ( ! extra_methods->isEmpty() )
			{
				Method* method = extra_methods->removeLast();
				if ( method->sync() )
				{
					const char* method_key = method->getSignature().getMethodKey().getChars();
					this->methods->insert( method_key, method );
					method_signatures->addLast( new MethodSignature( class_signature, method_key ) );
				}
				else
				{
					delete method_signatures;
					method_signatures = NULL;
					break;
				}
			}
		}
		delete extra_methods;
	}
	return method_signatures;
}
~







