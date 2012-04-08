#include "astral.tokenizer/JavaTokenizer.h"

#include <openxds.base/String.h>

using namespace astral::tokenizer;
using namespace openxds::base;

bool
JavaTokenizer::isKeyword( const String& word ) const
{
	return (
			word.contentEquals( "package" )
		||	word.contentEquals( "class" )
		||	word.contentEquals( "interface" )
		||	word.contentEquals( "enum" )
		||	word.contentEquals( "extends" )
		||	word.contentEquals( "implements" )
		||	word.contentEquals( "this" )
		||	word.contentEquals( "super" )
		||	word.contentEquals( "new" )
		||	word.contentEquals( "if" )
		||	word.contentEquals( "for" )
		||	word.contentEquals( "else" )
		||	word.contentEquals( "while" )
		||	word.contentEquals( "do" )
		||	word.contentEquals( "try" )
		||	word.contentEquals( "catch" )
		||	word.contentEquals( "switch" )
		||	word.contentEquals( "case" )
		||	word.contentEquals( "throws" )
		||	word.contentEquals( "throw" )
		||	word.contentEquals( "break" )
		||	word.contentEquals( "continue" )
		||	word.contentEquals( "return" )
		||	word.contentEquals( "import" ) );
}

bool
JavaTokenizer::isModifier( const String& word ) const
{
	return (
			word.contentEquals( "abstract"     )
		||	word.contentEquals( "final"        )
		||	word.contentEquals( "native"       )
		||	word.contentEquals( "public"       )
		||	word.contentEquals( "private"      )
		||	word.contentEquals( "protected"    )
		||	word.contentEquals( "static"       )
		||	word.contentEquals( "stringfp"     )
		||	word.contentEquals( "synchronized" )
		||	word.contentEquals( "transient"    )
		||	word.contentEquals( "volatile"     ) );
}

bool
JavaTokenizer::isPrimitiveType( const String& word ) const
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
