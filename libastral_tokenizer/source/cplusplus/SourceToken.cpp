#include "astral.tokenizer/SourceToken.h"
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>

using namespace astral::tokenizer;
using namespace openxds::base;
using namespace openxds::util;

const String* SourceToken::Unknown         = new String( "UNKNOWN" );

const String* SourceToken::Word            = new String( "WORD" );
const String* SourceToken::File            = new String( "FILE" );
const String* SourceToken::Package         = new String( "PACKAGE" );
const String* SourceToken::Import          = new String( "IMPORT" );
const String* SourceToken::Include         = new String( "INCLUDE" );
const String* SourceToken::Class           = new String( "CLASS" );
const String* SourceToken::ClassName       = new String( "CLASSNAME" );
const String* SourceToken::Interface       = new String( "INTERFACE" );
const String* SourceToken::Annotation      = new String( "ANNOTATION" );
const String* SourceToken::IMethod         = new String( "IMETHOD" );
const String* SourceToken::Method          = new String( "METHOD" );
const String* SourceToken::Block           = new String( "BLOCK" );
const String* SourceToken::StartBlock      = new String( "STARTBLOCK" );
const String* SourceToken::EndBlock        = new String( "ENDBLOCK" );
const String* SourceToken::Member          = new String( "MEMBER" );
const String* SourceToken::MemberName      = new String( "MEMBERNAME" );
const String* SourceToken::Expression      = new String( "EXPRESSION" );
const String* SourceToken::StartExpression = new String( "STARTEXPRESSION" );
const String* SourceToken::EndExpression   = new String( "ENDEXPRESSION" );
const String* SourceToken::Parameters      = new String( "PARAMETERS" );
const String* SourceToken::Parameter       = new String( "PARAMETER" );
const String* SourceToken::Arguments       = new String( "ARGUMENTS" );
const String* SourceToken::Argument        = new String( "ARGUMENT" );
const String* SourceToken::Statement       = new String( "STATEMENT" );
const String* SourceToken::Declaration     = new String( "DECLARATION" );
const String* SourceToken::Comment         = new String( "COMMENT" );
const String* SourceToken::Javadoc         = new String( "JAVADOC" );
const String* SourceToken::BlankLine       = new String( "BLANKLINE" );
const String* SourceToken::Token           = new String( "TOKEN" );
const String* SourceToken::Symbol          = new String( "SYMBOL" );
const String* SourceToken::Keyword         = new String( "KEYWORD" );
const String* SourceToken::Modifier        = new String( "MODIFIER" );
const String* SourceToken::Type            = new String( "TYPE" );
const String* SourceToken::MethodName      = new String( "METHODNAME" );
const String* SourceToken::Variable        = new String( "VARIABLE" );
const String* SourceToken::Name            = new String( "NAME" );
const String* SourceToken::MethodCall      = new String( "METHODCALL" );
const String* SourceToken::Constructor     = new String( "CONSTRUCTOR" );
const String* SourceToken::Operator        = new String( "OPERATOR" );
const String* SourceToken::AssignmentOp    = new String( "ASSIGNMENTOP" );
const String* SourceToken::PrefixOp        = new String( "PREFIXOP" );
const String* SourceToken::InfixOp         = new String( "INFIXOP" );
const String* SourceToken::PostfixOp       = new String( "POSTFIXOP" );
const String* SourceToken::PreInfixOp      = new String( "PREINFIXOP" );
const String* SourceToken::PrePostfixOp    = new String( "PREPOSTFIXOP" );
const String* SourceToken::Selector        = new String( "SELECTOR" );
const String* SourceToken::Value           = new String( "VALUE" );
const String* SourceToken::Float           = new String( "FLOAT" );
const String* SourceToken::Integer         = new String( "INTEGER" );
const String* SourceToken::Number          = new String( "NUMBER" );
const String* SourceToken::DoubleQuote     = new String( "DOUBLEQUOTE" );
const String* SourceToken::Quote           = new String( "QUOTE" );
const String* SourceToken::Stop            = new String( "STOP" );
const String* SourceToken::Tab             = new String( "TAB" );
const String* SourceToken::Space           = new String( "SPACE" );
const String* SourceToken::Whitespace      = new String( "WHITESPACE" );
const String* SourceToken::Newline         = new String( "NEWLINE" );
const String* SourceToken::LineComment     = new String( "LINECOMMENT" );
const String* SourceToken::Escaped         = new String( "ESCAPED" );
const String* SourceToken::Other           = new String( "OTHER" );

SourceToken::SourceToken( TokenType aType, openxds::base::String* aValue )
{
	this->type  = aType;
	this->value = aValue->asString();
	delete aValue;
}

SourceToken::SourceToken( const SourceToken& aSourceToken )
{
	this->type  = aSourceToken.type;
	this->value = aSourceToken.value->asString();
}

