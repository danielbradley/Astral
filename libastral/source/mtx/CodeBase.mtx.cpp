..		Class: CodeBase

The /CodeBase/ class is used to federate /CompilationUnit/ objects together.

...			Life-cycle

The typical life-cycle of a /CodeBase/ object involves the following:

1.	Instantiation and population with source files:

~
CodeBase codebase;
codebase.addSourceFile( "some/source/File.java" );
codebase.addBaseDirectory( "a/directory" );
~

2.	Refresh of imports after change addition of files:

~
codebase.refreshImports();
~

3.	Retrieve and interate through /CompilationUnit/ objects.

~
IIterator<CompilationUnit&>* it = codebase.getCompilationUnits();
while ( it->hasNext() )
{
	CompilationUnit& cu = it->next();
	...
}
delete it;
~

!
~!include/astral/CodeBase.h~
#ifndef ASTRAL_CODEBASE_H
#define ASTRAL_CODEBASE_H

#include "astral.h"
#include <openxds.base.h>
#include <openxds.adt.h>
#include <openxds/Object.h>

namespace astral {
~
!




...			Definition

The CodeBase class inherits directly from the openxds::Object class.

~include/astral/CodeBase.h~
class CodeBase : openxds::Object {
~




...			Interface

The /CodeBase/ class has only two instance members.
The /files/ dictionary maps each /CompilationUnit/ unit from its filename.
The /files/ dictionary /owns/ all /CompilationUnit/ objects.
The /symbolDB/ is used to federate symbols together.

~include/astral/CodeBase.h~
private:
	openxds::adt::IDictionary<CompilationUnit>* files; // filepath -> compilation unit
	SymbolDB* symbolDB;
~

!	
~include/astral/CodeBase.h~
public:
	CodeBase();
	virtual ~CodeBase();
	
	virtual       void                                          addBaseDirectory( const char* path );
	virtual       void                                             addSourceFile( const char* path );
	virtual       void                                            refreshImports();
	
//	virtual const CompilationUnit&                            getCompilationUnit( const openxds::base::String& methodSignature ) const;
	virtual const CompilationUnit&                            getCompilationUnit( const MethodSignature& aMethodSignature ) const;

	virtual       MemberSignature*                       completeMemberSignature( const char* cls, const char* member ) const;
	virtual       MethodSignature*                       completeMethodSignature( const char* cls, const char* method, const char* parameters ) const;
	virtual       bool                                              containsType( const char* fqType ) const;

	virtual       openxds::adt::IIterator<CompilationUnit>*  getCompilationUnits();
	virtual const openxds::adt::IIterator<CompilationUnit>*  getCompilationUnits() const;

	virtual       openxds::adt::IDictionary<CompilationUnit>&           getFiles()       { return *this->files; }
	virtual const openxds::adt::IDictionary<CompilationUnit>&           getFiles() const { return *this->files; }

	virtual       SymbolDB&                                          getSymbolDB()       { return *this->symbolDB; }
	virtual const SymbolDB&                                          getSymbolDB() const { return *this->symbolDB; }
};
    
};

#endif
~
!

....			Method Summary

~html~
<style>
TABLE    { width:850px; }
TABLE TH { font-family:arial; font-size:13px; text-align:left; }
TABLE TD { font-family:menlo; font-size:10px; }
TABLE A  { font-family:menlo; font-size:10px; }

</style>

<table>
<thead>
<tr>
<th>Returns</th><th>Method-name</th><th colspan='3'>Parameters</th>
</tr>
</thead>
<tbody>
<tr><td><b>                             </b></td> <td><a href='#CodeBase'           >CodeBase           </a></td>                                                                                                                                </tr>
<tr><td><b>void                         </b></td> <td><a href='#addBaseDirectory'   >addBaseDirectory   </a></td><td><b>const char*</b> path                     </td>                                                                           </tr>
<tr><td><b>void                         </b></td> <td><a href='#addSourceFile'      >addSourceFile      </a></td><td><b>const char*</b> path                     </td>                                                                           </tr>
<tr><td><b>void                         </b></td> <td><a href='#refreshImports'     >refreshImports     </a></td>                                                                                                                                </tr>

<tr><td><b>CompilationUnit&             </b></td> <td><a href='#getCompilationUnit'      >getCompilationUnit      </a></td><td><b>MethodSignature&</b> methodSignature </td>                                                                           </tr>
<tr><td><b>MemberSignature*             </b></td> <td><a href='#completeMemberSignature' >completeMemberSignature </a></td><td><b>const char*</b> fqClass              </td><td><b>const char*</b> member </td>                                        </tr>
<tr><td><b>MethodSignature*             </b></td> <td><a href='#completeMethodSignature' >completeMethodSignature </a></td><td><b>const char*</b> fqClass              </td><td><b>const char*</b> method </td><td><b>const char*</b> parameters </td> </tr>
<tr><td><b>bool                         </b></td> <td><a href='#containsType'            >containsType            </a></td><td><b>const char*</b> fqType               </td>                                                                           </tr>

