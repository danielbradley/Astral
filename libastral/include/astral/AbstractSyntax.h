#ifndef CROSSADAPTIVE_ASTRAL_ABSTRACTSYNTAX_H
#define CROSSADAPTIVE_ASTRAL_ABSTRACTSYNTAX_H

#include <openxds.base.h>
#include <openxds.util.h>
#include <openxds/Object.h>

namespace crossadaptive {
	namespace astral {
	
class AbstractSyntax : openxds::Object
{
public:
	enum AbstractSyntaxType
	{
		_FILE,
		_PACKAGE,
		_IMPORT,
		_CLASS,
		_INTERFACE,
		_METHOD,
		_BLOCK,
		_MEMBER,
		_EXPRESSION,
		_PARAMETERS,
		_STATEMENT,
		_DECLARATION,
		_COMMENT,
		_BLANKLINE,
		_TOKEN,
		_SYMBOL,
		_KEYWORD,
		_TYPE,
		_METHODNAME,
		_VARIABLE,
		_NAME,
		_METHODCALL,
		_OPERATOR,
		_SELECTOR,
		_VALUE,
		_STOP,
		_TAB,
		_WHITESPACE,
		_NEWLINE,
		_LINECOMMENT
	};

private:
	AbstractSyntaxType     type;
	openxds::util::IToken* token;

public:
	         AbstractSyntax( AbstractSyntax::AbstractSyntaxType aType, openxds::util::IToken* aToken );
	virtual	~AbstractSyntax();
	
//	virtual       void                         setASType( AbstractSyntaxType aType );
//
	virtual const openxds::base::String&       getValue() const;
	virtual const openxds::util::IToken&       getToken() const;
	virtual       openxds::base::String*       toString() const;
	virtual       AbstractSyntaxType       getTokenType() const;
	virtual const openxds::base::String& getTokenString() const;
	
	static const AbstractSyntaxType FILE        = _FILE;
	static const AbstractSyntaxType PACKAGE     = _PACKAGE;
	static const AbstractSyntaxType IMPORT      = _IMPORT;
	static const AbstractSyntaxType CLASS       = _CLASS;
	static const AbstractSyntaxType INTERFACE   = _INTERFACE;
	static const AbstractSyntaxType METHOD      = _METHOD;
	static const AbstractSyntaxType BLOCK       = _BLOCK;
	static const AbstractSyntaxType MEMBER      = _MEMBER;
	static const AbstractSyntaxType EXPRESSION  = _EXPRESSION;
	static const AbstractSyntaxType PARAMETERS  = _PARAMETERS;
	static const AbstractSyntaxType STATEMENT   = _STATEMENT;
	static const AbstractSyntaxType DECLARATION = _DECLARATION;
	static const AbstractSyntaxType COMMENT     = _COMMENT;
	static const AbstractSyntaxType BLANKLINE   = _BLANKLINE;
	static const AbstractSyntaxType TOKEN       = _TOKEN;
	static const AbstractSyntaxType SYMBOL      = _SYMBOL;
	static const AbstractSyntaxType KEYWORD     = _KEYWORD;
	static const AbstractSyntaxType TYPE        = _TYPE;
	static const AbstractSyntaxType METHODNAME  = _METHODNAME;
	static const AbstractSyntaxType VARIABLE    = _VARIABLE;
	static const AbstractSyntaxType NAME        = _NAME;
	static const AbstractSyntaxType METHODCALL  = _METHODCALL;
	static const AbstractSyntaxType OPERATOR    = _OPERATOR;
	static const AbstractSyntaxType SELECTOR    = _SELECTOR;
	static const AbstractSyntaxType VALUE       = _VALUE;
	static const AbstractSyntaxType STOP        = _STOP;
	static const AbstractSyntaxType TAB         = _TAB;
	static const AbstractSyntaxType WHITESPACE  = _WHITESPACE;
	static const AbstractSyntaxType NEWLINE     = _NEWLINE;
	static const AbstractSyntaxType LINECOMMENT = _LINECOMMENT;

private:
	static const openxds::base::String* Unknown;
	static const openxds::base::String* File;
	static const openxds::base::String* Package;
	static const openxds::base::String* Import;
	static const openxds::base::String* Class;
	static const openxds::base::String* Interface;
	static const openxds::base::String* Method;
	static const openxds::base::String* Block;
	static const openxds::base::String* Member;
	static const openxds::base::String* Expression;
	static const openxds::base::String* Parameters;
	static const openxds::base::String* Statement;
	static const openxds::base::String* Declaration;
	static const openxds::base::String* Comment;
	static const openxds::base::String* BlankLine;
	static const openxds::base::String* Token;
	static const openxds::base::String* Symbol;
	static const openxds::base::String* Keyword;
	static const openxds::base::String* Type;
	static const openxds::base::String* MethodName;
	static const openxds::base::String* Variable;
	static const openxds::base::String* Name;
	static const openxds::base::String* MethodCall;
	static const openxds::base::String* Operator;
	static const openxds::base::String* Selector;
	static const openxds::base::String* Value;
	static const openxds::base::String* Stop;
	static const openxds::base::String* Tab;
	static const openxds::base::String* Whitespace;
	static const openxds::base::String* Newline;
	static const openxds::base::String* LineComment;
	
};

	
	
};};

#endif