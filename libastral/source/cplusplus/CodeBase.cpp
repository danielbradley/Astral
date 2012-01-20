#include "astral/CodeBase.h"
#include "astral/CompilationUnit.h"
#include "astral/MemberSignature.h"
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
	
	CompilationUnit* cu = new CompilationUnit( path );
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

const CompilationUnit&
CodeBase::getCompilationUnit( const MethodSignature& aMethodSignature ) const
{
	const char* method_signature = aMethodSignature.getMethodCall().getChars();
	const IEntry<const IEntry<CompilationUnit> >* e = this->symbolDB->getSymbols().find( method_signature );
	const CompilationUnit& cu = e->getValue().getValue();
	delete e;
	
	return cu;
}

MemberSignature*
CodeBase::completeMemberSignature( const char* fqClassType, const char* member ) const
{
	MemberSignature* member_signature = new MemberSignature( "", fqClassType, member, "" );

	try
	{
		const char* fq_member = member_signature->getFQMember().getChars();
		const CompilationUnit& cu = this->symbolDB->getCompilationUnitForSymbolPrefix( fq_member );
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
CodeBase::completeMethodSignature( const char* cls, const char* method, const char* parameters ) const
{
	MethodSignature* ret = new MethodSignature();
	{
		StringBuffer call;
		call.append( method );
		call.append( "(" );
		call.append( parameters );
		call.append( ")" );

		StringBuffer full;
		full.append( cls );
		full.append( '.' );
		full.append( call.getChars() );

		try
		{
			const CompilationUnit& cu = this->symbolDB->getCompilationUnitForSymbolPrefix( full.getChars() );

			String* rtype = cu.resolveMethodType( call.getChars() );
			{
				StringBuffer sb;
				sb.append( full.getChars() );
				sb.append( '|' );
				sb.append( *rtype );

				delete ret;
				ret = new MethodSignature( sb.getChars() );
			}
			delete rtype;
		}
		catch ( NoSuchElementException* ex )
		{
			delete ex;
			
			//	If the method is not available, recursively check superclasses.
			try
			{
				const CompilationUnit& cu = this->symbolDB->getCompilationUnitForSymbolPrefix( cls );
				String* super = cu.resolveFQTypeOfType( cu.getSuperclass().getChars() );
				if ( ! super->contentEquals( "" ) )
				{
					delete ret;
					ret = this->completeMethodSignature( super->getChars(), method, parameters );
				}
				delete super;
			}
			catch ( NoSuchElementException* ex )
			{
				delete ex;
			}
		}
	}
	return ret;
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

