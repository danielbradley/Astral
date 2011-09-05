
#include "crossadaptive.astral/AbstractSyntax.h"
#include <openxds.util/IToken.h>
#include <openxds.base/String.h>

using namespace crossadaptive::astral;
using namespace openxds::base;
using namespace openxds::util;

const String* AbstractSyntax::Unknown     = new String( "UNKNOWN" );
const String* AbstractSyntax::File        = new String( "FILE" );
const String* AbstractSyntax::Package     = new String( "PACKAGE" );
const String* AbstractSyntax::Import      = new String( "IMPORT" );
const String* AbstractSyntax::Class       = new String( "CLASS" );
const String* AbstractSyntax::Interface   = new String( "INTERFACE" );
const String* AbstractSyntax::Method      = new String( "METHOD" );
const String* AbstractSyntax::Block       = new String( "BLOCK" );
const String* AbstractSyntax::Member      = new String( "MEMBER" );
const String* AbstractSyntax::Expression  = new String( "EXPRESSION" );
const String* AbstractSyntax::Parameters  = new String( "PARAMETERS" );
const String* AbstractSyntax::Statement   = new String( "STATEMENT" );
const String* AbstractSyntax::Declaration = new String( "DECLARATION" );
const String* AbstractSyntax::Comment     = new String( "COMMENT" );
const String* AbstractSyntax::BlankLine   = new String( "BLANKLINE" );
const String* AbstractSyntax::Token       = new String( "TOKEN" );
const String* AbstractSyntax::Symbol      = new String( "SYMBOL" );
const String* AbstractSyntax::Keyword     = new String( "KEYWORD" );
const String* AbstractSyntax::Type        = new String( "TYPE" );
const String* AbstractSyntax::MethodName  = new String( "METHODNAME" );
const String* AbstractSyntax::Variable    = new String( "VARIABLE" );
const String* AbstractSyntax::Name        = new String( "NAME" );
const String* AbstractSyntax::MethodCall  = new String( "METHODCALL" );
const String* AbstractSyntax::Operator    = new String( "OPERATOR" );
const String* AbstractSyntax::Selector    = new String( "SELECTOR" );
const String* AbstractSyntax::Value       = new String( "VALUE" );
const String* AbstractSyntax::Stop        = new String( "STOP" );
const String* AbstractSyntax::Tab         = new String( "TAB" );
const String* AbstractSyntax::Whitespace  = new String( "WHITESPACE" );
const String* AbstractSyntax::Newline     = new String( "NEWLINE" );
const String* AbstractSyntax::LineComment = new String( "LINECOMMENT" );

AbstractSyntax::AbstractSyntax(  AbstractSyntax::AbstractSyntaxType aType, IToken* aToken )
{
	this->type  = aType;
	this->token = aToken;
}

AbstractSyntax::~AbstractSyntax()
{
	delete this->token;
}

AbstractSyntax::AbstractSyntaxType
AbstractSyntax::getTokenType() const
{
	return this->type;
}

const String&
AbstractSyntax::getValue() const
{
	return *this->token->getValue();
}

const IToken&
AbstractSyntax::getToken() const
{
	return *this->token;
}

String*
AbstractSyntax::toString() const
{
	return this->getValue().asString();
}

const String&
AbstractSyntax::getTokenString() const
{
	switch ( this->type )
	{
	case FILE:
		return *AbstractSyntax::File;
	case PACKAGE:
		return *AbstractSyntax::Package;
	case IMPORT:
		return *AbstractSyntax::Import;
	case CLASS:
		return *AbstractSyntax::Class;
	case INTERFACE:
		return *AbstractSyntax::Interface;
	case METHOD:
		return *AbstractSyntax::Method;
	case BLOCK:
		return *AbstractSyntax::Block;
	case MEMBER:
		return *AbstractSyntax::Member;
	case EXPRESSION:
		return *AbstractSyntax::Expression;
	case PARAMETERS:
		return *AbstractSyntax::Parameters;
	case STATEMENT:
		return *AbstractSyntax::Statement;
	case DECLARATION:
		return *AbstractSyntax::Declaration;
	case COMMENT:
		return *AbstractSyntax::Comment;
	case BLANKLINE:
		return *AbstractSyntax::BlankLine;
	case TOKEN:
		return *AbstractSyntax::Token;
	case SYMBOL:
		return *AbstractSyntax::Symbol;
	case KEYWORD:
		return *AbstractSyntax::Keyword;
	case TYPE:
		return *AbstractSyntax::Type;
	case METHODNAME:
		return *AbstractSyntax::MethodName;
	case VARIABLE:
		return *AbstractSyntax::Variable;
	case NAME:
		return *AbstractSyntax::Name;
	case METHODCALL:
		return *AbstractSyntax::MethodCall;
	case OPERATOR:
		return *AbstractSyntax::Operator;
	case SELECTOR:
		return *AbstractSyntax::Selector;
	case VALUE:
		return *AbstractSyntax::Value;
	case STOP:
		return *AbstractSyntax::Stop;
	case TAB:
		return *AbstractSyntax::Tab;
	case WHITESPACE:
		return *AbstractSyntax::Whitespace;
	case NEWLINE:
		return *AbstractSyntax::Newline;
	case LINECOMMENT:
		return *AbstractSyntax::LineComment;
	default:
		return *AbstractSyntax::Unknown;
	}
}


