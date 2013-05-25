#include <astral.tokenizer/JavaTokenizer.h>
#include <astral.tokenizer/SourceToken.h>
#include <openxds.base/Environment.h>
#include <openxds.base/String.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.adt/IIterator.h>
#include <openxds.io/Path.h>
#include <cstdio>

using namespace astral::tokenizer;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::io;

static void            usage();
static void            run( int argc, const char** argv );
static Sequence<Path>* convertArgumentsIntoFilePaths( int argc, const char** argv );
static void            tokenizeEachPath( const Sequence<Path>& paths );
static void            tokenizePath( const Path& path );
static void            printToken( const SourceToken& token );

static void printToken( const SourceToken& token );

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
	return 0;
}

static void usage()
{
	fprintf( stdout, "Usage: tokenize <Source File> ...\n" );
}

static void run( int argc, const char** argv )
{
	Sequence<Path>* paths = convertArgumentsIntoFilePaths( argc, argv );
	{
		tokenizeEachPath( *paths );
	}
	delete paths;
}

Sequence<Path>* convertArgumentsIntoFilePaths( int argc, const char** argv )
{
	Sequence<Path>* paths = new Sequence<Path>();
	for ( int i=1; i < argc; i++ )
	{
		paths->addLast( new Path( argv[i] ) );
	}
	return paths;
}
	
void tokenizeEachPath( const Sequence<Path>& paths )
{
	const IIterator<Path>* it = paths.elements();
	while ( it->hasNext() )
	{
		tokenizePath( it->next() );
	}
	delete it;
}

void tokenizePath( const Path& path )
{
	fprintf( stdout, "<style>\n" );
	fprintf( stdout, "*                   { font-size:12px; font-family:'Lucida Console', 'Monaco', monospace; }\n" );
	fprintf( stdout, ".KEYWORD, .MODIFIER { color:#800080; }\n" );
	fprintf( stdout, ".MEMBER             { color:#00AA00; }\n" );
	fprintf( stdout, ".NAME               { color:#555555; }\n" );
	fprintf( stdout, ".TYPE               { color:#800080; }\n" );
	fprintf( stdout, "</style>\n" );

	fprintf( stdout, "<pre id='%s'>", path.getAbsolute().getChars() );

	SourceTokenizer* tokenizer = SourceTokenizer::createFor( path.getAbsolute() );
	while ( tokenizer->hasMoreTokens() )
	{
		SourceToken* token = tokenizer->nextToken();
		{
			printToken( *token );
		}
		delete token;
	}
	delete tokenizer;

	fprintf( stdout, "</pre" );
}

void printToken( const SourceToken& token )
{
	const char* type  = token.getTokenTypeString().getChars();
	const char* value = token.getValue().getChars();

	switch ( token.getTokenType() )
	{
	case SourceToken::SPACE:
	case SourceToken::TAB:
	case SourceToken::BLANKLINE:
	case SourceToken::NEWLINE:
		fprintf( stdout, "%s", value );
		break;
	default:
		//fprintf( stdout, "<%s value='%s' />", type, value );
		fprintf( stdout, "<span class='%s'>%s</span>", type, value );
	}
}
