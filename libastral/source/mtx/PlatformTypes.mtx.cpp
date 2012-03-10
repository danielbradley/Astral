.	Platforms Types

The /Platform Types/ class encapsulates a dictionary that can be queried to determine
platform types that are valid.


~!include/astral/PlatformTypes.h~
#ifndef ASTRAL_PLATFORMTYPES_H
#define ASTRAL_PLATFORMTYPES_H

#include "astral.h"

#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds.exceptions/NoSuchElementException.h>

using openxds::adt::IDictionary;
using openxds::base::String;
using openxds::exceptions::NoSuchElementException;

namespace astral {

class PTypes
{
private:
	IDictionary<String>* dictionary;
	
public:
	         PTypes();
	virtual ~PTypes();

	virtual const String& resolve( const char* type ) const throw (NoSuchElementException*);
	virtual       bool    hasType( const char* type ) const;
	
private:
	virtual void populate();
};

};

#endif
~




~!source/cplusplus/PlatformTypes.cpp~

#include "astral/PlatformTypes.h"

#include <openxds.adt/IDictionary.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.base/String.h>

using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;

PTypes::PTypes()
{
	this->dictionary = new Dictionary<String>();
	this->populate();
}

PTypes::~PTypes()
{
	delete this->dictionary;
}

const String&
PTypes::resolve( const char* type ) const
throw (NoSuchElementException*)
{
	IEntry<String>* e = this->dictionary->find( type );
	const String& str = e->getValue();
	delete e;
	return str;
}

bool
PTypes::hasType( const char* type ) const
{
	try
	{
		delete this->dictionary->find( type );
		return true;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
		return false;
	}
}
~

~source/cplusplus/PlatformTypes.cpp~
void
PTypes::populate()
{
	//	java.lang Interfaces
	this->dictionary->insert( "Cloneable",     new String( "java.lang.Cloneable" ) ); 
	this->dictionary->insert( "Iterable",      new String( "java.lang.Iterable"  ) ); 
	this->dictionary->insert( "Runnable",      new String( "java.lang.Runnable"  ) ); 

	//	java.lang Classes
	this->dictionary->insert( "Character",     new String( "java.lang.Character"    ) ); 
	this->dictionary->insert( "String",        new String( "java.lang.String"       ) ); 
	this->dictionary->insert( "StringBuffer",  new String( "java.lang.StringBuffer" ) ); 
	
	//	java.io Classes
	this->dictionary->insert( "File",           new String( "java.io.File"           ) ); 
	this->dictionary->insert( "Reader",         new String( "java.io.Reader"         ) ); 
	this->dictionary->insert( "PushbackReader", new String( "java.io.PushbackReader" ) ); 

}
~