<tr><td><b>IIterator(CompilationUnit)*  </b></td> <td><a href='#getCompilationUnits'     >getCompilationUnits     </a></td> </tr>

<tr><td><b>Dictionary(CompilationUnit)& </b></td> <td><a href='#getFiles'                >getFiles                </a></td> </tr>
<tr><td><b>SymbolDB&                    </b></td> <td><a href='#getSymbolDB'             >getSymbolDB             </a></td> </tr>


</tbody>
</table>
~






!
~!source/cplusplus/CodeBase.cpp~
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
~
!




...			Constructor

~html~
<a name='CodeBase'></a><hr>
~

*CodeBase*

~
CodeBase::CodeBase();
~

Returns
|
A new instance of /CodeBase/.
|

Implementation

~source/cplusplus/CodeBase.cpp~
CodeBase::CodeBase()
{
	this->files    = new Dictionary<CompilationUnit>();
	this->symbolDB = new SymbolDB();
}
~





...			Destructor

~html~
<a name='~CodeBase'></a><hr>
~

*~CodeBase*

~
CodeBase::~CodeBase();
~

Implemenation

~source/cplusplus/CodeBase.cpp~
CodeBase::~CodeBase()
{
	delete this->files;
	delete this->symbolDB;
}
~




...			Methods

~html~
<a name='addBaseDirectory'></a><hr>
~

*addBaseDirectory*

~
CodeBase::addBaseDirectory( const char* path );
~

~source/cplusplus/CodeBase.cpp~
void
CodeBase::addBaseDirectory( const char* path )
{
    
}
~



~html~
<a name='addSourceFile'></a><hr>
~

*addSourceFile*

~
CodeBase::addSourceFile( const char* path );
~

Parameters
|
/path/, of the source file to be added.
|

Implementation

~source/cplusplus/CodeBase.cpp~
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
~




~html~
<a name='refreshImports'></a><hr>
~

*refreshImports*

~
CodeBase::refreshImports();
~

Causes
|
The imports of each CompilationUnit to be refreshed.
|

~source/cplusplus/CodeBase.cpp~
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
~




~html~
<a name='getCompilationUnit(MethodSignature)'></a><hr>
~
*getCompilationUnit*

~
const CompilationUnit& CodeBase::getCompilationUnit( const MethodSignature& methodSignature );
~

Parameters
|
/methodSignature/, identifying a method of a class.
|

Returns
|
a reference to the /CompilationUnit/ containing the specified method.
|

Implementation

~source/cplusplus/CodeBase.cpp~
const CompilationUnit&
CodeBase::getCompilationUnit( const MethodSignature& aMethodSignature ) const
{
	const char* method_signature = aMethodSignature.getMethodCall().getChars();
	const IEntry<const IEntry<CompilationUnit> >* e = this->symbolDB->getSymbols().find( method_signature );
	const CompilationUnit& cu = e->getValue().getValue();
	delete e;
	
	return cu;
}
~




~html~
<a name='completeMemberSignature'></a><hr>
~
*completeMemberSignature*

~
MemberSignature* CodeBase::completeMemberSignature( const char* fqType, const char* member );
~

Parameters
|
/fqType/, a fully qualified type that identifies a class;

/member/, the name of a member of the class identified by /fqType/.
|

Returns
|
A new instance of /MemberSignature/ corresponding to the method specified.
|

Implementation

~source/cplusplus/CodeBase.cpp~
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
~




~html~
<a name='completeMethodSignature'></a><hr>
~
*completeMethodSignature*

~
MethodSignature* CodeBase::completeMethodSignature( const char* fqType, const char* member, const char* parameters );
~

Parameters
|
/fqType/, a fully qualified type that identifies a class;

/member/, the name of a member of the class identified by /fqType/;

/parameters/, the non-qualified types of the method.
|

Returns
|
A new instance of /MethodSignature/, which now includes the return type as well.
|

Implementation

~source/cplusplus/CodeBase.cpp~
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
~




~html~
<a name='containsType'></a><hr>
~
*containsType*

~
bool CodeBase::containsType( const char* fqType ) const;
~

Parameters
|
/fqType/, a fully qualified type.
|

Returns
|
/true/, if /fqType/ exists within the codebase, /false/ otherwise.
|

Implementation

~source/cplusplus/CodeBase.cpp~
bool
CodeBase::containsType( const char* fqType ) const
{
	return this->symbolDB->containsType( fqType );
}
~





~html~
<a name='getCompilationUnits'></a><hr>
~
*getCompilationUnits*

~
IIterator<CompilationUnit>* CodeBase::getCompilationUnits();
~

Returns
|
A new iterator instance that iterates through the /CompilationUnit/ objects of this /CodeBase/.
|

Implementation

~source/cplusplus/CodeBase.cpp~
IIterator<CompilationUnit>*
CodeBase::getCompilationUnits()
{
	return this->files->values();
}
~

~source/cplusplus/CodeBase.cpp~
const IIterator<CompilationUnit>*
CodeBase::getCompilationUnits() const
{
	return this->files->values();
}
~


