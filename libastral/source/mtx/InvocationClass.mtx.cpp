~source/cobalt/InvocationClass.cpp~
namespace astral;

#include <openxds*>

use openxds.base.String;
use openxds.base.StringBuffer;
use openxds.adt.std.Sequence;

class InvocationClass
{
	@lastType               String*;              
	@methodCallReturnTypes  Sequence<String>*;
	@compoundString         StringBuffer*;

	public InvocationClass()
	{
		@lastType              := new String();
		@methodCallReturnTypes := new Sequence();
		@compoundString        := new StringBuffer();
	}

	public reset() : void
	{
		@lastReturnType        := new String();
		@methodCallReturnTypes := new Sequence();
		@compondString         := new StringBuffer();
	}

	public setLastType( aType : String& ) : void
	{
		@lastType        := new String( aType );
	}

	public addMethodCallReturnType( String& aReturnType )
	{
		@methodCallReturnTypes.addLast( new String( aReturnType ) );
	}

	public appendSelector() : void
	{
		@compoundString.append( "." );
	}

	public appendName( aName : String& )
	{
		@compoundString.append( aName );
	}

	public const getLastType() : const String&
	{
		return @lastType;
	}
	
	public const getCompoundString() : const String&
	{
		return @compoundString.getContent();
	}

} end class;
~

~!include/astral/InvocationClass.h~
#include <openxds/Object.h>
#include <openxds.adt.h>
#include <openxds.base.h>
~

~include/astral/InvocationClass.h~
namespace astral {
~

~include/astral/InvocationClass.h~
class InvocationClass : openxds::Object
{
private:
	openxds::base::String*                                       lastType;
	openxds::base::String*                                   platformType;
	openxds::adt::ISequence<openxds::base::String>* methodCallReturnTypes;
	openxds::base::StringBuffer*                           compoundString;
	bool                                                           primed;

public:
	 InvocationClass();
	~InvocationClass();
	
	virtual void                                     reset();
	
	virtual void                                clearTypes();
	virtual void                             clearLastType();

	virtual void                                   unprime() { this->primed = false; }
	virtual void                            setUnknownName( const openxds::base::String& aName );
	virtual void                               setLastType( const openxds::base::String& aType );
	virtual void                               setLastType(       openxds::base::String* aType );
	virtual void                           setPlatformType( const openxds::base::String& aType );

	virtual void                                appendName( const openxds::base::String& aName );

	virtual void                            pushReturnType();
	virtual void                             popReturnType();

	virtual bool                                 wasPrimed() const { return this->primed; }
	virtual bool                          hasEnclosingType() const;
	virtual bool                               hasLastType() const;
	virtual bool                           hasPlatformType() const;
	virtual bool                         hasCompoundString() const;

	virtual const openxds::base::String&  getEnclosingType() const;
	virtual const openxds::base::String&       getLastType() const;
	virtual const openxds::base::String&   getPlatformType() const;
	virtual const openxds::base::String& getCompoundString() const;
~

~include/astral/InvocationClass.h~
}; // end class
~

~include/astral/InvocationClass.h~
}; // end namespace
~

~!source/cplusplus/InvocationClass.cpp~
#include "astral/InvocationClass.h"
~

~source/cplusplus/InvocationClass.cpp~
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.exceptions/NoSuchElementException.h>
~

~source/cplusplus/InvocationClass.cpp~
using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::exceptions;
~

~source/cplusplus/InvocationClass.cpp~
InvocationClass::InvocationClass()
{
	this->lastType              = new String();
	this->platformType          = new String();
	this->methodCallReturnTypes = new Sequence<String>();
	this->compoundString        = new StringBuffer();
	this->primed                = false;
}
~

~source/cplusplus/InvocationClass.cpp~
InvocationClass::~InvocationClass()
{
	delete this->lastType;
	delete this->platformType;
	delete this->methodCallReturnTypes;
	delete this->compoundString;
}
~

