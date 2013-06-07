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
	this->valid = false;

	this->original       = new String();
	this->classSignature = new ClassSignature();
	this->fqMethodCall   = new String();
	this->fqMethod       = new String();
	this->arguments      = new String();
	this->returnType     = new String();
	this->methodCall     = new String();
	this->methodName     = new String();
	this->methodKey      = new String();
}

MethodSignature::MethodSignature( const String& aSignature )
{
	this->valid = false;

	this->original       = new String();
	this->classSignature = new ClassSignature();
	this->fqMethodCall   = new String();
	this->fqMethod       = new String();
	this->arguments      = new String();
	this->returnType     = new String();
	this->methodCall     = new String();
	this->methodName     = new String();
	this->methodKey      = new String();

	this->initialise( aSignature );
}

MethodSignature::MethodSignature( const ClassSignature& aClassSignature, const String& aMethodKey )
{
	this->valid = false;

	this->original       = new String();
	this->classSignature = new ClassSignature();
	this->fqMethodCall   = new String();
	this->fqMethod       = new String();
	this->arguments      = new String();
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
	this->valid = false;

	this->original       = new String();
	this->classSignature = new ClassSignature();
	this->fqMethodCall   = new String();
	this->fqMethod       = new String();
	this->arguments      = new String();
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
	this->valid = false;

	this->original       = new String();
	this->classSignature = new ClassSignature();
	this->fqMethodCall   = new String();
	this->fqMethod       = new String();
	this->arguments      = new String();
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
	delete this->fqMethod;
	delete this->arguments;
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

	if ( this->original->contains( "(" ) )
	{
		this->initialiseFQMethodCallAndReturnType( aSignature );
		this->initialiseFQMethodAndArguments( *this->fqMethodCall );
		this->initialiseMethodNameAndMethodCall( *this->fqMethod, *this->arguments );
		this->initialiseClassSignature( *this->fqMethod );
		this->initialiseMethodKey( *this->methodCall, *this->returnType );
	}
	else
	{
		delete this->classSignature;
		       this->classSignature = new ClassSignature( *this->original );
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


bool
MethodSignature::initialiseFQMethodCallAndReturnType( const String& aSignature )
{
	bool ret = true;

	StringTokenizer st( aSignature );
	st.setDelimiter( '|' );
	{
		if ( (ret &= st.hasMoreTokens()) )
		{
			delete this->fqMethodCall;
			       this->fqMethodCall = st.nextToken();
		}

		if ( (ret &= st.hasMoreTokens()) )
		{
			delete this->returnType;
		           this->returnType = st.nextToken();
		}
	}
	return ret;
}

bool
MethodSignature::initialiseFQMethodAndArguments( const String& fqMethodCall )
{
	bool ret = true;

	StringTokenizer st( fqMethodCall );
	st.setDelimiter( '(' );
	{
		if ( (ret &= st.hasMoreTokens()) )
		{
			delete this->fqMethod;
				   this->fqMethod = st.nextToken();
		}
		
		if ( (ret &= st.hasMoreTokens()) )
		{
			String* s = st.nextToken();
			{
				delete this->arguments;
					   this->arguments = new FormattedString( "(%s", s->getChars() );
			}
			delete s;
		}
	}
	
	return ret;
}

bool
MethodSignature::initialiseMethodNameAndMethodCall( const String& fqMethod, const String& arguments )
{
	bool ret = true;
	
	Sequence<String> bits;
	{
		StringTokenizer st( fqMethod );
		st.setDelimiter( '.' );
		while ( st.hasMoreTokens() )
		{
			bits.addLast( st.nextToken() );
		}

		if ( (ret = !bits.isEmpty()) )
		{
			delete this->methodName;
			       this->methodName = bits.removeLast();
	
			delete this->methodCall;
			       this->methodCall = new FormattedString( "%s%s", this->methodName->getChars(), arguments.getChars() );
		}
	}
	
	return ret;
}

bool
MethodSignature::initialiseClassSignature( const String& fqMethod )
{
	bool ret = true;
	{
		Sequence<String> bits;
		StringBuffer     sb;
		StringTokenizer  st( fqMethod );

		st.setDelimiter( '.' );
		while ( st.hasMoreTokens() )
		{
			bits.addLast( st.nextToken() );
		}
		
		bits.removeLast(); // Remove method name.
		
		IIterator<String>* it = bits.elements();
		while ( it->hasNext() )
		{
			sb.append( it->next() );
			sb.append( "." );
		}
		delete it;

		if ( 0 < sb.getLength() ) sb.removeLast();
		
		if ( (ret = (0 < sb.getLength())) )
		{
			delete this->classSignature;
			       this->classSignature = new ClassSignature( sb.getChars() );
		}
	}
	return ret;
}

bool
MethodSignature::initialiseMethodKey( const String& methodCall, const String& returnType )
{
	bool ret = (0 < methodCall.getLength() && (0 < returnType.getLength()));

	if ( ret )
	{
		delete this->methodKey;
		       this->methodKey = new FormattedString( "%s|%s", methodCall.getChars(), returnType.getChars() );
	}
	
	return ret;
}
