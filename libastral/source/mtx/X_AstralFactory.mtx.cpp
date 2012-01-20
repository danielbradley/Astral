.	Appendices

..		Astral Factory

About

The *Astral Factory* class allows user code to instantiate an Astral object that has been populated from recursively searching from user supplied directories -- when instantiated directly the *Astral* object is populated by individual source files.

General usage is as follows:

~
#include <astral/AstralFactory.h>
#include <astral/CodeBase.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.io/Path.h>

...

using namespace openxds::adt::std;
using namespace openxds::io;
using namespace astral;

...

Sequence<Path> sequence;
sequence.insertLast( new Path( "<directory>" ) );
sequence.insertLast( new Path( "<directory>" ) );

Astral* astral = AstralFactory::createAstral( sequence, ".java" );

... usage code here ...

delete astral;
~


.	Implementation

..		Header - astral/AstralFactory.h

!
~!include/astral/AstralFactory.h~
#ifndef ASTRAL_ASTRALFACTORY_H
#define ASTRAL_ASTRALFACTORY_H
~
!

The *AstralFactory* class makes use of the following libraries:

~include/astral/AstralFactory.h~
#include "astral.h"
~

~include/astral/AstralFactory.h~
#include <openxds.adt.h>
#include <openxds.io.h>
#include <openxds/Object.h>
~

~include/astral/AstralFactory.h~
class AstralFactory : openxds::Object
{
public:
	static astral::CodeBase*
	createAstral(
		const openxds::adt::IList<openxds::io::Path>& pathList,
		const char*                                   extension
	);

	static astral::CodeBase*
	createAstralExport(
		const openxds::adt::IList<openxds::io::Path>& pathList,
		const char*                                   extension
	);

	static void
	recursivelySearch(
		           astral::CodeBase& astral,
		const openxds::io::Path&   path,
		const char*                extension );
};
~

!
~include/astral/AstralFactory.h~
#endif
~
!

..		Body - source/cplusplus/AstralFactory.cpp

!
~!source/cplusplus/AstralFactory.cpp~
#include "astral/CodeBase.h"
#include "astral/Export.h"
#include "astral/AstralFactory.h"
~
!

!
~source/cplusplus/AstralFactory.cpp~
#include <openxds.io/Directory.h>
#include <openxds.io/Path.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.adt/IList.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.base/String.h>
#include <openxds.io/IO.h>
#include <openxds.io/PrintWriter.h>
~
!

!
~source/cplusplus/AstralFactory.cpp~
using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::io;
~
!

!
~source/cplusplus/AstralFactory.cpp~
static void      processPaths( CodeBase& astral, const IList<Path>& pathList, const char* extension );
~
!




...			Class method: createAstral

The *createAstral* method recursively searches each path provided in the /pathList/ for files with a suffix matching the supplied /extension/.

~source/cplusplus/AstralFactory.cpp~
CodeBase*
AstralFactory::createAstral( const IList<Path>& pathList, const char* extension )
{
	CodeBase* codebase = new CodeBase();
	{
		processPaths( *codebase, pathList, extension );
		codebase->refreshImports();
	}
	return codebase;
}
~

...			Class method: createAstralExport

The *createAstralExport* method instantiates an *AstralExport* object and populates it by calling *processPaths*.

~source/cplusplus/AstralFactory.cpp~
CodeBase*
AstralFactory::createAstralExport( const IList<Path>& pathList, const char* extension )
{
	CodeBase* codebase = new CodeBase();
	{
		processPaths( *codebase, pathList, extension );
		codebase->refreshImports();
	}
	return codebase;
}
~

...			Static procedure: processPath


The *processPath* method causes each path in the /pathList/ to be recursively searched by calling *recursivelySearch*.

~source/cplusplus/AstralFactory.cpp~
static void processPaths( CodeBase& astral, const IList<Path>& pathList, const char* extension )
{
	const IPIterator<Path>* it = pathList.positions();
	while ( it->hasNext() )
	{
		const IPosition<Path>* p = it->next();
		{
			const Path& path = p->getElement();
			AstralFactory::recursivelySearch( astral, path, extension );
		}
		delete p;
	}
	delete it;
}
~

...			Static procedure: recursivelySearch

~source/cplusplus/AstralFactory.cpp~
void
AstralFactory::recursivelySearch( CodeBase& codebase, const Path& path, const char* extension )
{
	Directory current( path.getAbsolute().getChars() );
	IList<String>* files = current.retrieveFileList();
	{
		IPIterator<String>* it = files->positions();
		while ( it->hasNext() )
		{
			IPosition<String>* p = it->next();
			{
				const String& filename = p->getElement();
				if ( ! filename.startsWith( "." ) )
				{
					Path* child = path.childPath( filename );
					if ( child->getExtension().contentEquals( extension ) )
					{
						//IO::out().printf( "AstralFactory::recursivelySearch: %s\n", child->getAbsolute().getChars() );
						codebase.addSourceFile( child->getAbsolute().getChars() );
					}
					else
					{
						Directory possible( child->getAbsolute().getChars() );
						if ( possible.exists() )
						{
							recursivelySearch( codebase, *child, extension );
						}
					}
					delete child;
				}
			}
			delete p;
		}
		delete it;
	}
	delete files;

	codebase.refreshImports();
}
~