SourceToken::~SourceToken()
{
	delete this->value; this->value = null;
}

void
SourceToken::setType( SourceToken::TokenType aType )
{
	this->type = aType;
}

void
SourceToken::setValue( String* aValue )
{
	delete this->value;
	this->value = aValue;
}

const String&
SourceToken::getValue() const
{
	return *this->value;
}

SourceToken::TokenType
SourceToken::getTokenType() const
{
	return this->type;
}

String*
SourceToken::getEscapedValue() const
{
	StringBuffer sb;

	long max = this->value->getLength();
	for ( long i=0; i < max; i++ )
	{
		long ch = this->value->charAt((int)i);
		
		switch ( ch )
		{
		case '\n':
			sb.append( '\\' );
			sb.append( 'n' );
			break;
		case '\t':
			sb.append( '\\' );
			sb.append( 't' );
			break;
		default:
			sb.append( ch );
		}
	}
	
	return sb.asString();
}

const String&
SourceToken::getTokenTypeString() const
{
	switch ( this->type )
	{
	case WORD:            return *SourceToken::Word;
	case FILE:            return *SourceToken::File;
	case PACKAGE:         return *SourceToken::Package;
	case IMPORT:          return *SourceToken::Import;
	case INCLUDE:         return *SourceToken::Include;
	case CLASS:           return *SourceToken::Class;
	case CLASSNAME:       return *SourceToken::ClassName;
	case INTERFACE:       return *SourceToken::Interface;
	case ANNOTATION:      return *SourceToken::Annotation;
	case IMETHOD:         return *SourceToken::IMethod;
	case METHOD:          return *SourceToken::Method;
	case BLOCK:           return *SourceToken::Block;
	case STARTBLOCK:      return *SourceToken::StartBlock;
	case ENDBLOCK:        return *SourceToken::EndBlock;
	case MEMBER:          return *SourceToken::Member;
	case MEMBERNAME:      return *SourceToken::MemberName;
	case EXPRESSION:      return *SourceToken::Expression;
	case STARTEXPRESSION: return *SourceToken::StartExpression;
	case ENDEXPRESSION:   return *SourceToken::EndExpression;
	case PARAMETERS:      return *SourceToken::Parameters;
	case PARAMETER:       return *SourceToken::Parameter;
	case ARGUMENTS:       return *SourceToken::Arguments;
	case ARGUMENT:        return *SourceToken::Argument;
	case STATEMENT:       return *SourceToken::Statement;
	case DECLARATION:     return *SourceToken::Declaration;
	case COMMENT:         return *SourceToken::Comment;
	case JAVADOC:         return *SourceToken::Javadoc;
	case BLANKLINE:       return *SourceToken::BlankLine;
	case TOKEN:           return *SourceToken::Token;
	case SYMBOL:          return *SourceToken::Symbol;
	case KEYWORD:         return *SourceToken::Keyword;
	case MODIFIER:        return *SourceToken::Modifier;
	case TYPE:            return *SourceToken::Type;
	case METHODNAME:      return *SourceToken::MethodName;
	case VARIABLE:        return *SourceToken::Variable;
	case NAME:            return *SourceToken::Name;
	case METHODCALL:      return *SourceToken::MethodCall;
	case CONSTRUCTOR:     return *SourceToken::Constructor;
	case OPERATOR:        return *SourceToken::Operator;
	case ASSIGNMENTOP:    return *SourceToken::AssignmentOp;
	case PREFIXOP:        return *SourceToken::PrefixOp;
	case INFIXOP:         return *SourceToken::InfixOp;
	case POSTFIXOP:       return *SourceToken::PostfixOp;
	case PREPOSTFIXOP:    return *SourceToken::PrePostfixOp;
	case PREINFIXOP:      return *SourceToken::PreInfixOp;
	case SELECTOR:        return *SourceToken::Selector;
	case VALUE:           return *SourceToken::Value;
	case FLOAT:           return *SourceToken::Float;
	case INTEGER:         return *SourceToken::Integer;
	case NUMBER:          return *SourceToken::Number;
	case DOUBLEQUOTE:     return *SourceToken::DoubleQuote;
	case QUOTE:           return *SourceToken::Quote;
	case STOP:            return *SourceToken::Stop;
	case TAB:             return *SourceToken::Tab;
	case SPACE:           return *SourceToken::Space;
	case WHITESPACE:      return *SourceToken::Whitespace;
	case NEWLINE:         return *SourceToken::Newline;
	case LINECOMMENT:     return *SourceToken::LineComment;
	case ESCAPED:         return *SourceToken::Escaped;
	case OTHER:           return *SourceToken::Other;
	default:              return *SourceToken::Unknown;
	}
}























