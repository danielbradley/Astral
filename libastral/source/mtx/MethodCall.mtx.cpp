.	MethodCall

~

Copyright 2012 Daniel Bradley. All rights reserved.

License: LGPLv2

namespace astral;

public class MethodCall
{
	@methodName   : readonly string;
	@parameters   : readonly string[];
	@combinations : readonly string[];
}

public new()
{
	@methodName   = new string();
	@parameters   = new string[];
	@combinations = new string[];
}

public new( methodCall : string& )
{
	string[] strings = methodCall.extact( "%(%)" );
	new ( strings[0], strings[1] );
}

public new( methodName : string&, parameters : string& )
{
	@methodName   = new string( methodName );
	@parameters   = parameters.tokenize( ',' );
	@combinations = new string[];
}

public new( instance : MethodCall& )
{
	@methodName = instance.methodName.copy();
	@parameters = instance.parameters.copy();
	@parameters = instance.combinations.copy();
}

public combinations()
{
	if ( @parameters.length && !@combinations.length )
	{
		@combinations[0] = concat( @parameters, ',' );

		integer c=0;
		integer n=1;

		while ( c < n )
		{
			for ( integer i=0; i < @parameters.length )
			{
				@combinations[n] = increment( c, i );
				if ( @combinations[n] ) n++;
			}
			c++;
		}
	}
}
~




~!include/astral/MethodCall.h~
#ifndef ASTRAL_METHODCALL_H
#define ASTRAL_METHODCALL_H

#include <openxds.adt.h>
#include <openxds.adt.std.h>
#include <openxds.base.h>

using openxds::adt::IMap;
using openxds::adt::ISequence;
using openxds::base::String;

namespace astral {

class MethodCall
{
private:
	String*            methodName;
	String*            methodCall;
	ISequence<String>* parameters;
	ISequence<String>* variations;

public:
	 MethodCall( const char* methodName );
	 MethodCall( const char* methodName, const char* parameters );
	 MethodCall( const char* methodName, const ISequence<String>& parameters );
	~MethodCall();
	
	void             applyParameterisation( const IMap<String>& parameterisation );

	const String&            getMethodCall() const { return *this->methodCall; }
	const String&            getMethodName() const { return *this->methodName; }
	const ISequence<String>& getParameters() const { return *this->parameters; }

	const ISequence<String>& generateVariations();
	
private:
	void variateVariationAtParameter( long c, long i );

};

};

#endif
~

~!source/cplusplus/MethodCall.cpp~
#include "astral/MethodCall.h"

#include <openxds.adt/IMap.h>
#include <openxds.adt/ISequence.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.base/StringTokenizer.h>
#include <openxds.exceptions/NoSuchElementException.h>

using namespace astral;

using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::exceptions;

static ISequence<String>*     explode( const String&            string,  char delimiter );
static String*                 concat( const ISequence<String>& strings, char delimiter );
static ISequence<String>* copyStrings( const ISequence<String>& strings );
static ISequence<String>*    tokenize( const char* pattern, const char* content );

MethodCall::MethodCall( const char* methodCall )
{
	ISequence<String>* tokens = tokenize( "%(%)", methodCall );		//TODO tokenize
	if ( tokens )
	{
		this->methodCall = new String( methodCall );
		this->methodName = new String( tokens->get( 0 ) );
		this->parameters = explode( tokens->get( 1 ), ',' );
		this->variations = new Sequence<String>();
	}
	else
	{
		this->methodCall = new String();
		this->methodName = new String();
		this->parameters = new Sequence<String>();
		this->variations = new Sequence<String>();
	}
}

MethodCall::MethodCall( const char* methodName, const char* parameters )
{
	this->methodName = new String( methodName );
	this->methodCall = new FormattedString( "%s(%s)", methodName, parameters );
	this->parameters = explode( parameters, ',' );
	this->variations = new Sequence<String>();
}

MethodCall::MethodCall( const char* _methodName, const ISequence<String>& parameters )
{
	String* params = concat( parameters, ',' );
	{
		this->methodName = new String( _methodName );
		this->methodCall = new FormattedString( "%s(%s)", _methodName, params->getChars() );
		this->parameters = copyStrings( parameters );
		this->variations = new Sequence<String>();
	}
	delete params;
}

MethodCall::~MethodCall()
{
	delete this->methodName;
	delete this->methodCall;
	delete this->parameters;
	delete this->variations;
}

