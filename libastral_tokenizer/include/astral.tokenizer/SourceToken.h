#ifndef ASTRAL_TOKENIZER_SOURCETOKEN_H
#define ASTRAL_TOKENIZER_SOURCETOKEN_H

#include <openxds.base.h>
#include <openxds.util/IToken.h>
#include <openxds.util.h>
#include <openxds/Object.h>

namespace astral {
	namespace tokenizer {
	
class SourceToken : public openxds::Object
{
public:
	enum TokenType
	{
		_WORD,
		_FILE,
		_PACKAGE,
		_IMPORT,
		_INCLUDE,
		_CLASS,
		_CLASSNAME,
		_INTERFACE,
		_ENUM,
		_ANNOTATION,
		_IMETHOD,
		_METHOD,
		_BLOCK,
		_STARTBLOCK,
		_ENDBLOCK,
		_MEMBER,
		_MEMBERNAME,
		_EXPRESSION,
		_STARTEXPRESSION,
		_ENDEXPRESSION,
		_CLAUSE,
		_PARAMETERS,
		_PARAMETER,
		_ARGUMENTS,
		_ARGUMENT,
		_STATEMENT,
		_DECLARATION,
		_COMMENT,
		_JAVADOC,
		_BLANKLINE,
		_TOKEN,
		_SYMBOL,
		_KEYWORD,
		_MODIFIER,
		_TYPE,
		_METHODNAME,
		_VARIABLE,
		_NAME,
		_METHODCALL,
		_CONSTRUCTOR,
		_OPERATOR,
		_ASSIGNMENTOP,
		_PREFIXOP,
		_INFIXOP,
		_POSTFIXOP,
		_PREINFIXOP,
		_PREPOSTFIXOP,
		_SELECTOR,
		_VALUE,
		_FLOAT,
		_INTEGER,
		_NUMBER,
		_DOUBLEQUOTE,
		_QUOTE,
		_STOP,
		_TAB,
		_SPACE,
		_WHITESPACE,
		_NEWLINE,
		_LINECOMMENT,
		_ESCAPED,
		_OTHER
	};

private:
	TokenType                        type;
	openxds::base::String*          value;
	long                   nrOfCharacters;
	long                        nrOfLines;
	long                           offset;

public:
	         SourceToken( TokenType aType, openxds::base::String* aValue );
	         SourceToken( const SourceToken& aSourceToken );
	virtual	~SourceToken();
	
	virtual void setType( SourceToken::TokenType aType );
	virtual void setValue( openxds::base::String* aValue );
	virtual void setNrOfCharacters( long nr );
	virtual void setNrOfLines( long nr );
	virtual void setOffset( long nr );

	virtual const openxds::base::String&           getValue() const;
	virtual const openxds::base::String& getTokenTypeString() const;
	virtual       SourceToken::TokenType       getTokenType() const;
	virtual       openxds::base::String*    getEscapedValue() const;
	virtual long                          getNrOfCharacters() const;
	virtual long                               getNrOfLines() const;
	virtual long                                  getOffset() const;
	
