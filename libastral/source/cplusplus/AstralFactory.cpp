#include "astral/CodeBase.h"
#include "astral/Export.h"
#include "astral/AstralFactory.h"

#include <openxds.io/Directory.h>
#include <openxds.io/Path.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.adt/IList.h>
#include <openxds.adt/IPIterator.h>
#include <openxds.base/String.h>
#include <openxds.io/IO.h>
#include <openxds.io/PrintWriter.h>

using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::io;

static void      processPaths( CodeBase& astral, const IList<Path>& pathList, const char* extension );

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

static void processPaths( CodeBase& astral, const IList<Path>& pathList, const char* extension )
{
	const IPIterator<Path>* it = pathList.positions();
	while ( it->hasNext() )
	{
		const IPosition<Path>* p = it->next();
		{
			const Path& path = p->getElement();
			AstralFactory::recursivelySearch( astral, path, "", "", extension );
		}
		delete p;
	}
	delete it;
}

//void
//AstralFactory::recursivelySearch( CodeBase& codebase, const Path& path, const char* extension )
//{
//	Directory current( path.getAbsolute().getChars() );
//	IList<String>* files = current.retrieveFileList();
//	{
//		IPIterator<String>* it = files->positions();
//		while ( it->hasNext() )
//		{
//			IPosition<String>* p = it->next();
//			{
//				const String& filename = p->getElement();
//				if ( ! filename.startsWith( "." ) )
//				{
//					Path* child = path.childPath( filename );
//					if ( child->getExtension().contentEquals( extension ) )
//					{
//						//IO::out().printf( "AstralFactory::recursivelySearch: %s\n", child->getAbsolute().getChars() );
//						codebase.addSourceFile( child->getAbsolute().getChars() );
//					}
//					else
//					{
//						Directory possible( child->getAbsolute().getChars() );
//						if ( possible.exists() )
//						{
//							recursivelySearch( codebase, *child, extension );
//						}
//					}
//					delete child;
//				}
//			}
//			delete p;
//		}
//		delete it;
//	}
//	delete files;
//
//	codebase.refreshImports();
//}

void
AstralFactory::recursivelySearch( CodeBase& codebase, const Path& working, const String& project, const String& sourcePath, const char* extension )
{
	Directory current( working.getAbsolute().getChars() );
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
					Path* child = working.childPath( filename );
					if ( child->getExtension().contentEquals( extension ) )
					{
						IO::out().printf( "AstralFactory::recursivelySearch: %s\n", child->getAbsolute().getChars() );
						
						//fprintf( stderr, "Adding: %s\n", sourcePath.getChars() );
						
						codebase.addSourceFile( child->getAbsolute().getChars(), project.getChars(), sourcePath.getChars() );
					}
					else
					{
						Directory possible( child->getAbsolute().getChars() );
						if ( possible.exists() )
						{
							recursivelySearch( codebase, *child, project, sourcePath, extension );
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