void
MethodCall::applyParameterisation( const IMap<String>& parameterisation )
{
	StringBuffer sb;
	sb.append( *this->methodName );
	sb.append( '(' );

	long len = this->parameters->size();
	for ( int i=0; i < len; i++ )
	{
		const String& parameter = this->parameters->get(i);
		try
		{
			const String& substitute = parameterisation.get( parameter.getChars() );
			delete this->parameters->set( i, new String( substitute ) );
		}
		catch ( NoSuchElementException* ex )
		{
			delete ex;
		}
		sb.append( this->parameters->get(i) );
		sb.append( ',' );
	}
	if ( len ) sb.removeLast();
	sb.append( ')' );

	delete this->methodCall; this->methodCall = sb.asString();
}

const ISequence<String>&
MethodCall::generateVariations()
{
	if ( 0 == this->variations->size() )
	{
		this->variations->insertLast( new String( *this->methodCall ) );
		
		long nrParameters = 0;
		if ( 0 != (nrParameters = this->parameters->size()) )
		{
			long c=0;
			
			while ( c < this->variations->size() )
			{
				for ( long i=0; i < nrParameters; i++ )
				{
					variateVariationAtParameter( c, i );
				}
				c++;
			}
		}
	}

	return *this->variations;
}

void
MethodCall::variateVariationAtParameter( long c, long i )
{
	const char* base = this->variations->get( c ).getChars();

	MethodCall mc( base );
	ISequence<String>* parameters = copyStrings( mc.getParameters() );
	{
		bool hit = false;
	
		const String& target_parameter = parameters->get( i );
		if ( 0 < target_parameter.getLength() )
		{
			switch ( target_parameter.charAt( 0 ) )
			{
			case 's': // short
				if ( target_parameter.contentEquals( "short" ) )
				{
					delete parameters->set( i, new String( "int" ) );
					hit = true;
				}
				break;
			case 'i': // int
				if ( target_parameter.contentEquals( "int" ) )
				{
					delete parameters->set( i, new String( "long" ) );
					hit = true;
				}
				break;
			case 'f': // float
				if ( target_parameter.contentEquals( "float" ) )
				{
					delete parameters->set( i, new String( "double" ) );
					hit = true;
				}
				break;
			default:
				break;
			}

			if ( hit )
			{
				MethodCall new_variant( mc.getMethodName().getChars(), *parameters );
				this->variations->insertLast( new String( new_variant.getMethodCall() ) );
			}
		}
	}
	delete parameters;
}
~

~source/cplusplus/MethodCall.cpp~
static ISequence<String>* explode( const String& string, const char delimiter )
{
	ISequence<String>* seq = new Sequence<String>();
	{
		StringTokenizer st( string );
		st.setDelimiter( delimiter );
		while ( st.hasMoreTokens() )
		{
			seq->insertLast( st.nextToken() );
		}
	}
	return seq;
}

static String* concat( const ISequence<String>& strings, const char delimiter )
{
	StringBuffer sb;
	const IIterator<String>* it = strings.elements();
	while ( it->hasNext() )
	{
		sb.append( it->next() );
		sb.append( delimiter );
	}
	delete it;

	sb.removeLast();
	
	return sb.asString();
}

static ISequence<String>* copyStrings( const ISequence<String>& strings )
{
	ISequence<String>* seq = new Sequence<String>();
	{
		const IIterator<String>* it = strings.elements();
		while ( it->hasNext() )
		{
			seq->insertLast( new String( it->next() ) );
		}
		delete it;
	}
	return seq;
}

/*
	%(%)

	something(int,char);
	          ^
	(	something
	)	int,char


	(%)
	
	something(int,char);

	something

*/
static ISequence<String>* tokenize( const char* _pattern, const char* content )
{
	ISequence<String>* tokens = new Sequence<String>();
	{
		bool parse = false;
		String pattern( _pattern );

		long len = pattern.getLength();
		for ( long i=0; i < len; i++ )
		{
			char ch;
			switch ( (ch = pattern.charAt(i)) )
			{
			case '%':
				parse = true;
				break;
			default:
				{
					StringBuffer sb;
				
					while ( ('\0' != *content) && (ch != *content) )
					{
						if ( parse ) sb.append( *content );
						content++;
					}
					
					while ( ch == *content ) content++;
				
					if ( parse )
					{
						tokens->insertLast( sb.asString() );
						parse = false;
					}
				}
			}
		}
	}
	return tokens;
}
~


