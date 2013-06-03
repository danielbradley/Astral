#include "astral.tokenizer/IxTokenizer.h"

#include <openxds.base/String.h>
#include <openxds.io/IO.h>
#include <openxds.io/PrintWriter.h>

using namespace astral::tokenizer;
using namespace openxds::base;
using namespace openxds::io;

IxTokenizer::~IxTokenizer()
{}

bool
IxTokenizer::isKeyword( const String& word ) const
{
	return (
			word.contentEquals( "namespace" )
		||	word.contentEquals( "class" )
		||	word.contentEquals( "interface" )
		||	word.contentEquals( "enum" )
		||	word.contentEquals( "extends" )
		||	word.contentEquals( "implements" )
		||	word.contentEquals( "super" )
		||	word.contentEquals( "new" )
		||	word.contentEquals( "delete" )
		||	word.contentEquals( "if" )
		||	word.contentEquals( "for" )
		||	word.contentEquals( "else" )
		||	word.contentEquals( "while" )
		||	word.contentEquals( "do" )
		||	word.contentEquals( "switch" )
		||	word.contentEquals( "case" )
		||	word.contentEquals( "break" )
		||	word.contentEquals( "continue" )
		||	word.contentEquals( "return" )
		||	word.contentEquals( "import" )
		||	word.contentEquals( "author" )
		||	word.contentEquals( "copyright" )
		||	word.contentEquals( "fallthrough" )
		||	word.contentEquals( "foreach" )
		||	word.contentEquals( "licence" )
		||	word.contentEquals( "license" )
		||	word.contentEquals( "load" )
		||	word.contentEquals( "use" ) );

//		||	word.contentEquals( "try" )
//		||	word.contentEquals( "catch" )
//		||	word.contentEquals( "throws" )
//		||	word.contentEquals( "throw" )
}

bool
IxTokenizer::isModifier( const String& word ) const
{
	return (
			word.contentEquals( "abstract"     )
		||	word.contentEquals( "const"        )
		||	word.contentEquals( "final"        )
		||	word.contentEquals( "native"       )
		||	word.contentEquals( "public"       )
		||	word.contentEquals( "hidden"       )
		||	word.contentEquals( "private"      )
		||	word.contentEquals( "protected"    )
		||	word.contentEquals( "static"       ) );

//		||	word.contentEquals( "stringfp"     )
//		||	word.contentEquals( "synchronized" )
//		||	word.contentEquals( "transient"    )
//		||	word.contentEquals( "volatile"     ) );
}

bool
IxTokenizer::isPrimitiveType( const String& word ) const
{
	return (
			word.contentEquals( "byte" )
		||	word.contentEquals( "short" )
		||	word.contentEquals( "char" )
		||	word.contentEquals( "int" )
		||	word.contentEquals( "long" )
		||	word.contentEquals( "float" )
		||	word.contentEquals( "double" )
		||	word.contentEquals( "boolean" )
		||	word.contentEquals( "void" ) );
}

bool
IxTokenizer::IsPrimitiveType( const String& word )
{
	const char* val = word.getChars();
	IO::err().println( val );
	
	return (
			word.contentEquals( "byte" )
		||	word.contentEquals( "short" )
		||	word.contentEquals( "char" )
		||	word.contentEquals( "int" )
		||	word.contentEquals( "long" )
		||	word.contentEquals( "float" )
		||	word.contentEquals( "double" )
		||	word.contentEquals( "boolean" )
		||	word.contentEquals( "void" ) );
}

SourceToken*
IxTokenizer::parseAnnotation( openxds::base::String* word )
{
	char ch = word->charAt(0);

	SourceToken* token = SourceTokenizer::parseAnnotation( word );
	switch ( ch )
	{
	case '@':
		token->setType( SourceToken::MEMBER );
		break;
	
	case '#':
		token->setType( SourceToken::MEMBER );
		break;
	}
	return token;
}