	static const TokenType WORD            = _WORD;
	static const TokenType FILE            = _FILE;
	static const TokenType PACKAGE         = _PACKAGE;
	static const TokenType IMPORT          = _IMPORT;
	static const TokenType INCLUDE         = _INCLUDE;
	static const TokenType CLASS           = _CLASS;
	static const TokenType CLASSNAME       = _CLASSNAME;
	static const TokenType INTERFACE       = _INTERFACE;
	static const TokenType ENUM            = _ENUM;
	static const TokenType ANNOTATION      = _ANNOTATION;
	static const TokenType IMETHOD         = _IMETHOD;
	static const TokenType METHOD          = _METHOD;
	static const TokenType BLOCK           = _BLOCK;
	static const TokenType STARTBLOCK      = _STARTBLOCK;
	static const TokenType ENDBLOCK        = _ENDBLOCK;
	static const TokenType MEMBER          = _MEMBER;
	static const TokenType MEMBERNAME      = _MEMBERNAME;
	static const TokenType EXPRESSION      = _EXPRESSION;
	static const TokenType STARTEXPRESSION = _STARTEXPRESSION;
	static const TokenType ENDEXPRESSION   = _ENDEXPRESSION;
	static const TokenType CLAUSE          = _CLAUSE;
	static const TokenType PARAMETERS      = _PARAMETERS;
	static const TokenType PARAMETER       = _PARAMETER;
	static const TokenType ARGUMENTS       = _ARGUMENTS;
	static const TokenType ARGUMENT        = _ARGUMENT;
	static const TokenType STATEMENT       = _STATEMENT;
	static const TokenType DECLARATION     = _DECLARATION;
	static const TokenType COMMENT         = _COMMENT;
	static const TokenType JAVADOC         = _JAVADOC;
	static const TokenType BLANKLINE       = _BLANKLINE;
	static const TokenType TOKEN           = _TOKEN;
	static const TokenType SYMBOL          = _SYMBOL;
	static const TokenType KEYWORD         = _KEYWORD;
	static const TokenType MODIFIER        = _MODIFIER;
	static const TokenType TYPE            = _TYPE;
	static const TokenType METHODNAME      = _METHODNAME;
	static const TokenType VARIABLE        = _VARIABLE;
	static const TokenType NAME            = _NAME;
	static const TokenType METHODCALL      = _METHODCALL;
	static const TokenType CONSTRUCTOR     = _CONSTRUCTOR;
	static const TokenType OPERATOR        = _OPERATOR;
	static const TokenType ASSIGNMENTOP    = _ASSIGNMENTOP;
	static const TokenType PREFIXOP        = _PREFIXOP;
	static const TokenType INFIXOP         = _INFIXOP;
	static const TokenType POSTFIXOP       = _POSTFIXOP;
	static const TokenType PREINFIXOP      = _PREINFIXOP;
	static const TokenType PREPOSTFIXOP    = _PREPOSTFIXOP;
	static const TokenType SELECTOR        = _SELECTOR;
	static const TokenType VALUE           = _VALUE;
	static const TokenType FLOAT           = _FLOAT;
	static const TokenType INTEGER         = _INTEGER;
	static const TokenType NUMBER          = _NUMBER;
	static const TokenType DOUBLEQUOTE     = _DOUBLEQUOTE;
	static const TokenType QUOTE           = _QUOTE;
	static const TokenType STOP            = _STOP;
	static const TokenType TAB             = _TAB;
	static const TokenType SPACE           = _SPACE;
	static const TokenType WHITESPACE      = _WHITESPACE;
	static const TokenType NEWLINE         = _NEWLINE;
	static const TokenType LINECOMMENT     = _LINECOMMENT;
	static const TokenType ESCAPED         = _ESCAPED;
	static const TokenType OTHER           = _OTHER;

private:
	static const openxds::base::String* Unknown;
	static const openxds::base::String* Word;
	static const openxds::base::String* File;
	static const openxds::base::String* Package;
	static const openxds::base::String* Import;
	static const openxds::base::String* Include;
	static const openxds::base::String* Class;
	static const openxds::base::String* ClassName;
	static const openxds::base::String* Interface;
	static const openxds::base::String* Enum;
	static const openxds::base::String* Annotation;
	static const openxds::base::String* Method;
	static const openxds::base::String* IMethod;
	static const openxds::base::String* Block;
	static const openxds::base::String* StartBlock;
	static const openxds::base::String* EndBlock;
	static const openxds::base::String* Member;
	static const openxds::base::String* MemberName;
	static const openxds::base::String* Expression;
	static const openxds::base::String* StartExpression;
	static const openxds::base::String* EndExpression;
	static const openxds::base::String* Clause;
	static const openxds::base::String* Parameters;
	static const openxds::base::String* Parameter;
	static const openxds::base::String* Arguments;
	static const openxds::base::String* Argument;
	static const openxds::base::String* Statement;
	static const openxds::base::String* Declaration;
	static const openxds::base::String* Comment;
	static const openxds::base::String* Javadoc;
	static const openxds::base::String* BlankLine;
	static const openxds::base::String* Token;
	static const openxds::base::String* Symbol;
	static const openxds::base::String* Keyword;
	static const openxds::base::String* Modifier;
	static const openxds::base::String* Type;
	static const openxds::base::String* MethodName;
	static const openxds::base::String* Variable;
	static const openxds::base::String* Name;
	static const openxds::base::String* MethodCall;
	static const openxds::base::String* Constructor;
	static const openxds::base::String* Operator;
	static const openxds::base::String* AssignmentOp;
	static const openxds::base::String* PrefixOp;
	static const openxds::base::String* InfixOp;
	static const openxds::base::String* PostfixOp;
	static const openxds::base::String* PreInfixOp;
	static const openxds::base::String* PrePostfixOp;
	static const openxds::base::String* Selector;
	static const openxds::base::String* Value;
	static const openxds::base::String* Float;
	static const openxds::base::String* Integer;
	static const openxds::base::String* Number;
	static const openxds::base::String* DoubleQuote;
	static const openxds::base::String* Quote;
	static const openxds::base::String* Stop;
	static const openxds::base::String* Tab;
	static const openxds::base::String* Space;
	static const openxds::base::String* Whitespace;
	static const openxds::base::String* Newline;
	static const openxds::base::String* LineComment;
	static const openxds::base::String* Escaped;
	static const openxds::base::String* Other;

};
	
};};

#endif