#include "astral.ast/AST.h"
#include "astral.tokenizer/SourceToken.h"
#include "astral.tokenizer/JavaTokenizer.h"

#include <openxds.io/File.h>
#include <openxds.io.exceptions/FileNotFoundException.h>
#include <openxds.io.exceptions/IOException.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.adt.std/Tree.h>
#include <openxds.adt/ITree.h>
#include <openxds.base/Character.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.io/InputStream.h>
#include <openxds.io/InputStreamReader.h>
#include <openxds.io/IOBuffer.h>

using namespace astral::ast;
using namespace astral::tokenizer;
using namespace openxds::io;
using namespace openxds::io::exceptions;
using namespace openxds::base;
using namespace openxds::adt;
using namespace openxds::adt::std;

static bool                parseAll( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static bool          parseStatement( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer, bool parseArg );
//static void         parseStatementX( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void           handleKeyword( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void              handleStop( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static bool        handleStartBlock( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer, bool parseEnum );
static void        handleMethodCall( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer, bool parsedNew );
static void   handleStartParameters( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void         parseParameters( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static String*       parseParameter( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void    handleStartArguments( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void          parseArguments( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void   handleStartExpression( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void         parseExpression( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );

//static void           parseArgument( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static bool              parseBlock( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer, bool parseEnum );
static void      parseEnumStatement( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void parseTrailingWhitespace( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );

static SourceToken* parseType( SourceTokenizer& tokenizer );
static void pushbackTokens( SourceTokenizer& tokenizer, Sequence<SourceToken>& tokens );

static bool           isMemberLevel( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void                 countup( ITree<SourceToken>& ast, IPosition<SourceToken>& parent );

AST::AST()
{
	this->ast      = new Tree<SourceToken>();
	this->location = new String();
	this->valid    = true;
}

AST::~AST()
{
	delete this->location;
	delete this->ast;
}

openxds::adt::ITree<SourceToken>&
AST::getTree() const
{
	return *this->ast;
}

void
AST::parseString( const String& content )
{
	delete ast->addRoot( new SourceToken( SourceToken::OTHER, new String() ) );

	SourceTokenizer* t = new JavaTokenizer( new InputStreamReader( new InputStream( new IOBuffer( content ) ) ) );
	{
		parseFromTokenizer( *t );
	}
	delete t;
}

void
AST::parseFile( const char* location )
{
	delete this->location;
	this->location = new String( location );

	delete ast->addRoot( new SourceToken( SourceToken::FILE, this->location->asString() ) );

	SourceTokenizer* tokenizer = new JavaTokenizer( *this->location );
	{
		this->parseFromTokenizer( *tokenizer );
	}
	delete tokenizer;
}




void
AST::parseFromTokenizer( SourceTokenizer& tokenizer )
{
	try
	{
		IPosition<SourceToken>* root = this->ast->root();
		{
			this->valid = parseAll( *this->ast, *root, tokenizer );
		}
		countup( *this->ast, *root );
		delete root;
	}
	catch ( FileNotFoundException* ex )
	{
		fprintf( stderr, "%s\n", ex->getMessage() );
		delete ex;
	}
	catch ( IOException* ex )
	{
		fprintf( stderr, "%s\n", ex->getMessage() );
		delete ex;
	}
}

static bool
parseAll( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	bool status = true;

	while ( tokenizer.hasMoreTokens() )
	{
		SourceToken::TokenType type = tokenizer.peekNextToken().getTokenType();
		switch ( type )
		{
		case SourceToken::NEWLINE:
		case SourceToken::BLANKLINE:
		//case SourceToken::COMMENT:
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;
			
		case SourceToken::ENDBLOCK:
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;
		
		default:
			SourceToken* token = new SourceToken( SourceToken::STATEMENT, new String() );
			IPosition<SourceToken>* p = ast.addChild( parent, token );
			{
				status &= parseStatement( ast, *p, tokenizer, false );
			}
			countup( ast, *p );
			delete p;
		}
	}

	return status;
}

static bool isMethodCall( SourceTokenizer& tokenizer )
{
	bool ret = false;
	Sequence<SourceToken> tokens;
	
	tokens.addLast( tokenizer.nextToken() );
	
	bool loop = true;
	while ( loop && tokenizer.hasMoreTokens() )
	{
		SourceToken* token = tokenizer.nextToken();
		switch ( token->getTokenType() )
		{
		case SourceToken::SPACE:
			break;
		case SourceToken::STARTEXPRESSION:
			ret = true;
			loop = false;
			break;
		default:
			loop = false;
		}
		tokens.addLast( token );
	}
	
	pushbackTokens( tokenizer, tokens );

	return ret;
}

static bool
parseStatement( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer, bool parseArg )
{
	bool status = true;

	SourceToken::TokenType looksie = tokenizer.sneakyPeek();
	if ( parseArg )
	{
		looksie = parent.getElement().getTokenType();
	}
	else
	{
		parent.getElement().setType( looksie );
	}

	bool parsed_new  = false;

	SourceToken* st_type = NULL;
	
	bool loop = true;
	while ( loop && tokenizer.hasMoreTokens() )
	{
		const char* _value = tokenizer.peekNextToken().getValue().getChars(); _value = _value;
		SourceToken::TokenType type = tokenizer.peekNextToken().getTokenType();

		switch ( type )
		{
		case SourceToken::STOP:
			handleStop( ast, parent, tokenizer );
			loop = false;
			break;

		case SourceToken::ENDBLOCK:
			loop = false;
			break;

		case SourceToken::SYMBOL:
			if ( parseArg && tokenizer.peekNextToken().getValue().contentEquals( "," ) )
			{
				handleStop( ast, parent, tokenizer );
				loop = false;
			}
			else
			{
				delete ast.addChild( parent, tokenizer.nextToken() );
			}
			break;

		case SourceToken::LINECOMMENT:
			delete ast.addChild( parent, tokenizer.nextToken() );
			loop = false;
			break;

		case SourceToken::STARTBLOCK:
			status = handleStartBlock( ast, parent, tokenizer, (SourceToken::ENUM == looksie) );
			loop = false;
			break;

		case SourceToken::KEYWORD:
			if ( tokenizer.peekNextToken().getValue().contentEquals( "new" ) )
			{
				parsed_new = true;
			}
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;

		case SourceToken::STARTEXPRESSION:
			switch ( looksie )
			{
			case SourceToken::METHOD:
				handleStartParameters( ast, parent, tokenizer );
				break;

			default:
				handleStartExpression( ast, parent, tokenizer );
			}
			break;

		case SourceToken::ENDEXPRESSION:
			if ( parseArg )
			{
				loop = false;
			}
			else
			{
				delete ast.addChild( parent, tokenizer.nextToken() );
			}
			break;

		case SourceToken::TYPE:
			switch ( looksie )
			{
			case SourceToken::METHOD:
				st_type = parseType( tokenizer );
				parent.getElement().setValue( new String( st_type->getValue() ) );
				delete ast.addChild( parent, st_type );
				break;
			
			default:
				st_type = parseType( tokenizer );
				delete ast.addChild( parent, st_type );
			}
			break;

		case SourceToken::WORD:
			{
				switch ( looksie )
				{
				case SourceToken::PACKAGE:
				case SourceToken::IMPORT:
					{
						SourceToken* parsed = parseType( tokenizer );
						parsed->setType( SourceToken::NAME );
						delete ast.addChild( parent, parsed );
					}
					break;

				case SourceToken::CLASS:
					{
						SourceToken* parsed = parseType( tokenizer );
						parsed->setType( SourceToken::CLASSNAME );
						delete ast.addChild( parent, parsed );
					}
					break;

				case SourceToken::METHOD:
					{
						SourceToken* token = tokenizer.nextToken();
						token->setType( SourceToken::NAME );
						if ( st_type )
						{
							token->setType( SourceToken::METHODNAME );
							parent.getElement().setValue( new String( token->getValue() ) );
						}
						delete ast.addChild( parent, token );
					}
					break;

				default:
					if ( isMethodCall( tokenizer ) )
					{
						handleMethodCall( ast, parent, tokenizer, parsed_new );
						parsed_new = false;
					}
					else if ( st_type )
					{
						SourceToken* token = tokenizer.nextToken();
						token->setType( SourceToken::VARIABLE );
						delete ast.addChild( parent, token );
						
						st_type = NULL;
					}
					else
					{
						SourceToken* token = tokenizer.nextToken();
						token->setType( SourceToken::NAME );
						delete ast.addChild( parent, token );
					}
				}
			}
			break;

		default:
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;
		}
	}

	status &= (false == loop);

	return status;
}

static bool
parseStatementX( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer, bool parseArg )
{
	bool status = true;

	SourceToken::TokenType looksie = tokenizer.sneakyPeek();
	if ( parseArg )
	{
		looksie = parent.getElement().getTokenType();// SourceToken::OTHER;
	}
	else
	{
		parent.getElement().setType( looksie );
	}

	bool could_be_type = true;

	bool parsed_type   = false;
	bool parsed_new    = false;
	//bool parsed_params = false;
	int  words         = 0;

	SourceToken* st_type = NULL;
	
	bool loop = true;
	while ( loop && tokenizer.hasMoreTokens() )
	{
		const char* _value = tokenizer.peekNextToken().getValue().getChars();
		SourceToken::TokenType type = tokenizer.peekNextToken().getTokenType();

		switch ( type )
		{
		case SourceToken::STOP:
			handleStop( ast, parent, tokenizer );
			loop = false;
			break;

		case SourceToken::ENDBLOCK:
			loop = false;
			break;

		case SourceToken::SYMBOL:
			if ( parseArg && tokenizer.peekNextToken().getValue().contentEquals( "," ) )
			{
				handleStop( ast, parent, tokenizer );
				loop = false;
			}
			else
			{
				delete ast.addChild( parent, tokenizer.nextToken() );
			}
			break;

		case SourceToken::LINECOMMENT:
			delete ast.addChild( parent, tokenizer.nextToken() );
			loop = false;
			break;

		case SourceToken::STARTBLOCK:
			status = handleStartBlock( ast, parent, tokenizer, (SourceToken::ENUM == looksie) );
			loop = false;
			break;

		case SourceToken::KEYWORD:
			if ( tokenizer.peekNextToken().getValue().contentEquals( "new" ) )
			{
				parsed_new = true;
			}
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;

		case SourceToken::STARTEXPRESSION:
			switch ( looksie )
			{
			case SourceToken::METHOD:
				handleStartParameters( ast, parent, tokenizer );
				break;

			default:
				handleStartExpression( ast, parent, tokenizer );
			}
			break;

		case SourceToken::ENDEXPRESSION:
			if ( parseArg )
			{
				loop = false;
			}
			else
			{
				delete ast.addChild( parent, tokenizer.nextToken() );
			}
			break;

		case SourceToken::SELECTOR:
			words = 0;
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;

		case SourceToken::TYPE:
		case SourceToken::WORD:
		
			if ( !parsed_type )
			{
				switch ( looksie )
				{
				case SourceToken::METHOD:
					{
						st_type = parseType( tokenizer );
						st_type->setType( SourceToken::METHODNAME );
						parent.getElement().setValue( new String( st_type->getValue() ) );
						delete ast.addChild( parent, st_type );
					}
					break;

				case SourceToken::PACKAGE:
				case SourceToken::IMPORT:
					{
						SourceToken* parsed = parseType( tokenizer );
						parsed->setType( SourceToken::NAME );
						delete ast.addChild( parent, parsed );
					}
					break;

				case SourceToken::CLASS:
					{
						SourceToken* parsed = parseType( tokenizer );
						parsed->setType( SourceToken::CLASSNAME );
						delete ast.addChild( parent, parsed );
					}
					break;

				case SourceToken::DECLARATION:
					{
						st_type = parseType( tokenizer );
						delete ast.addChild( parent, st_type );
					}
					break;

				default:
					if ( isMethodCall( tokenizer ) )
					{
						handleMethodCall( ast, parent, tokenizer, parsed_new );
						parsed_new = false;
						words--;
					} else {
						SourceToken* token = tokenizer.nextToken();
						token->setType( SourceToken::NAME );
						delete ast.addChild( parent, token );
					}
				}
				parsed_type = true;
			}
			else
			{
				switch ( looksie )
				{
				case SourceToken::CLAUSE:
				case SourceToken::DECLARATION:
					{
						if ( (1 != words) && isMethodCall( tokenizer ) )
						{
							handleMethodCall( ast, parent, tokenizer, parsed_new );
							parsed_new = false;
						}
						else
						{
							SourceToken* token = tokenizer.nextToken();
							if ( 1 == words )
							{
								token->setType( SourceToken::VARIABLE );
								if ( st_type ) st_type->setType( SourceToken::TYPE );
							}
							else
							{
								token->setType( SourceToken::NAME );
							}
							delete ast.addChild( parent, token );
						}
					}
					break;
				case SourceToken::METHOD:
					{
						SourceToken* token = tokenizer.nextToken();
						token->setType( SourceToken::NAME );
						if ( 1 == words )
						{
							if ( st_type ) st_type->setType( SourceToken::TYPE );
							token->setType( SourceToken::METHODNAME );
							parent.getElement().setValue( new String( token->getValue() ) );
						}
						delete ast.addChild( parent, token );
					}
					break;
				default:
					if ( isMethodCall( tokenizer ) )
					{
						handleMethodCall( ast, parent, tokenizer, parsed_new );
						parsed_new = false;
						words--;
					} else {
						SourceToken* token = tokenizer.nextToken();
						token->setType( SourceToken::NAME );
						delete ast.addChild( parent, token );
					}
				}
			}
			words++;
			break;
		default:
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;
		}
	}

	status &= (false == loop);

	return status;
}

static void pushbackTokens( SourceTokenizer& tokenizer, Sequence<SourceToken>& tokens )
{
	while ( ! tokens.isEmpty() )
	{
		tokenizer.pushback( tokens.removeLast() );
	}
}

static void concatTokens( StringBuffer& sb, Sequence<SourceToken>& tokens )
{
	while ( ! tokens.isEmpty() )
	{
		sb.append( tokens.getFirst().getValue() );
		delete tokens.removeFirst();
	}
}

static SourceToken* parseType( SourceTokenizer& tokenizer )
{
	SourceToken* ret = NULL;
	{
		StringBuffer sb;
		{
			const char* val = tokenizer.peekNextToken().getValue().getChars();

			Sequence<SourceToken> tokens;
			tokens.addLast( tokenizer.nextToken() );
			
			bool loop = true;
			while ( loop && tokenizer.hasMoreTokens() )
			{
				val = tokenizer.peekNextToken().getValue().getChars();
			
				switch ( tokenizer.peekNextToken().getTokenType() )
				{
				case SourceToken::SELECTOR:
				case SourceToken::WORD:
				case SourceToken::INFIXOP:
					tokens.addLast( tokenizer.nextToken() );
					break;
				case SourceToken::STOP:
					concatTokens( sb, tokens );
					ret = new SourceToken( SourceToken::TYPE, sb.asString() );
					loop = false;
					break;
				case SourceToken::SPACE:
					tokens.addLast( tokenizer.nextToken() );
					switch ( tokenizer.peekNextToken().getTokenType() )
					{
					case SourceToken::WORD:
						tokenizer.pushback( tokens.removeLast() );
						concatTokens( sb, tokens );
						ret = new SourceToken( SourceToken::TYPE, sb.asString() );
						loop = false;
						break;
					default:
						pushbackTokens( tokenizer, tokens );
						ret = tokenizer.nextToken();
						loop = false;
					}
					break;
				case SourceToken::STARTEXPRESSION:
				default:
					pushbackTokens( tokenizer, tokens );
					ret = tokenizer.nextToken();
					loop = false;
				}
			}
		}
	}
	return ret;
}

static String* generalizeType( const String& type )
{
	String* gen_type = NULL;
	if ( type.contentEquals( "long" ) || type.contentEquals( "int" ) || type.contentEquals( "short" ) )
	{
		gen_type = new String( "INTEGER" );
	}
	else if ( type.contentEquals( "float" ) || type.contentEquals( "double" ) )
	{
		gen_type = new String( "FLOAT" );
	}
	else
	{
		gen_type = new String( type );
	}
	return gen_type;
}

static bool isMemberLevel( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	bool ret = false;

	if ( ast.hasParent( parent ) )
	{
		IPosition<SourceToken>* p1 = ast.parent( parent );
		if ( ast.hasParent( *p1 ) )
		{
			IPosition<SourceToken>* p2 = ast.parent( *p1 );
			if ( SourceToken::CLASS == p2->getElement().getTokenType() )
			{
				ret = true;
			}
			delete p2;
		}
		delete p1;
	}
	return ret;
}			

static bool handleStartBlock( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer, bool parseEnum )
{
	bool status = true;

	IPosition<SourceToken>* block = ast.addChild( parent, new SourceToken( SourceToken::BLOCK, new String() ) );
	{
		delete ast.addChild( *block, tokenizer.nextToken() );
		parseTrailingWhitespace( ast, *block, tokenizer );
		status &= parseBlock( ast, *block, tokenizer, parseEnum );
		parseTrailingWhitespace( ast, *block, tokenizer );
	}
	countup( ast, *block );
	delete block;

	return status;
}

	static bool parseBlock( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer, bool parseEnum )
	{
		bool loop = true;
		while ( loop && tokenizer.hasMoreTokens() )
		{
			const SourceToken&      token = tokenizer.peekNextToken();
			SourceToken::TokenType  ttype = token.getTokenType();
			IPosition<SourceToken>* stmt  = NULL;
			
			switch ( ttype )
			{
			case SourceToken::BLANKLINE:
			//case SourceToken::COMMENT:
			case SourceToken::NEWLINE:
			case SourceToken::TAB:
				delete ast.addChild( parent, tokenizer.nextToken() );
				break;
			case SourceToken::ENDBLOCK:
				delete ast.addChild( parent, tokenizer.nextToken() );
				loop = false;
				break;
			default:
				if ( parseEnum )
				{
					stmt = ast.addChild( parent, new SourceToken( SourceToken::STATEMENT, new String() ) );
					parseEnumStatement( ast, *stmt, tokenizer );
				}
				else
				{
					stmt = ast.addChild( parent, new SourceToken( SourceToken::STATEMENT, new String() ) );
					parseStatement( ast, *stmt, tokenizer, false );
				}
				countup( ast, *stmt );
				delete stmt;
			}
		}
		
		return (loop == false);
	}

		static void parseEnumStatement( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
		{
			bool loop = true;
			while ( loop && tokenizer.hasMoreTokens() )
			{
				const SourceToken&     token = tokenizer.peekNextToken();
				SourceToken::TokenType ttype = token.getTokenType();
				
				switch ( ttype )
				{
				case SourceToken::ENDBLOCK:
					loop = false;
					break;
					
				case SourceToken::STOP:
					delete ast.addChild( parent, tokenizer.nextToken() );
					parseTrailingWhitespace( ast, parent, tokenizer );
					loop = false;
					break;
					
				case SourceToken::WORD:
					{
						SourceToken* tok = tokenizer.nextToken();
						tok->setType( SourceToken::VARIABLE );
						delete ast.addChild( parent, tok );
					}
					break;
					
				default:
					delete ast.addChild( parent, tokenizer.nextToken() );
				}
			}
		}


static void handleStartParameters( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	IPosition<SourceToken>* params = ast.addChild( parent, new SourceToken( SourceToken::PARAMETERS, new String() ) );
	{
		delete ast.addChild( *params, tokenizer.nextToken() );
		parseTrailingWhitespace( ast, *params, tokenizer );
		parseParameters( ast, *params, tokenizer );
		parseTrailingWhitespace( ast, *params, tokenizer );

		StringBuffer sb;
		sb.append( parent.getElement().getValue() );
		sb.append( '(' );
		sb.append( params->getElement().getValue() );
		sb.append( ')' );
		
		parent.getElement().setValue( sb.asString() );
	}
	countup( ast, *params );
	delete params;
}

	static void parseParameters( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
	{
		StringBuffer sb;
	
		bool loop = true;
		while ( loop && tokenizer.hasMoreTokens() )
		{
			const SourceToken&      token = tokenizer.peekNextToken();
			SourceToken::TokenType  ttype = token.getTokenType();
			IPosition<SourceToken>* stmt  = NULL;
			
			switch ( ttype )
			{
			case SourceToken::BLANKLINE:
				delete ast.addChild( parent, tokenizer.nextToken() );
				break;
			case SourceToken::TAB:
				delete ast.addChild( parent, tokenizer.nextToken() );
				break;
			case SourceToken::SPACE:
				delete ast.addChild( parent, tokenizer.nextToken() );
				break;
			case SourceToken::ENDEXPRESSION:
				delete ast.addChild( parent, tokenizer.nextToken() );
				loop = false;
				break;
			case SourceToken::NEWLINE:
				delete ast.addChild( parent, tokenizer.nextToken() );
				break;
			default:
				stmt = ast.addChild( parent, new SourceToken( SourceToken::PARAMETER, new String() ) );
				String* param_type = parseParameter( ast, *stmt, tokenizer );
				{
					sb.append( *param_type );
					sb.append( "," );

//					if ( ! param_type->contentEquals( "" ) )
//					{
//						stmt->getElement().setType( SourceToken::PARAMETER );
//					}
				}

				delete param_type;
				countup( ast, *stmt );
				delete stmt;
			}
		}
		
		sb.removeLast();
		parent.getElement().setValue( sb.asString() );
	}

		static String*
		parseParameter( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
		{
			String* parameter_type = new String();
			{
				SourceToken* word1 = NULL;
				SourceToken* word2 = NULL;
			
				bool loop = true;
				while ( loop && tokenizer.hasMoreTokens() )
				{
					SourceToken::TokenType type = tokenizer.peekNextToken().getTokenType();
					//const char* value = tokenizer.peekNextToken().getValue().getChars();

					switch ( type )
					{
					case SourceToken::ENDEXPRESSION:
						loop = false;
						break;
					case SourceToken::SYMBOL:
						if ( tokenizer.peekNextToken().getValue().contentEquals( "," ) ) loop = false;
						delete ast.addChild( parent, tokenizer.nextToken() );
						break;
					case SourceToken::TYPE:
					case SourceToken::WORD:
						if ( NULL == word1 )
						{
							word1 = parseType( tokenizer );
							//word1 = tokenizer.nextToken();
							word1->setType( SourceToken::VARIABLE );
							delete ast.addChild( parent, word1 );
						}
						else
						{
							word2 = tokenizer.nextToken();
							word1->setType( SourceToken::TYPE );
							word2->setType( SourceToken::VARIABLE );
							delete ast.addChild( parent, word2 );

							//parent.getElement().setValue( generalizeType( word1->getValue() ) );
							parent.getElement().setValue( new String( word1->getValue() ) );
						}
						break;
					default:
						delete ast.addChild( parent, tokenizer.nextToken() );
					}
				}

				if ( 0 == parameter_type->getLength() && (NULL != word2) )
				{
					delete parameter_type;
					//parameter_type = (word1 != NULL) ? generalizeType( word1->getValue() ) : new String();
					parameter_type = (word1 != NULL) ? new String( word1->getValue() ) : new String();
				}
			}
			
			return parameter_type;
		}

static void handleMethodCall( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer, bool parsedNew )
{
	IPosition<SourceToken>* methodcall = ast.addChild( parent, new SourceToken( SourceToken::METHODCALL, new String() ) );
	{
		SourceToken* token = tokenizer.nextToken();
		methodcall->getElement().setValue( new String( token->getValue() ) );

		if ( parsedNew )
		{
			token->setType( SourceToken::CONSTRUCTOR );
		} else {
			token->setType( SourceToken::METHODCALL );
		}
		delete ast.addChild( *methodcall, token );
		
		parseTrailingWhitespace( ast, *methodcall, tokenizer );
		handleStartArguments( ast, *methodcall, tokenizer );
	}
	countup( ast, *methodcall );
	delete methodcall;
}

static void handleStartArguments( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	IPosition<SourceToken>* params = ast.addChild( parent, new SourceToken( SourceToken::ARGUMENTS, new String() ) );
	{
		delete ast.addChild( *params, tokenizer.nextToken() );
		parseTrailingWhitespace( ast, *params, tokenizer );
		parseArguments( ast, *params, tokenizer );
		parseTrailingWhitespace( ast, *params, tokenizer );
	}
	countup( ast, *params );
	delete params;
}

	static void parseArguments( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
	{
		StringBuffer sb;
	
		bool loop = true;
		while ( loop && tokenizer.hasMoreTokens() )
		{
			const SourceToken&      token = tokenizer.peekNextToken();
			SourceToken::TokenType  ttype = token.getTokenType();
			IPosition<SourceToken>* stmt  = NULL;
			
			switch ( ttype )
			{
			case SourceToken::ENDEXPRESSION:
				delete ast.addChild( parent, tokenizer.nextToken() );
				loop = false;
				break;

			case SourceToken::ENDBLOCK:
				loop = false;
				break;

			case SourceToken::BLANKLINE:
			case SourceToken::TAB:
			case SourceToken::SPACE:
			case SourceToken::NEWLINE:
				delete ast.addChild( parent, tokenizer.nextToken() );
				break;
			case SourceToken::SYMBOL:
				delete ast.addChild( parent, tokenizer.nextToken() );
			default:
				stmt = ast.addChild( parent, new SourceToken( SourceToken::ARGUMENT, new String() ) );
				parseStatement( ast, *stmt, tokenizer, true );
				countup( ast, *stmt );
				delete stmt;
			}
		}
		
		sb.removeLast();
		parent.getElement().setValue( sb.asString() );
	}

//		static void
//		parseArgument( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
//		{
//			bool loop = true;
//			while ( loop && tokenizer.hasMoreTokens() )
//			{
//				SourceToken::TokenType type = tokenizer.peekNextToken().getTokenType();
//
//				switch ( type )
//				{
//				case SourceToken::SYMBOL:
//					if ( tokenizer.peekNextToken().getValue().contentEquals( "," ) ) loop = false;
//					delete ast.addChild( parent, tokenizer.nextToken() );
//					break;
//				case SourceToken::ENDEXPRESSION:
//					loop = false;
//					break;
//				default:
//					delete ast.addChild( parent, tokenizer.nextToken() );
//				}
//			}
//		}

static void handleStartExpression( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	IPosition<SourceToken>* exp = ast.addChild( parent, new SourceToken( SourceToken::EXPRESSION, new String() ) );
	{
		delete ast.addChild( *exp, tokenizer.nextToken() );
		parseTrailingWhitespace( ast, *exp, tokenizer );
		parseExpression( ast, *exp, tokenizer );
		parseTrailingWhitespace( ast, *exp, tokenizer );
	}
	countup( ast, *exp );
	delete exp;
}

	static void parseExpression( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
	{
		StringBuffer sb;
	
		bool loop = true;
		while ( loop && tokenizer.hasMoreTokens() )
		{
			const SourceToken&      token = tokenizer.peekNextToken();
			SourceToken::TokenType  ttype = token.getTokenType();
			IPosition<SourceToken>* stmt  = NULL;
			
			switch ( ttype )
			{
			case SourceToken::ENDEXPRESSION:
				delete ast.addChild( parent, tokenizer.nextToken() );
				loop = false;
				break;

			case SourceToken::ENDBLOCK:
				loop = false;
				break;

			case SourceToken::BLANKLINE:
			case SourceToken::TAB:
			case SourceToken::SPACE:
			case SourceToken::NEWLINE:
				delete ast.addChild( parent, tokenizer.nextToken() );
				break;

			case SourceToken::SYMBOL:
				delete ast.addChild( parent, tokenizer.nextToken() );

			default:
				stmt = ast.addChild( parent, new SourceToken( SourceToken::CLAUSE, new String() ) );
				parseStatement( ast, *stmt, tokenizer, true );
				countup( ast, *stmt );
				delete stmt;
			}
		}
		
		sb.removeLast();
		parent.getElement().setValue( sb.asString() );
	}

static void handleStop( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	delete ast.addChild( parent, tokenizer.nextToken() );

	IPosition<SourceToken>* p1 = ast.hasParent( parent ) ? ast.parent( parent ) : NULL;
	if ( p1 )
	{
		IPosition<SourceToken>* p2 = ast.hasParent( *p1 ) ? ast.parent( *p1 ) : NULL;
		if ( p2 )
		{
			switch ( p2->getElement().getTokenType() )
			{
			case SourceToken::CLASS:
				parent.getElement().setType( SourceToken::MEMBER );
				break;
			default:
				break;
			}
		}
		delete p2;
	}
	delete p1;

	parseTrailingWhitespace( ast, parent, tokenizer );
}

static void handleKeyword( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	SourceToken* token = tokenizer.nextToken();

//	if ( token->getValue().contentEquals( "import" ) )
//	{
//		parent.getElement().setType( SourceToken::IMPORT );
//	}
//	else if ( token->getValue().contentEquals( "package" ) )
//	{
//		parent.getElement().setType( SourceToken::PACKAGE );
//	}
//	else if ( token->getValue().contentEquals( "class" ) )
//	{
//		parent.getElement().setType( SourceToken::CLASS );
//	}
//	else if ( token->getValue().contentEquals( "interface" ) )
//	{
//		parent.getElement().setType( SourceToken::INTERFACE );
//	}
	delete ast.addChild( parent, token );
}

static void parseTrailingWhitespace( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	bool loop = true;
	while ( loop && tokenizer.hasMoreTokens() )
	{
		const SourceToken&     token = tokenizer.peekNextToken();
		SourceToken::TokenType ttype = token.getTokenType();
		
		switch ( ttype )
		{
		case SourceToken::SPACE:
		case SourceToken::TAB:
		case SourceToken::WHITESPACE:
		case SourceToken::LINECOMMENT:
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;
		case SourceToken::NEWLINE:
			delete ast.addChild( parent, tokenizer.nextToken() );
			loop = false;
			break;
		default:
			loop = false;
		}
	}
}

void
AST::adjustOffsets( IPosition<SourceToken>& parent )
{
	long characters = 0;
	long lines      = 0;

	IPIterator<SourceToken>* it = this->ast->children( parent );
	while ( it->hasNext() )
	{
		IPosition<SourceToken>* p = it->next();
		{
			SourceToken& token = p->getElement();
			token.setOffset( lines );
			characters += token.getNrOfCharacters();
			lines      += token.getNrOfLines();
		}
		delete p;
	}
	delete it;

	parent.getElement().setNrOfCharacters( characters );
	parent.getElement().setNrOfLines( lines );

	if ( this->ast->hasParent( parent ) )
	{
		IPosition<SourceToken>* p = this->ast->parent( parent );
		{
			this->adjustOffsets( *p );
		}
		delete p;
	}
}

void
AST::recount( IPosition<SourceToken>& p )
{
	if ( this->ast->hasParent( p ) )
	{
		IPosition<SourceToken>* parent = this->ast->parent( p );
		{
			countup( *this->ast, *parent );
		}
		delete parent;
	}
}

void countup( ITree<SourceToken>& ast, IPosition<SourceToken>& parent )
{
	long characters = 0;
	long lines      = 0;

	IPIterator<SourceToken>* it = ast.children( parent );
	while ( it->hasNext() )
	{
		IPosition<SourceToken>* p = it->next();
		{
			SourceToken& token = p->getElement();
			if ( &token )
			{
				token.setOffset( lines );
				characters += token.getNrOfCharacters();
				lines      += token.getNrOfLines();
			}
			else
			{
				;
			}
		}
		delete p;
	}
	delete it;

	parent.getElement().setNrOfCharacters( characters );
	parent.getElement().setNrOfLines( lines );
}

AST*
AST::copySubtree( IPosition<SourceToken>& p )
{
	AST* ast = new AST();
	delete ast->ast;
	ast->ast = this->ast->copyAsTree( p );
	
	return ast;
}

AST*
AST::copySubtree( const IPosition<SourceToken>& p ) const
{
	AST* ast = new AST();
	delete ast->ast;
	ast->ast = this->ast->copyAsTree( p );
	
	return ast;
}

//void
//AST::replaceSubtree( IPosition<SourceToken>& p, const AST& ast )
//{
//	ITree<SourceToken>* method_tree = ast.ast->copyAsTree( ast.ast->getRoot() );
//	{
//		this->ast->swapSubtrees( p, *method_tree, method_tree->getRoot() );
//	}
//	delete method_tree;
//	
//	this->adjustOffsets( p );
//}

bool
AST::isValid() const
{
	return this->valid;
}

