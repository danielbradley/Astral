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

