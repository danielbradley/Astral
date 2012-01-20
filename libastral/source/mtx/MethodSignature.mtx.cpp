,,		MethodSignature class

~!include/astral/MethodSignature.h~
#ifndef ASTRAL_METHODSIGNATURE_H
#define ASTRAL_METHODSIGNATURE_H

#include <openxds.base.h>

namespace astral {

class MethodSignature
{
private:
	openxds::base::String* original;				//	openxds.base.String.chatAt(int)|char
	openxds::base::String* methodCall;				//	openxds.base.String.charAt(int)|char
	openxds::base::String* returnType;				//	char
	openxds::base::String* nspace;					//	openxds.base
	openxds::base::String* cls;						//	String
	openxds::base::String* fqClass;                 //  openxds.base.String
	openxds::base::String* method;					//	charAt(int)
	openxds::base::String* methodName;				//	charAt
	openxds::base::String* methodKey;				//	charAt(int)|char

public:
	                                     MethodSignature();
	                                     MethodSignature( const openxds::base::String& aSignature );
	                                     MethodSignature( const MethodSignature& aMethodSignature );
	virtual                             ~MethodSignature();

	virtual const openxds::base::String&     getOriginal() const { return *this->original;   }
	virtual const openxds::base::String&   getMethodCall() const { return *this->methodCall; }
	virtual const openxds::base::String&   getReturnType() const { return *this->returnType; }
	virtual const openxds::base::String&    getNamespace() const { return *this->nspace;     }
	virtual const openxds::base::String&        getClass() const { return *this->cls;        }
	virtual const openxds::base::String&      getFQClass() const { return *this->fqClass;    }
	virtual const openxds::base::String&       getMethod() const { return *this->method;     }
	virtual const openxds::base::String&   getMethodName() const { return *this->methodName; }
	virtual const openxds::base::String&    getMethodKey() const { return *this->methodKey; }

protected:
	virtual       void                        initialise( const openxds::base::String& aSignature );

};

};

#endif
~



~!source/cplusplus/MethodSignature.cpp~
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
	this->original   = new String();
	this->nspace     = new String();
	this->cls        = new String();
	this->fqClass    = new String();
	this->method     = new String();
	this->returnType = new String();
	this->methodName = new String();
	this->methodCall = new String();
}




MethodSignature::MethodSignature( const String& aSignature )
{
	this->initialise( aSignature );
}




MethodSignature::MethodSignature( const MethodSignature& aMethodSignature )
{
	this->initialise( aMethodSignature.getOriginal() );
}




MethodSignature::~MethodSignature()
{
	delete this->original;
	delete this->nspace;
	delete this->cls;
	delete this->fqClass;
	delete this->method;
	delete this->returnType;
	delete this->methodName;
	delete this->methodCall;
}




void
MethodSignature::initialise( const String& aSignature )
{
	this->original = new String( aSignature );

	StringTokenizer st( aSignature );
	st.setDelimiter( '|' );
	{
		this->methodCall = st.hasMoreTokens() ? st.nextToken() : new String();
		this->returnType = st.hasMoreTokens() ? st.nextToken() : new String();
	}
	
	Sequence<String> bits;
	{
		StringTokenizer st2( *this->methodCall );
		st2.setDelimiter( '.' );
		while ( st2.hasMoreTokens() )
		{
			bits.addLast( st2.nextToken() );
		}
	
		this->method = !bits.isEmpty() ? bits.removeLast() : new String();
		this->cls    = !bits.isEmpty() ? bits.removeLast() : new String();
	
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
		
		StringTokenizer st3( *this->method );
		st3.setDelimiter( '(' );
		{
			this->methodName = st3.hasMoreTokens() ? st3.nextToken() : new String();
		}
		
		{
			StringBuffer sb3;
			sb3.append( *this->method );
			sb3.append( '|' );
			sb3.append( *this->returnType );
		
			this->methodKey = sb3.asString();
		}
		
		this->fqClass = new FormattedString( "%s.%s", this->nspace->getChars(), this->cls->getChars() );
	}
}
~