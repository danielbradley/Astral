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

MethodSignature::MethodSignature( const ClassSignature& aClassSignature, const char* methodName, const char* parameters, const char* rType )
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
	sb.append( methodName );
	sb.append( "(" );
	sb.append( parameters );
	sb.append( ")" );
	sb.append( "|" );
	sb.append( rType );
	
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
			   
		if ( ! this->returnType ) abort();
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

MethodSignature*
MethodSignature::createInContext( const String& original, const String& className )
{
	MethodSignature* sig = new MethodSignature( original );
	if ( ! sig->isValid() )
	{
		if ( sig->getMethodName().contentEquals( className ) )
		{
			delete sig;
			       sig = new MethodSignature( FormattedString( "%s%s", original.getChars(), className.getChars() ) );
		}
		else
		{
			delete sig;
			       sig = NULL;
		}
	}
	return sig;
}




