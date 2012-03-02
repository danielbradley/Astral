.	Signature classes

~!include/astral/ClassSignature.h~
#ifndef ASTRAL_CLASSSIGNATURE_H
#define ASTRAL_CLASSSIGNATURE_H

#include <openxds.base.h>

using openxds::base::String;

namespace astral {

class ClassSignature
{
private:
	String* fqClass;				//  openxds.base.String
	String* nspace;					//	openxds.base
	String* cls;					//	String

public:
	 ClassSignature();
	 ClassSignature( const String&         fqClass   );
	 ClassSignature( const ClassSignature& signature );
	~ClassSignature();

	virtual void initialise( const String& fqClass );

	virtual const String&   getFQClass() const { return *this->fqClass; }
	virtual const String& getNamespace() const { return *this->nspace;  }
	virtual const String& getClassName() const { return *this->cls;     }

	virtual         bool    isComplete() const;
};

};

#endif
~

~!source/cplusplus/ClassSignature.cpp~
#include "astral/ClassSignature.h"

#include <openxds.base/Character.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.base/StringTokenizer.h>
#include <openxds.adt.std/Sequence.h>

using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
~

~source/cplusplus/ClassSignature.cpp~
ClassSignature::ClassSignature()
{
	this->fqClass = NULL;
	this->nspace  = NULL;
	this->cls     = NULL;
}
~

~source/cplusplus/ClassSignature.cpp~
ClassSignature::ClassSignature( const String& fqClass )
{
	this->initialise( fqClass );
}
~

~source/cplusplus/ClassSignature.cpp~
ClassSignature::ClassSignature( const ClassSignature& signature )
{
	this->fqClass = new String( *signature.fqClass );
	this->nspace  = new String( *signature.nspace  );
	this->cls     = new String( *signature.cls     );
}
~

~source/cplusplus/ClassSignature.cpp~
ClassSignature::~ClassSignature()
{
	delete this->fqClass;
	delete this->nspace;
	delete this->cls;
}
~

~source/cplusplus/ClassSignature.cpp~
void
ClassSignature::initialise( const String& fqClass )
{
	this->fqClass = new String( fqClass );

	Sequence<String> bits;

	StringTokenizer st( fqClass );
	st.setDelimiter( '.' );
	while ( st.hasMoreTokens() )
	{
		bits.addLast( st.nextToken() );
	}

	this->cls = !bits.isEmpty() ? bits.removeLast() : new String();

	StringBuffer sb;
	{
		IIterator<String>* it = bits.elements();
		while ( it->hasNext() )
		{
			sb.append( it->next() );
			sb.append( "." );
		}
		delete it;
		if ( 0 < sb.getLength() ) sb.removeLast();
	}
	this->nspace = sb.asString();
}

bool
ClassSignature::isComplete() const
{
	bool complete = false;
	if ( this->nspace->getLength() && this->cls->getLength() )
	{
		complete = Character::isUppercase( this->cls->charAt(0) );
	}
	return complete;
}
~

~!include/astral/MethodSignature.h~
#ifndef ASTRAL_METHODSIGNATURE_H
#define ASTRAL_METHODSIGNATURE_H

#include <astral.h>
#include <astral/ClassSignature.h>
#include <openxds.base.h>

using openxds::base::String;

namespace astral {

class MethodSignature
{
private:
	String*               original;		//	openxds.base.String.charAt(int)|char
	ClassSignature* classSignature;		//	openxds.base.String
	String*           fqMethodCall;		//	openxds.base.String.charAt(int)
	String*             returnType;		//	char
	String*             methodCall;		//	charAt(int)
	String*             methodName;		//	charAt
	String*              methodKey;		//	charAt(int)|char

public:
	 MethodSignature();
	 MethodSignature( const String&          original  );
	 MethodSignature( const ClassSignature& aClassSignature, const String& aMethodKey );
	 MethodSignature( const MethodSignature& signature );
	~MethodSignature();

	virtual void                              initialise( const openxds::base::String& aSignature );

	virtual const String&               getOriginal() const { return *this->original;       }
	virtual const ClassSignature& getClassSignature() const { return *this->classSignature; }
	virtual const String&           getFQMethodCall() const { return *this->fqMethodCall;   }
	virtual const String&             getReturnType() const { return *this->returnType;     }
	virtual const String&             getMethodCall() const { return *this->methodCall;     }
	virtual const String&             getMethodName() const { return *this->methodName;     }
	virtual const String&              getMethodKey() const { return *this->methodKey;      }

