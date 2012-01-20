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

protected:
	virtual       void                        initialise( const openxds::base::String& aSignature );

};

};

#endif

