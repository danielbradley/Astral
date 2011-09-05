#include "astral.tokenizer/SourceTokenizer.h"
#include "astral.tokenizer/SourceToken.h"

#include <openxds.util/ITextTokenizer.h>
#include <openxds.util/ITextToken.h>
#include <openxds.util/TextTokenizer.h>
#include <openxds.io/File.h>
#include <openxds.io/FileInputStream.h>
#include <openxds.io/InputStreamReader.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.adt.std/Sequence.h>

using namespace astral::tokenizer;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::exceptions;
using namespace openxds::io;
using namespace openxds::io::exceptions;
using namespace openxds::util;

static String* combine( char ch1, char ch2, char ch3=' ', char ch4=' ' );

SourceTokenizer::SourceTokenizer( const String& location )
{
	File*              file   = new File( location );
	FileInputStream*   is     = new FileInputStream( file );
	InputStreamReader* reader = new InputStreamReader( is );

	this->tt = new TextTokenizer( reader );
	this->tokenQueue = new Sequence<SourceToken>();
}

SourceTokenizer::SourceTokenizer( ITextTokenizer* aTextTokenizer )
{
	this->tt         = aTextTokenizer;
	this->tokenQueue = new Sequence<SourceToken>();
}

SourceTokenizer::~SourceTokenizer()
{
	delete this->tt;
	delete this->tokenQueue;
}

SourceToken*
SourceTokenizer::nextToken() throw (IOException*,NoSuchElementException*)
{
	if ( ! this->hasMoreTokens() ) throw new NoSuchElementException();
	
	return this->tokenQueue->removeFirst();
}

const SourceToken&
SourceTokenizer::peekNextToken() throw (IOException*,NoSuchElementException*)
{
	if ( ! this->hasMoreTokens() ) throw new NoSuchElementException();

	return this->tokenQueue->getFirst();
}

void
SourceTokenizer::pushback( SourceToken* token )
{
	this->tokenQueue->addFirst( token );
}

bool
SourceTokenizer::hasMoreTokens() throw (IOException*)
{
	if ( ! this->tokenQueue->isEmpty() )
	{
		return true;
	}
	else if ( this->tt->hasMoreTokens() )
	{
		ITextToken* token           = this->tt->nextToken();
		{
			ITextToken::ITokenType type = token->getTokenType();
			SourceToken* source_token = NULL;
			String* str = token->getValue().asString();
			SourceToken::TokenType ttype = SourceToken::TOKEN;

			switch ( type )
			{
			case ITextToken::ESCAPED:
				this->tokenQueue->addLast( new SourceToken( SourceToken::ESCAPED, str ) );
				break;
			case ITextToken::NUMBER:
				this->tokenQueue->addLast( new SourceToken( SourceToken::NUMBER, str ) );
				break;
			case ITextToken::NEWLINE:
				this->tokenQueue->addLast( new SourceToken( SourceToken::NEWLINE, str ) );
				break;
			case ITextToken::SPACE:
			case ITextToken::TAB:
			case ITextToken::WHITESPACE:
				delete str;
				source_token = this->parseWhitespace( *token );
				this->tokenQueue->addLast( source_token );
				break;
			case ITextToken::WORD:
				source_token = this->parseWordToken( str );
				this->tokenQueue->addLast( source_token );
				break;
			case ITextToken::SYMBOL:
				source_token = this->parseSymbolToken( str );
				this->tokenQueue->addLast( source_token );
				break;
			default:
				this->tokenQueue->addLast( new SourceToken( ttype, str ) );
			}
		}
		delete token;

		return ! this->tokenQueue->isEmpty();
	}
	else
	{
		return false;
	}
}

