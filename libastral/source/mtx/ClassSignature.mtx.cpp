.	Signature classes

~!include/astral/ClassSignature.h~
#ifndef ASTRAL_CLASSSIGNATURE_H
#define ASTRAL_CLASSSIGNATURE_H

#include <openxds.base.h>
#include <openxds.adt.h>
#include <openxds.adt.std.h>

using openxds::adt::ISequence;
using openxds::adt::std::Sequence;
using openxds::base::String;

namespace astral {

class ClassSignature
{
private:
	String*				original;			//	java.util.Vector<String>
	String*              fqClass;			//  java.util.Vector
	String*               nspace;			//	java.util
	String*         genericClass;			//	Vector<String>
	String*                  cls;			//	Vector
	Sequence<String>* parameters;			//	{ String }
	bool                 generic;			//	true

public:
	 ClassSignature();
	 ClassSignature( const String&         original  );
	 ClassSignature( const ClassSignature& signature );
	~ClassSignature();

	virtual void initialise( const String& fqClass );

	virtual const String&               getOriginal() const { return *this->original;     }
	virtual const String&                getFQClass() const { return *this->fqClass;      }
	virtual const String&              getNamespace() const { return *this->nspace;       }
	virtual const String&           getGenericClass() const { return *this->genericClass; }
	virtual const String&              getClassName() const { return *this->cls;          }
	virtual const Sequence<String>&   getParameters() const { return *this->parameters;   }
	virtual       bool                    isGeneric() const { return this->generic;     }
	virtual       bool                   isComplete() const;
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
	this->original     = NULL;
	this->fqClass      = NULL;
	this->nspace       = NULL;
	this->genericClass = NULL;
	this->cls          = NULL;
	this->parameters   = new Sequence<String>();
	this->generic    = false;
}
~

~source/cplusplus/ClassSignature.cpp~
ClassSignature::ClassSignature( const String& fqClass )
{
	this->original     = NULL;
	this->fqClass      = NULL;
	this->nspace       = NULL;
	this->genericClass = NULL;
	this->cls          = NULL;
	this->parameters   = new Sequence<String>();
	this->generic    = false;

	this->initialise( fqClass );
}
~

~source/cplusplus/ClassSignature.cpp~
ClassSignature::ClassSignature( const ClassSignature& signature )
{
	this->original     = NULL;
	this->fqClass      = NULL;
	this->nspace       = NULL;
	this->genericClass = NULL;
	this->cls          = NULL;
	this->parameters   = new Sequence<String>();
	this->generic    = false;

	this->initialise( *signature.original );
}
~

~source/cplusplus/ClassSignature.cpp~
ClassSignature::~ClassSignature()
{
	delete this->original;
	delete this->fqClass;
	delete this->nspace;
	delete this->genericClass;
	delete this->cls;
	delete this->parameters;
}
~

~source/cplusplus/ClassSignature.cpp~
void
ClassSignature::initialise( const String& fqClass )
{
	//	original
	this->original = new String( fqClass );

	Sequence<String> bits;

	StringTokenizer st( fqClass );
	st.setDelimiter( '.' );
	while ( st.hasMoreTokens() )
	{
		bits.addLast( st.nextToken() );
	}

	//	genericClass
	this->genericClass = !bits.isEmpty() ? bits.removeLast() : new String();

	{
		StringTokenizer st( *this->genericClass );
		st.setDelimiter( '<' );

		//	cls
		this->cls = st.hasMoreTokens() ? st.nextToken() : new String();
		if ( st.hasMoreTokens() )
		{
			this->generic = true;
			String* parameters = st.nextToken();
			{
				StringTokenizer st2( *parameters );
				st2.setDelimiters( ",>" );
				while ( st2.hasMoreTokens() )
				{
					//	parameters
					this->parameters->addLast( st2.nextToken() );
				}
			}
			delete parameters;
		}
	}

	//	nspace
	{
		StringBuffer sb;
		IIterator<String>* it = bits.elements();
		while ( it->hasNext() )
		{
			sb.append( it->next() );
			sb.append( "." );
		}
		delete it;
		if ( 0 < sb.getLength() ) sb.removeLast();
		this->nspace = sb.asString();
	}
	
	//	fqClass
	this->fqClass = new FormattedString( "%s.%s", this->nspace->getChars(), this->cls->getChars() );
}

bool
ClassSignature::isComplete() const
{
	bool complete = false;
	if ( this->nspace->getLength() && this->cls->getLength() )
	{
		complete = true;// Character::isUppercase( this->cls->charAt(0) );
	}
	return complete;
}
~
