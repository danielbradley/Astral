#include <astral/AstralFactory.h>
#include <astral/CodeBase.h>
#include <astral/Export.h>
#include <openxds.base/Environment.h>
#include <openxds.base/String.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.io/Path.h>
#include <cstdio>

using namespace astral;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::io;

static void usage()
{
	fprintf( stdout, "Usage: astral <Search Dir> ...\n" );
}

static void run( int argc, const char** argv )
{
	Sequence<Path> paths;
	for ( int i=1; i < argc; i++ )
	{
		paths.addLast( new Path( argv[i] ) );
	}

	CodeBase* codebase = AstralFactory::createAstralExport( paths, ".java" );
	{
		Export::exportAdvancedHTMLTo( "html", *codebase );
	}
	delete codebase;
}

int main( int argc, const char** argv )
{
	Environment::init( argc, argv );

	switch ( argc )
	{
	case 1:
		usage();
		break;
	default:
		run( argc, argv );
	}
}