SourceToken*
SourceTokenizer::parseWhitespace( const ITextToken& token )
{
	SourceToken* source_token = NULL;
	{
		SourceToken::TokenType sttype = SourceToken::TOKEN;
		switch ( token.getTokenType() )
		{
		case ITextToken::SPACE:
			sttype = SourceToken::SPACE;
			break;
		case ITextToken::TAB:
			sttype = SourceToken::TAB;
			break;
		case ITextToken::WHITESPACE:
			sttype = SourceToken::WHITESPACE;
			break;
		}

		StringBuffer sb;
		sb.append( token.getValue() );
		{
			ISequence<ITextToken>* tokens = new Sequence<ITextToken>();
			{
				bool loop = true;
				while ( loop && this->tt->hasMoreTokens() )
				{
					ITextToken*            next = this->tt->nextToken();
					ITextToken::ITokenType type = next->getTokenType();

					tokens->addLast( next );
					
					switch ( type )
					{
					case ITextToken::SPACE:
					case ITextToken::TAB:
					case ITextToken::WHITESPACE:
						break;
					case ITextToken::NEWLINE:
						sttype = SourceToken::BLANKLINE;
						while ( ! tokens->isEmpty() )
						{
							ITextToken* token = tokens->removeFirst();
							sb.append( token->getValue() );
							delete token;
						}
						loop = false;
						break;
					case ITextToken::SYMBOL:
						if ( next->getValue().contentEquals( "/" ) && this->tt->peekNextToken().getValue().contentEquals( "*" ) )
						{
							ITextToken* last = tokens->removeLast();
							{
								while ( ! tokens->isEmpty() )
								{
									ITextToken* token = tokens->removeFirst();
									sb.append( token->getValue() );
									delete token;
								}
							
								String* str = last->getValue().asString();
								{
									SourceToken* comment = this->parseSymbolToken( str );
									{
										sb.append( comment->getValue() );
										sttype = comment->getTokenType();
									}
									delete comment;
								}
							}
							delete last;
							loop = false;
						} else {
							while ( ! tokens->isEmpty() )
							{
								this->tt->pushback( tokens->removeLast() );
							}
							loop = false;
						}
						break;
					default:
						while ( ! tokens->isEmpty() )
						{
							this->tt->pushback( tokens->removeLast() );
						}
						loop = false;
					}
				}
			}
			delete tokens;
		}
		source_token = new SourceToken( sttype, sb.asString() );
	}
	return source_token;
}

SourceToken*
SourceTokenizer::parseWordToken( String* word )
{
	SourceToken::TokenType ttype = SourceToken::TOKEN;
	StringBuffer sb;
	sb.append( *word );
	delete word;
	{
		bool loop = true;
		while ( loop == this->tt->hasMoreTokens() )
		{
			const ITextToken& token = this->tt->peekNextToken();
			
			switch ( token.getTokenType() )
			{
			case ITextToken::SYMBOL:
				if ( token.getValue().contentEquals( "_" ) )
				{
					sb.append( token.getValue() );
					delete this->tt->nextToken();
				} else {
					loop = false;
				}
				break;
			case ITextToken::WORD:
			case ITextToken::NUMBER:
				sb.append( token.getValue() );
				delete this->tt->nextToken();
				break;
			default:
				loop = false;
			}
		}

		word = sb.asString();

		if ( isKeyword( *word ) )
		{
			ttype = SourceToken::KEYWORD;
		}
		else if ( isModifier( *word ) )
		{
			ttype = SourceToken::MODIFIER;
		}
		else if ( isType( *word ) )
		{
			ttype = SourceToken::TYPE;
		}
		else
		{
			ttype = SourceToken::WORD;
		}
	}
	delete word;
	return new SourceToken( ttype, sb.asString() );
}

