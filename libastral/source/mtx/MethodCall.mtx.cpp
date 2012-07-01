.	MethodCall

The /MethodCall/ class is used to generate all of the possible permutations
of a method-call that might match up against a defined method.
For example, a method may be passed an /int/, but the corresponding method only accepts a /long/.
Or a subclass may be passed to a method that accepts a parent class.

Additionaly, the /Method Call/ class is also used to perform parameterisation conversion.
For example, a generic class may be parameterised with /E/ == /String/.
Methods defined in terms of /E/ will need to be matched with methodcalls that pass /String/.

The general usage is:

~
MethodCall* method_call = new MethodCall( *cu, "insertAt", "int,String" );
{
	ISequence<String>* variations = method_call->generateVariations();
	{
		...
	}
	delete variations;
}
delete method_call;
~

/generateVariations/ returns a sequence of possible methodcalls.

The /apply Parameterisation/ method may be called to perform parameterisation substitutions.

~
Map parameterisation;
parameterisation.put( "String", new String( "E" ) );

MethodCall* method_call = new MethodCall( *cu, "insertAt", "int,String" );
{
	method_call->applyParameterisation( parameterisation );
	ISequence<String>* variations = method_call->generateVariations();
	{
		...
	}
	delete variations;
}
delete method_call;
~






~!include/astral/MethodCall.h~
#ifndef ASTRAL_METHODCALL_H
#define ASTRAL_METHODCALL_H

#include <astral.h>
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
	const CompilationUnit&   cu;

	String*            methodName;
	String*            methodCall;
	ISequence<String>* parameters;
	ISequence<String>* variations;

public:
	 MethodCall( const CompilationUnit& cu, const char* methodName );
	 MethodCall( const CompilationUnit& cu, const char* methodName, const char* parameters );
	 MethodCall( const CompilationUnit& cu, const char* methodName, const ISequence<String>& parameters );
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
#include "astral/ClassSignature.h"
#include "astral/CodeBase.h"
#include "astral/CompilationUnit.h"
#include <astral.tokenizer/JavaTokenizer.h>

#include <openxds.adt/IMap.h>
#include <openxds.adt/ISequence.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.base/StringTokenizer.h>
#include <openxds.exceptions/NoSuchElementException.h>

using namespace astral;
using namespace astral::tokenizer;

using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::exceptions;

static ISequence<String>*     explode( const String&            string,  char delimiter );
static String*                 concat( const ISequence<String>& strings, char delimiter );
static ISequence<String>* copyStrings( const ISequence<String>& strings );
static ISequence<String>*    tokenize( const char* pattern, const char* content );

MethodCall::MethodCall( const CompilationUnit& cu, const char* methodCall ) : cu( cu )
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

MethodCall::MethodCall( const CompilationUnit& cu, const char* methodName, const char* parameters ) : cu( cu )
{
	this->methodName = new String( methodName );
	this->methodCall = new FormattedString( "%s(%s)", methodName, parameters );
	this->parameters = explode( parameters, ',' );
	this->variations = new Sequence<String>();
}

MethodCall::MethodCall( const CompilationUnit& cu, const char* _methodName, const ISequence<String>& parameters ) : cu( cu )
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
		if ( parameterisation.has( parameter.getChars() ) )
		{
			const String& substitute = parameterisation.get( parameter.getChars() );
			delete this->parameters->set( i, new String( substitute ) );
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

	MethodCall mc( cu, base );
	ISequence<String>* parameters = copyStrings( mc.getParameters() );
	{
		bool hit = false;
	
		if ( parameters->has( i ) )
		{
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

				if ( !hit && !target_parameter.contentEquals( "E" ) )
				{
					JavaTokenizer java_tokenizer;
					if ( ! java_tokenizer.isPrimitiveType( target_parameter ) )
					{
						String* fq_type = cu.resolveFQTypeOfType( target_parameter.getChars() );
						{
							ClassSignature cls( *fq_type );
							if ( cu.getCodeBase().hasCompilationUnit( cls ) )
							{
								const CompilationUnit& comp = cu.getCodeBase().getCompilationUnit( cls );
								const String& superclass = comp.getSuperclass();
								
								if ( ! superclass.contentEquals( "" ) )
								{
									delete parameters->set( i, new String( superclass ) );
									hit = true;
								}
							}
						}
						delete fq_type;
					}
				}

				if ( hit )
				{
					MethodCall new_variant( cu, mc.getMethodName().getChars(), *parameters );
					this->variations->insertLast( new String( new_variant.getMethodCall() ) );
				}
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


