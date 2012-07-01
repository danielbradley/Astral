#include "astral/CodeBase.h"
#include "astral/CompilationUnit.h"
#include "astral/Export.h"
#include "astral/MemberSignature.h"
#include "astral/Method.h"
#include "astral/MethodsList.h"
#include "astral/MethodSignature.h"
#include "astral/SymbolDB.h"

#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt/IEntry.h>
#include <openxds.adt/ISequence.h>
#include <openxds.adt/IDictionary.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.exceptions/NoSuchElementException.h>

using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::exceptions;

CodeBase::CodeBase()
{
	this->files    = new Dictionary<CompilationUnit>();
	this->symbolDB = new SymbolDB();
}

CodeBase::~CodeBase()
{
	delete this->files;
	delete this->symbolDB;
}

void
CodeBase::addBaseDirectory( const char* path )
{
    
}

void
CodeBase::addSourceFile( const char* path )
{
	
	CompilationUnit* cu = new CompilationUnit( *this, path );
	cu->initialise();
    
	IEntry<CompilationUnit>* entry = this->files->insert( path, cu );
	{
		this->symbolDB->registerCU( *entry );
	}
	delete entry;
}

void
CodeBase::refreshImports()
{
	IEIterator<CompilationUnit>* ie = this->files->entries();
	while ( ie->hasNext() )
	{
		IEntry<CompilationUnit>* e = ie->next();
		{
			CompilationUnit& compilation_unit = e->getValue();
			compilation_unit.resetImportedTypes( this->getSymbolDB() );
		}
		delete e;
	}
	delete ie;
}

ISequence<MethodSignature>*
CodeBase::saveMethod( const MethodSignature& aMethodSignature )
{
	ISequence<MethodSignature>* method_signatures = NULL;

	try
	{
		CompilationUnit& cu = this->getCompilationUnit( aMethodSignature.getClassSignature() );
		MethodsList&     ml = cu.getMethodsList();

		//if ( (can_save = ml.saveMethod( aMethodSignature )) )
		if ( (method_signatures = ml.synchroniseMethods( aMethodSignature )) )
		{
			cu.save();
			this->reregister( cu );
			//Export::toXML( *this );
		}
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}
	
	return method_signatures;
}

void
CodeBase::reregister( const CompilationUnit& aCu )
{
	try
	{
		IEntry<CompilationUnit>* e = this->files->find( aCu.getLocation().getChars() );
		{
			this->symbolDB->deregisterCU( *e );
			this->symbolDB->registerCU( *e );
		}
		delete e;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}
}

CompilationUnit&
CodeBase::getCompilationUnit( const ClassSignature& aClassSignature )
{
	const char* fq_class = aClassSignature.getFQClass().getChars();
	try
	{
		IEntry<const IEntry<CompilationUnit> >* e = this->symbolDB->getClasses().find( fq_class );
		const CompilationUnit& cu = e->getValue().getValue();
		delete e;
		return const_cast<CompilationUnit&>( cu );
	}
	catch ( NoSuchElementException* ex )
	{
		//	It is questionable whether this should really throw an except as
		//	it is often expected that the desired CompilationUnit will not
		//	be found, i.e. when it is a Java library class.
		throw;
	}
}

const CompilationUnit&
CodeBase::getCompilationUnit( const ClassSignature& aClassSignature ) const
{
	return const_cast<CodeBase*>( this )->getCompilationUnit( aClassSignature );
}

bool
CodeBase::hasCompilationUnit( const ClassSignature& aClassSignature ) const
{
	bool _has = false;
	try
	{
		this->getCompilationUnit( aClassSignature );
		_has = true;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}
	return _has;
}

MemberSignature*
CodeBase::completeMemberSignature( const char* fqClassType, const char* member ) const
{
	MemberSignature* member_signature = new MemberSignature( "", fqClassType, member, "" );

	try
	{
		const char* fq_member = member_signature->getFQMember().getChars();
		const CompilationUnit& cu = this->symbolDB->getCompilationUnitForSymbol( fq_member );
		String* type = cu.resolveMemberType( member );
		{
			delete member_signature;
			member_signature = new MemberSignature( "", fqClassType, member, type->getChars() );
		}
		delete type;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}

	return member_signature;
}

MethodSignature*
CodeBase::completeMethodSignature( const char* fqClass, const char* methodName, const char* parameters ) const
{
	MethodSignature* signature = null;//new MethodSignature();
	try
	{
		ClassSignature classSignature( fqClass );
		const CompilationUnit& cu = this->getCompilationUnit( classSignature );

		//delete signature;
		signature = cu.matchingMethodSignatureX( classSignature, methodName, parameters );
		if ( ! signature )
		{
			String* fqSuperclass = cu.resolveFQTypeOfType( cu.getSuperclass().getChars() );
			if ( ! fqSuperclass->contentEquals( "" ) )
			{
				signature = this->completeMethodSignature( fqSuperclass->getChars(), methodName, parameters );
			}
//			else
//			{
//				signature = new MethodSignature();
//			}
			delete fqSuperclass;
		}
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}
	return signature;

//		try
//		{
//			//String* rtype = cu.resolveMethodType( call.getChars() );
//			String* rtype = cu.resolveMethodType( methodName, parameters );
//			{
//				delete ret;
//				ret = new MethodSignature( classSignature, methodName, parameters, rtype->getChars() );
//			}
//			delete rtype;
//		}
//		catch ( NoSuchElementException* ex )
//		{
//			delete ex;
//			
//			//	If the method is not available, recursively check superclasses.
//			try
//			{
//				const CompilationUnit& cu = this->symbolDB->getCompilationUnitForSymbolPrefix( cls );
//				String* super = cu.resolveFQTypeOfType( cu.getSuperclass().getChars() );
//				if ( ! super->contentEquals( "" ) )
//				{
//					delete ret;
//					ret = this->completeMethodSignature( super->getChars(), method, parameters );
//				}
//				delete super;
//			}
//			catch ( NoSuchElementException* ex )
//			{
//				delete ex;
//			}
//		}
}

bool
CodeBase::containsType( const char* fqType ) const
{
	return this->symbolDB->containsType( fqType );
}

IIterator<CompilationUnit>*
CodeBase::getCompilationUnits()
{
	return this->files->values();
}

const IIterator<CompilationUnit>*
CodeBase::getCompilationUnits() const
{
	return this->files->values();
}