~source/cplusplus/InvocationClass.cpp~
void
InvocationClass::reset()
{
	delete this->lastType;
	       this->lastType = new String();

	delete this->platformType;
	       this->platformType = new String();

	delete this->methodCallReturnTypes;
	       this->methodCallReturnTypes = new Sequence<String>();

	delete this->compoundString;
	       this->compoundString = new StringBuffer();
		   
	this->primed = false;
}
~

~source/cplusplus/InvocationClass.cpp~
void
InvocationClass::clearTypes()
{
	delete this->lastType;
	       this->lastType = new String();

	delete this->platformType;
	       this->platformType = new String();

	delete this->compoundString;
	       this->compoundString = new StringBuffer();
}
~

~source/cplusplus/InvocationClass.cpp~
void
InvocationClass::clearLastType()
{
	delete this->lastType;
	       this->lastType = new String();
}
~

~source/cplusplus/InvocationClass.cpp~
void
InvocationClass::setUnknownName( const String& aName )
{
	if ( this->hasLastType() )
	{
		this->appendName( this->getLastType() );
	}
	this->appendName( aName );

	this->primed = true;
}
~

~source/cplusplus/InvocationClass.cpp~
void
InvocationClass::setLastType( const String& aType )
{
	this->clearTypes();

	delete this->lastType;
	       this->lastType = new String( aType );

	this->primed = true;
}
~

~source/cplusplus/InvocationClass.cpp~
void
InvocationClass::setLastType( String* aType )
{
	this->clearTypes();

	delete this->lastType;
	       this->lastType = new String( *aType );

	delete aType;

	this->primed = true;
}
~

~source/cplusplus/InvocationClass.cpp~
void
InvocationClass::setPlatformType( const String& aType )
{
	this->clearTypes();

	delete this->platformType;
	       this->platformType = new String( aType );

	this->primed = true;
}
~

~source/cplusplus/InvocationClass.cpp~
void
InvocationClass::appendName( const String& aName )
{
	if ( this->compoundString->getLength() )
	{
		this->compoundString->append( "." );
	}

	this->compoundString->append( aName );

	this->clearLastType();
	
	this->primed = true;
}
~

~source/cplusplus/InvocationClass.cpp~
void
InvocationClass::pushReturnType()
{
	this->methodCallReturnTypes->addLast( this->lastType->asString() );
	
	this->primed = false;
}
~

~source/cplusplus/InvocationClass.cpp~
void
InvocationClass::popReturnType()
{
	try
	{
		this->setLastType( this->methodCallReturnTypes->removeLast() );
	} catch ( NoSuchElementException* ex )
	{
		fprintf( stderr, "!!! InvocationClass::popReturnType: NoSuchElementException thrown.\n" );
		delete ex;
	}
	this->primed = true;
}
~

~source/cplusplus/InvocationClass.cpp~
bool
InvocationClass::hasEnclosingType() const
{
	return (this->hasLastType() || this->hasPlatformType() || this->hasCompoundString() );
}
~

~source/cplusplus/InvocationClass.cpp~
bool
InvocationClass::hasLastType() const
{
	return (0 != this->lastType->getLength());
}
~

~source/cplusplus/InvocationClass.cpp~
bool
InvocationClass::hasPlatformType() const
{
	return (0 != this->platformType->getLength());
}
~

~source/cplusplus/InvocationClass.cpp~
bool
InvocationClass::hasCompoundString() const
{
	return (0 != this->compoundString->getLength());
}
~

~source/cplusplus/InvocationClass.cpp~
const String&
InvocationClass::getEnclosingType() const
{
	if ( this->hasCompoundString() )
	{
		return this->getCompoundString();
	}
	else if ( this->hasPlatformType() )
	{
		return this->getPlatformType();
	}
	else
	{
		return this->getLastType();
	}
}
~

~source/cplusplus/InvocationClass.cpp~
const String&
InvocationClass::getLastType() const
{
	return *this->lastType;
}
~

~source/cplusplus/InvocationClass.cpp~
const String&
InvocationClass::getPlatformType() const
{
	return *this->platformType;
}
~


~source/cplusplus/InvocationClass.cpp~
const String&
InvocationClass::getCompoundString() const
{
	return this->compoundString->getContent();
}
~