	virtual const String&                getFQClass() const { return this->classSignature->getFQClass();   }
	virtual const String&              getNamespace() const { return this->classSignature->getNamespace(); }
	virtual const String&              getClassName() const { return this->classSignature->getClassName();      }

	virtual bool                            isValid() const;
	virtual bool                         isComplete() const;
};

};

#endif
~

~!source/cplusplus/MethodSignature.cpp~
#include "astral/ClassSignature.h"
#include "astral/MethodSignature.h"

#include <openxds.base/FormattedString.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.base/StringTokenizer.h>
#include <openxds.adt.std/Sequence.h>

using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
~

To the best of its ability, parses the passed MethodSignature.
Those elements that cannot be found end up as empty strings.

~source/cplusplus/MethodSignature.cpp~
MethodSignature::MethodSignature()
{
	this->original       = new String();
	this->classSignature = new ClassSignature();
	this->fqMethodCall   = new String();
	this->returnType     = new String();
	this->methodCall     = new String();
	this->methodName     = new String();
	this->methodKey      = new String();
}

MethodSignature::MethodSignature( const String& aSignature )
{
	this->original       = new String();
	this->classSignature = new ClassSignature();
	this->fqMethodCall   = new String();
	this->returnType     = new String();
	this->methodCall     = new String();
	this->methodName     = new String();
	this->methodKey      = new String();

	this->initialise( aSignature );
}

MethodSignature::MethodSignature( const ClassSignature& aClassSignature, const String& aMethodKey )
{
	this->original       = new String();
	this->classSignature = new ClassSignature();
	this->fqMethodCall   = new String();
	this->returnType     = new String();
	this->methodCall     = new String();
	this->methodName     = new String();
	this->methodKey      = new String();

	StringBuffer sb;
	sb.append( aClassSignature.getFQClass() );
	sb.append( "." );
	sb.append( aMethodKey );
	
	const char* signature = sb.getChars();

	this->initialise( signature );
}

MethodSignature::MethodSignature( const MethodSignature& aMethodSignature )
{
	this->original       = new String();
	this->classSignature = new ClassSignature();
	this->fqMethodCall   = new String();
	this->returnType     = new String();
	this->methodCall     = new String();
	this->methodName     = new String();
	this->methodKey      = new String();

	this->initialise( aMethodSignature.getOriginal() );
}




MethodSignature::~MethodSignature()
{
	delete this->original;
	delete this->classSignature;
	delete this->fqMethodCall;
	delete this->returnType;
	delete this->methodCall;
	delete this->methodName;
	delete this->methodKey;
}




void
MethodSignature::initialise( const String& aSignature )
{
	delete this->original;
	       this->original = new String( aSignature );

	StringTokenizer st( aSignature );
	st.setDelimiter( '|' );
	{
		delete this->fqMethodCall;
		       this->fqMethodCall = st.hasMoreTokens() ? st.nextToken() : new String();

		delete this->returnType;
		       this->returnType = st.hasMoreTokens() ? st.nextToken() : new String();
	}
	
	Sequence<String> bits;
	{
		{
			StringTokenizer st2( *this->fqMethodCall );
			st2.setDelimiter( '.' );
			while ( st2.hasMoreTokens() )
			{
				bits.addLast( st2.nextToken() );
			}
	
			delete this->methodCall;
			       this->methodCall = !bits.isEmpty() ? bits.removeLast() : new String();

			if ( !this->methodCall->contains( "(" ) )
			{
				bits.addLast( this->methodCall );
				this->methodCall = new String();
			}
		}

		{
			StringBuffer sb;
			{
				IIterator<String>* it = bits.elements();
				while ( it->hasNext() )
				{
					sb.append( it->next() );
					sb.append( "." );
				}
				delete it;
			}
			if ( 0 < sb.getLength() ) sb.removeLast();
			delete this->classSignature;
			       this->classSignature = new ClassSignature( sb.getChars() );
		}

		{
			StringTokenizer st3( *this->methodCall );
			st3.setDelimiter( '(' );
			{
				delete this->methodName;
				       this->methodName = st3.hasMoreTokens() ? st3.nextToken() : new String();
			}
		}
		
		{
			StringBuffer sb;
			sb.append( *this->methodCall );
			sb.append( "|" );
			sb.append( *this->returnType );
			delete this->methodKey;
			       this->methodKey = sb.asString();
		}
	}
}



bool
MethodSignature::isValid() const
{
	return (0 < this->returnType->getLength() );
}



bool
MethodSignature::isComplete() const
{
	return (this->getNamespace().getLength() &&
	        this->getClassName().getLength() &&
			this->methodCall->getLength()    &&
			this->returnType->getLength() );
}
~