SourceToken*
SourceTokenizer::parseSymbolToken( String* word )
{
	SourceToken* source_token = NULL;
	{
		//SourceToken::TokenType ttype = SourceToken::SYMBOL;
		char ch1 = word->charAt(0);
		char ch2 = ' ';
		char ch3 = ' ';
		char ch4 = ' ';

		if ( this->tt->hasMoreTokens() ) ch2 = this->tt->peekNextToken().getValue().charAt(0);

		switch ( ch1 )
		{
		case '{':
			source_token = new SourceToken( SourceToken::STARTBLOCK, word );
			break;
		case '}':
			source_token = new SourceToken( SourceToken::ENDBLOCK, word );
			break;
		case '(':
			source_token = new SourceToken( SourceToken::STARTEXPRESSION, word );
			break;
		case ')':
			source_token = new SourceToken( SourceToken::ENDEXPRESSION, word );
			break;
		case '.':
			source_token = new SourceToken( SourceToken::SELECTOR, word );
			break;
		case ';':
			source_token = new SourceToken( SourceToken::STOP, word );
			break;
		case '~':
			source_token = new SourceToken( SourceToken::PREFIXOP, word );
			break;
		case '@':
			source_token = new SourceToken( SourceToken::ANNOTATION, word );
			break;
		case '=':
			switch ( ch2 )
			{
			case '=':
				source_token = new SourceToken( SourceToken::INFIXOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			default:
				source_token = new SourceToken( SourceToken::ASSIGNMENTOP, word );
			}
			break;
		case '+':
		case '-':
			switch ( ch2 )
			{
			case '=':
				source_token = new SourceToken( SourceToken::ASSIGNMENTOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			case '+':
				source_token = new SourceToken( SourceToken::PREPOSTFIXOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			case '>':
				source_token = new SourceToken( SourceToken::SELECTOR, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			default:
				source_token = new SourceToken( SourceToken::INFIXOP, word );
			}
			break;
		case '*':
			switch ( ch2 )
			{
			case '=':
				source_token = new SourceToken( SourceToken::ASSIGNMENTOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			default:
				source_token = new SourceToken( SourceToken::INFIXOP, word );
			}
			break;
		case '&':
			switch ( ch2 )
			{
			case '=':
				source_token = new SourceToken( SourceToken::ASSIGNMENTOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			case '&':
				source_token = new SourceToken( SourceToken::INFIXOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			default:
				source_token = new SourceToken( SourceToken::PREINFIXOP, word );
			}
			break;
		case '|':
			switch ( ch2 )
			{
			case '=':
				source_token = new SourceToken( SourceToken::ASSIGNMENTOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			case '|':
				source_token = new SourceToken( SourceToken::INFIXOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			default:
				source_token = new SourceToken( SourceToken::INFIXOP, word );
			}
			break;
		case '^':
			switch ( ch2 )
			{
			case '=':
				source_token = new SourceToken( SourceToken::ASSIGNMENTOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			default:
				source_token = new SourceToken( SourceToken::PREFIXOP, word );
			}
			break;
		case '%':
			switch ( ch2 )
			{
			case '=':
				source_token = new SourceToken( SourceToken::ASSIGNMENTOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			default:
				source_token = new SourceToken( SourceToken::INFIXOP, word );
			}
			break;
		case '!':
			switch ( ch2 )
			{
			case '=':
				source_token = new SourceToken( SourceToken::INFIXOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			default:
				source_token = new SourceToken( SourceToken::PREFIXOP, word );
			}
			break;
		case '<':
			switch ( ch2 )
			{
			case '=':
				source_token = new SourceToken( SourceToken::INFIXOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			case '<':
				delete this->tt->nextToken();
				if ( this->tt->hasMoreTokens() ) ch3 = tt->peekNextToken().getValue().charAt(0);
				
				switch ( ch3 )
				{
				case '=':
					source_token = new SourceToken( SourceToken::ASSIGNMENTOP, combine( ch1, ch2, ch3 ) );
					delete this->tt->nextToken();
					delete word;
					break;
				default:
					source_token = new SourceToken( SourceToken::INFIXOP, combine( ch1, ch2 ) );
					delete word;
				}
				break;
			default:
				source_token = new SourceToken( SourceToken::INFIXOP, word );
			}
			break;
		case '>':
			switch ( ch2 )
			{
			case '=':
				source_token = new SourceToken( SourceToken::INFIXOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			case '>':
				delete this->tt->nextToken();
				if ( this->tt->hasMoreTokens() ) ch3 = tt->peekNextToken().getValue().charAt(0);
				
				switch ( ch3 )
				{
				case '=':
					source_token = new SourceToken( SourceToken::ASSIGNMENTOP, combine( ch1, ch2, ch3 ) );
					delete this->tt->nextToken();
					delete word;
					break;
				case '>':
					delete this->tt->nextToken();
					if ( this->tt->hasMoreTokens() ) ch4 = tt->peekNextToken().getValue().charAt(0);
				
					switch ( ch4 )
					{
					case '=':
						source_token = new SourceToken( SourceToken::ASSIGNMENTOP, combine( ch1, ch2, ch3, ch4 ) );
						delete this->tt->nextToken();
						delete word;
						break;
					default:
						source_token = new SourceToken( SourceToken::INFIXOP, combine( ch1, ch2, ch3 ) );
						delete word;
					}
					break;
				default:
					source_token = new SourceToken( SourceToken::INFIXOP, combine( ch1, ch2 ) );
					delete word;
				}
				break;
			default:
				source_token = new SourceToken( SourceToken::INFIXOP, word );
			}
			break;
		case '/':
			switch ( ch2 )
			{
			case '=':
				source_token = new SourceToken( SourceToken::ASSIGNMENTOP, combine( ch1, ch2 ) );
				delete this->tt->nextToken();
				delete word;
				break;
			case '/':
				source_token = this->parseLineComment( word );
				break;
			case '*':
				source_token = this->parseComment( word );
				break;
			default:
				source_token = new SourceToken( SourceToken::INFIXOP, word );
			}
			break;
		case '\\':
			switch ( ch2 )
			{
			default:
				source_token = new SourceToken( SourceToken::ESCAPED, word );
			}
			break;
		case '\'':
			source_token = this->parseQuote( word );
			break;
		case '\"':
			source_token = this->parseDoubleQuote( word );
			break;
		default:
			source_token = new SourceToken( SourceToken::SYMBOL, word );
		}
	}
	return source_token;
}

static String* combine( char ch1, char ch2, char ch3, char ch4 )
{
	StringBuffer sb;
	sb.append( ch1 );
	sb.append( ch2 );
	
	if ( ' ' != ch3 ) sb.append( ch3 );
	if ( ' ' != ch4 ) sb.append( ch4 );
	
	return sb.asString();
}

SourceToken*
SourceTokenizer::parseLineComment( openxds::base::String* word )
{
	StringBuffer sb;
	sb.append( *word );
	delete word;
	
	while ( this->tt->hasMoreTokens() )
	{
		ITextToken* token           = this->tt->nextToken();
		ITextToken::ITokenType type = token->getTokenType();
		{
			sb.append( token->getValue() );
		}
		delete token;
		if ( ITextToken::NEWLINE == type ) break;
	}
	
	return new SourceToken( SourceToken::LINECOMMENT, sb.asString() );
}

SourceToken*
SourceTokenizer::parseComment( openxds::base::String* word )
{
	SourceToken::TokenType sttype = SourceToken::COMMENT;
	StringBuffer sb;
	sb.append( *word );
	delete word;

	bool first = true;
	bool loop = true;
	const char* ch;
	while ( loop && this->tt->hasMoreTokens() )
	{
		ITextToken* token           = this->tt->nextToken();
		{
			ITextToken::ITokenType type = token->getTokenType();
			{
				sb.append( token->getValue() );
				ch = token->getValue().getChars();
			}
			
			if ( first )
			{
				if ( this->tt->peekNextToken().getValue().contentEquals( "*" ) )
				{
					sttype = SourceToken::JAVADOC;
				}
				first = false;
			}

			if ( ITextToken::SYMBOL == type )
			{
				const ITextToken& next = this->tt->peekNextToken();
				
				if ( token->getValue().contentEquals( "*" ) && next.getValue().contentEquals( "/" ) )
				{
					sb.append( next.getValue() );
					delete tt->nextToken();
					
					switch( this->tt->peekNextToken().getTokenType() )
					{
					case ITextToken::TAB:
					case ITextToken::SPACE:
					case ITextToken::WHITESPACE:
					case ITextToken::NEWLINE:
						{
							ITextToken* ws = this->tt->nextToken();
							{
								SourceToken* st = this->parseWhitespace( *ws );
								sb.append( st->getValue() );
								delete st;
							}
							delete ws;
						}
					}
					loop = false;
				}
			}
		}
		delete token;
	}
	
	return new SourceToken( sttype, sb.asString() );
}

SourceToken*
SourceTokenizer::parseQuote( openxds::base::String* word )
{
	StringBuffer sb;
	sb.append( *word );
	delete word;

	bool loop = true;
	while ( loop && this->tt->hasMoreTokens() )
	{
		ITextToken* token = this->tt->nextToken();
		{
			ITextToken::ITokenType type = token->getTokenType();
			{
				sb.append( token->getValue() );
				if ( (ITextToken::SYMBOL == type) && (token->getValue().contentEquals( "\'" )) ) loop = false;
			}
		}
		delete token;
	}
	
	return new SourceToken( SourceToken::QUOTE, sb.asString() );
}

SourceToken*
SourceTokenizer::parseDoubleQuote( openxds::base::String* word )
{
	StringBuffer sb;
	sb.append( *word );
	delete word;

	bool loop = true;
	while ( loop && this->tt->hasMoreTokens() )
	{
		ITextToken* token = this->tt->nextToken();
		{
			ITextToken::ITokenType type = token->getTokenType();
			{
				sb.append( token->getValue() );
				if ( (ITextToken::SYMBOL == type) && (token->getValue().contentEquals( "\"" )) ) loop = false;
			}
		}
		delete token;
	}
	
	return new SourceToken( SourceToken::DOUBLEQUOTE, sb.asString() );
}


static bool processExpression( ISequence<SourceToken>& tokens, SourceTokenizer& tokenizer )
{
	int  level = 1;
	bool loop  = true;
	while ( loop && tokenizer.hasMoreTokens() )
	{
		SourceToken* token = tokenizer.nextToken();
		tokens.addLast( token );
		
		switch ( token->getTokenType() )
		{
		case SourceToken::STOP:
			loop = false;
			break;
		case SourceToken::STARTEXPRESSION:
			level++;
			break;
		case SourceToken::ENDEXPRESSION:
			level--;
			break;
		}
		
		if ( 0 == level) loop = false;
	}
	
	return (0 == level);
}

/**
 *	Looks ahead up the tokenizer to determine what type of statement is being parsed.
 */
SourceToken::TokenType
SourceTokenizer::sneakyPeek()
{
	SourceToken::TokenType ret = SourceToken::STATEMENT;
	
	Sequence<SourceToken> tokens;

	//int modifiers = 0;
	int keywords  = 0;
	int words     = 0;
	int spaces    = 0;
	int exp       = 0;
	int ass       = 0;
	int blocks    = 0;
	int stops     = 0;
	
	bool is_package = false;
	bool is_import  = false;
	
	bool loop = true;
	while ( loop && this->hasMoreTokens() )
	{
		SourceToken* token = this->nextToken();
		tokens.addLast( token );

		switch ( token->getTokenType() )
		{
		case SourceToken::SPACE:
		case SourceToken::TAB:
		case SourceToken::WHITESPACE:
			spaces++;
			break;
		case SourceToken::KEYWORD:
			keywords++;
			     if ( token->getValue().contentEquals( "package" ) ) { is_package = true; }
			else if ( token->getValue().contentEquals( "import"  ) ) { is_import  = true; }
			break;
		case SourceToken::TYPE:
		case SourceToken::WORD:
			words++;
			{
				bool inner_loop = true;
				while ( inner_loop && this->hasMoreTokens() )
				{
					const SourceToken& token = this->peekNextToken();
					
					switch ( token.getTokenType() )
					{
					case SourceToken::SELECTOR:
					case SourceToken::WORD:
						tokens.addLast( this->nextToken() );
						break;
					case SourceToken::STARTEXPRESSION:
						exp++;
						tokens.addLast( this->nextToken() );
						if ( ! processExpression( tokens, *this ) )
						{
							inner_loop = false;
							loop       = false;
						}
						break;
					default:
						inner_loop = false;
					}
				}
			}
			break;
		case SourceToken::STARTEXPRESSION:
			exp++;
			break;
		case SourceToken::ASSIGNMENTOP:
			ass++;
			loop = false;
			break;
		case SourceToken::STARTBLOCK:
			blocks++;
			loop = false;
			break;
		case SourceToken::STOP:
			stops++;
			loop = false;
			break;
		}
	}

	while ( ! tokens.isEmpty() )
	{
		this->pushback( tokens.removeLast() );
	}

	if ( blocks )
	{
		//	class/interface, enum, if, else, while, switch, block, method.

		if ( keywords )
		{
			//	class/interface, enum, if, else, while, switch.

			if ( exp )
			{
				// if, else if, while, switch
				ret = SourceToken::STATEMENT;
			}
			else
			{
				//	class/interface
				ret = SourceToken::CLASS;
			}
		}
		else
		{
			if ( exp )
			{
				//	method
				ret = SourceToken::METHOD;
			}
			else
			{
				ret = SourceToken::STATEMENT;
			}
		}
	}
	else
	{
		//	package, import, declaration (member, local), statement, lval, rval.
		
		if ( words )
		{
			if ( is_package )
			{
				ret = SourceToken::PACKAGE;
			}
			else if ( is_import )
			{
				ret = SourceToken::IMPORT;
			}
			else if ( words >= 2 )
			{
				ret = SourceToken::DECLARATION;
			}
			else
			{
				ret = SourceToken::STATEMENT;
			}
		}
		else
		{
			ret = SourceToken::STATEMENT;
		}
	}

	return ret;
}