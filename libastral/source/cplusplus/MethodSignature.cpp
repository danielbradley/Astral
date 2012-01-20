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

