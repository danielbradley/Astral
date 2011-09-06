#include <astral/AstralFactory.h>
#include <astral/AstralExport.h>
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
	fprintf( stdout, "Usage: astral <Java file> ...\n" );
}

static void run( int argc, char** argv )
{
//	AstralExport astral;
//
//	for ( int i=1; i < argc; i++ )
//	{
//		astral.add( argv[i] );
//	}

	Sequence<Path> paths;
	for ( int i=1; i < argc; i++ )
	{
		paths.addLast( new Path( argv[i] ) );
	}

	AstralExport* astral = AstralFactory::createAstralExport( paths, ".java" );
	astral->exportHTMLTo( "html" );
}

int main( int argc, char** argv )
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