#include <astral/AstralFactory.h>
#include <astral/CodeBase.h>
#include <astral/Export.h>
#include <openxds.base/Environment.h>
#include <openxds.base/String.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.io/Path.h>
#include <cstdio>
#include <string>

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
	const char* ext_java   = ".java";
	const char* ext_cobalt = ".cm";
	const char* ext = ext_java;

	int i=1;
	
	if ( (argc > 1) && (0 == strcmp( argv[1], "--cobalt" )) )
	{
		ext = ext_cobalt;
		i++;
	}

	Sequence<Path> paths;
	for ( i; i < argc; i++ )
	{
		paths.addLast( new Path( argv[i] ) );
	}

	CodeBase* codebase = AstralFactory::createAstralExport( paths, ext );
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