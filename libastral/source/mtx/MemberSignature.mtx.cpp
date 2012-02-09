,,		MemberSignature class

~!include/astral/MemberSignature.h~
#ifndef ASTRAL_MEMBERSIGNATURE_H
#define ASTRAL_MEMBERSIGNATURE_H

#include <openxds.base.h>

namespace astral {

class MemberSignature
{
private:
	openxds::base::String* original;				//	public fqClass.memberName|Type
	openxds::base::String* modifier;				//	public
	openxds::base::String* fqMember;				//	fqClass.memberName
	openxds::base::String* fqClass;					//	fqClass
	openxds::base::String* member;					//	memberName
	openxds::base::String* type;					//	Type
	
public:
	                                     MemberSignature();
	                                     MemberSignature( const char* mod, const char* fqClass, const char* member, const char* type );
	                                     MemberSignature( const openxds::base::String& aSignature );
	                                     MemberSignature( const MemberSignature& aMethodSignature );
	virtual                             ~MemberSignature();

	virtual const openxds::base::String&     getOriginal() const { return *this->original; }
	virtual const openxds::base::String&     getModifier() const { return *this->modifier; }
	virtual const openxds::base::String&     getFQMember() const { return *this->fqMember; }
	virtual const openxds::base::String&      getFQClass() const { return *this->fqClass;  }
	virtual const openxds::base::String&       getMember() const { return *this->member;   }
	virtual const openxds::base::String&         getType() const { return *this->type;     }
	
	virtual bool                                 isValid() const;

protected:
	virtual       void                        initialise( const openxds::base::String& aSignature );

};

};

#endif
~



~!source/cplusplus/MemberSignature.cpp~
#include "astral/MemberSignature.h"

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

~source/cplusplus/MemberSignature.cpp~
MemberSignature::MemberSignature()
{
	this->original = new String();
	this->modifier = new String();
	this->fqMember = new String();
	this->fqClass  = new String();
	this->member   = new String();
	this->type     = new String();
}

MemberSignature::MemberSignature( const char* mod, const char* fqClass, const char* member, const char* type )
{
	this->modifier = new String( mod     );
	this->fqClass  = new String( fqClass );
	this->member   = new String( member  );
	this->type     = new String( type );

	this->fqMember = new FormattedString( "%s.%s", fqClass, member );

	StringBuffer sb;
	if ( 0 < this->modifier->getLength() )
	{
		sb.append( mod );
		sb.append( "." );
	}
	if ( 0 < this->fqClass->getLength() )
	{
		sb.append( fqClass );
		sb.append( "." );
	}
	if ( 0 < this->member->getLength() )
	{
		sb.append( member );
		sb.append( "|" );
	}
	if ( 0 < this->type->getLength() )
	{
		sb.append( type );
		sb.append( "." );
	}
	sb.removeLast();
	this->original = sb.asString();
}

MemberSignature::MemberSignature( const String& aSignature )
{
	this->initialise( aSignature );
}




MemberSignature::MemberSignature( const MemberSignature& aMethodSignature )
{
	this->initialise( aMethodSignature.getOriginal() );
}




MemberSignature::~MemberSignature()
{
	delete this->original;
	delete this->modifier;
	delete this->fqMember;
	delete this->fqClass;
	delete this->member;
	delete this->type;
}




void
MemberSignature::initialise( const String& aSignature )
{
	this->original = new String( aSignature );

	StringTokenizer st( aSignature );
	String* tmp = NULL;
	{
		st.setDelimiter( '|' );
		tmp        = st.hasMoreTokens() ? st.nextToken() : new String();
		this->type = st.hasMoreTokens() ? st.nextToken() : new String();

		StringTokenizer st( *tmp );
		st.setDelimiter( ' ' );
		{
			this->modifier = st.hasMoreTokens() ? st.nextToken() : new String();
			if ( st.hasMoreTokens() )
			{
				this->fqMember = st.nextToken();
			}
			else
			{
				this->fqMember = this->modifier;
				this->modifier = new String();
			}
			
			StringBuffer sb;
			StringTokenizer st( *this->fqMember );
			st.setDelimiter( '.' );

			this->member = new String();
			while ( st.hasMoreTokens() )
			{
				sb.append( *this->member );
				delete this->member;
				this->member = st.nextToken();
			}
			
			this->fqClass = sb.asString();
		}
	}
	delete tmp;
}




bool
MemberSignature::isValid() const
{
	return (0 < this->type->getLength());
}
~