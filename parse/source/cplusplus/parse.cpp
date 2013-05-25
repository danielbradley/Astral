#include <astral.ast/AST.h>
#include <astral.tours/HTMLPrintTour.h>
#include <openxds.base/Environment.h>
#include <openxds.base/String.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.io/IO.h>
#include <openxds.io/Path.h>
#include <cstdio>

using namespace astral::ast;
using namespace astral::tours;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::io;

static void usage()
{
	fprintf( stdout, "Usage: parse <File>\n" );
}

static void run( int argc, const char** argv )
{
	const char* filepath = argv[1];

	AST ast; ast.parseFile( filepath );

	IO::out().println( "<pre>" );
	{
		HTMLPrintTour tour( ast.getTree(), IO::out() );
		tour.doGeneralTour();
	}
	IO::out().println( "</pre>" );
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