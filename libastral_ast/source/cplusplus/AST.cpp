#include "astral.ast/AST.h"
#include "astral.tokenizer/SourceToken.h"
#include "astral.tokenizer/IxTokenizer.h"
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
#include <openxds.exceptions/NoSuchElementException.h>
#include <openxds.io/InputStream.h>
#include <openxds.io/InputStreamReader.h>
#include <openxds.io/IOBuffer.h>

using namespace astral::ast;
using namespace astral::tokenizer;
using namespace openxds::io;
using namespace openxds::io::exceptions;
using namespace openxds::base;
using namespace openxds::exceptions;
using namespace openxds::adt;
using namespace openxds::adt::std;

static bool                parseAll( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static bool          parseStatement( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer, bool parseArg );
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

static void pushbackTokens( SourceTokenizer& tokenizer, Sequence<SourceToken>& tokens );

static bool           isMemberLevel( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void                 countup( ITree<SourceToken>& ast, IPosition<SourceToken>& parent );

AST::AST()
{
	this->ast      = new Tree<SourceToken>();
	this->location = new String();
	this->valid    = true;
	this->indent   = new Sequence<SourceToken>();
}

AST::~AST()
{
	delete this->location;
	delete this->ast;
}

void
AST::clearTree()
{
	delete this->ast; this->ast = new Tree<SourceToken>();
}


openxds::adt::ITree<SourceToken>&
AST::getTree() const
{
	return *this->ast;
}

void
AST::parseString( const String& content )
{
	this->clearTree();

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
	this->clearTree();

	delete this->location;
	this->location = new String( location );

	delete ast->addRoot( new SourceToken( SourceToken::FILE, this->location->asString() ) );

	SourceTokenizer* tokenizer = NULL;
	{
		String file_path( location );
	
		if ( file_path.endsWith( ".java" ) )
		{
			tokenizer = new JavaTokenizer( *this->location );
		}
		else
		if ( file_path.endsWith( ".ix" ) )
		{
			tokenizer = new IxTokenizer( *this->location );
		}
	
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

	bool parsed_new       = false;
	bool parsed_classname = false;

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
				st_type = tokenizer.nextToken();
				parent.getElement().setValue( new String( st_type->getValue() ) );
				delete ast.addChild( parent, st_type );
				break;
			
			default:
				st_type = tokenizer.nextToken();
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
						SourceToken* parsed = tokenizer.nextToken();
						parsed->setType( SourceToken::NAME );
						delete ast.addChild( parent, parsed );
					}
					break;

				case SourceToken::CLASS:
					{
						if ( ! parsed_classname )
						{
							SourceToken* parsed = tokenizer.nextToken();
							parsed->setType( SourceToken::CLASSNAME );
							delete ast.addChild( parent, parsed );
							parsed_classname = true;
						}
						else
						{
							SourceToken* parsed = tokenizer.nextToken();
							parsed->setType( SourceToken::NAME );
							delete ast.addChild( parent, parsed );
						}
					}
					break;

				case SourceToken::METHOD:
					{
						SourceToken* token = tokenizer.nextToken();
						token->setType( SourceToken::METHODNAME );
						parent.getElement().setValue( new String( token->getValue() ) );
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
			case SourceToken::NEWLINE:
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
							word1 = tokenizer.nextToken();
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

	if ( SourceToken::DECLARATION == parent.getElement().getTokenType() )
	{
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
	}

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

void
AST::storeIndent()
{
	IPosition<SourceToken>* p = this->findFirstMethodPosition();
	{
		this->addWhitespaceTokensToIndent();
		this->matchAndRemoveIndentFromLines( *p, false );
	}
	delete p;
}

void
AST::addWhitespaceTokensToIndent()
{
	IPosition<SourceToken>* p = this->findFirstMethodPosition();
	{
		IPIterator<SourceToken>* it = this->ast->children( *p );
		{
			bool loop = true; // until first non-space/tab element.
			while ( loop && it->hasNext() )
			{
				IPosition<SourceToken>* p = it->next();
				SourceToken& token = p->getElement();
				switch ( token.getTokenType() )
				{
				case SourceToken::TAB:
				case SourceToken::SPACE:
				case SourceToken::LINECOMMENT:
					{
						SourceToken* t = this->ast->remove( p );
						this->indent->addLast( t );
					}
					break;

				default:
					loop = false;
					delete p;
				}
			}
		}
		delete it;
	}
	delete p;
}

bool
AST::matchAndRemoveIndentFromLines( IPosition<SourceToken>& p, bool extractMatch )
{
	bool extract_match = extractMatch;
	long token_index   = 0;
	long indent_len    = this->indent->size();

	IPIterator<SourceToken>* it = this->ast->children( p );
	while ( it->hasNext() )
	{
		IPosition<SourceToken>* p = it->next();
		{
			SourceToken&           token  = p->getElement();
			SourceToken::TokenType tType  = p->getElement().getTokenType();
			const String&          tValue = p->getElement().getValue();
			
			//fprintf( stdout, "%20s - '%s'\n", token.getValue().getChars(), token.getTokenTypeString().getChars() );

			if ( this->ast->isInternal( *p ) )
			{
				extract_match = this->matchAndRemoveIndentFromLines( *p, extract_match );
				token_index   = 0;
			}
			else
			{
				switch ( tType )
				{
				case SourceToken::NEWLINE:
				case SourceToken::BLANKLINE:
				case SourceToken::LINECOMMENT:
					extract_match = true;
					break;
					
				default:
					if ( extract_match )
					{
						try
						{
							SourceToken::TokenType iType  = this->indent->get( token_index ).getTokenType();
							const String&          iValue = this->indent->get( token_index ).getValue();
							
							if ( (iType == tType) && iValue.contentEquals( tValue ) )
							{
								this->ast->remove( p ); p = null;
								token_index++;
								
								if ( token_index == indent_len )
								{
									extract_match = false;
									token_index   = 0;
								}
							}
							else
							{
								extract_match = false;
								token_index   = 0;
							}
						}
						catch ( NoSuchElementException* ex )
						{
							delete ex;
							extract_match = false;
							token_index   = 0;
						}
					}
				}
			}
		}
		delete p;
	}
	return extract_match;
}

String*
AST::getIndent() const
{
	StringBuffer sb;
	long len = this->indent->size();
	for ( long i=0; i < len; i++ )
	{
		sb.append( this->indent->get(i).getValue() );
	}
	return sb.asString();
};


//void
//AST::unstoreIndent()
//{
//	IPosition<SourceToken>* p = this->findFirstMethodPosition();
//	{
//		long len = this->indent->size();
//		for ( long i=0; i < len; i++ )
//		{
//			const SourceToken& token = this->indent->get(i);
//			this->ast->insertChildAt( *p, new SourceToken( token ), i );
//		}
//		this->addIndentAfterNewlines( *p, false );
//	}
//	delete p;
//	
//	while ( ! this->indent->isEmpty() ) delete this->indent->removeFirst();
//}
//
//static void insertIndentAt( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, ISequence<SourceToken>& indent, long index )
//{
//	IIterator<SourceToken>* it = indent.elements();
//	while ( it->hasNext() )
//	{
//		SourceToken& token = it->next();
//		ast.insertChildAt( parent, new SourceToken( token ), index++ );
//	}
//	delete it;
//}
//
//bool
//AST::addIndentAfterNewlines( IPosition<SourceToken>& parent, bool insertIndent )
//{
//	bool insert_indent = insertIndent;
//
//	if ( insertIndent && this->ast->nrChildren( parent ) )
//	{
//		IPosition<SourceToken>* p = this->ast->child( parent, 0 );
//		if ( this->ast->isExternal( *p ) )
//		{
//			insertIndentAt( *this->ast, parent, *this->indent, 0 );
//			insert_indent = false;
//		}
//	}
//
//	long len = this->ast->nrChildren( parent );
//	for ( long i=0; i < len; i++ )
//	{
//		try
//		{
//			IPosition<SourceToken>* p = this->ast->child( parent, i );
//			{
//				SourceToken& token = p->getElement();
//
//				fprintf( stdout, "%20s - '%s'\n", token.getValue().getChars(), token.getTokenTypeString().getChars() );
//
//				if ( this->ast->isInternal( *p ) )
//				{
//					insert_indent = this->addIndentAfterNewlines( *p, insert_indent );
//				}
//				else
//				{
//					SourceToken::TokenType tType = p->getElement().getTokenType();
//					switch ( tType )
//					{
//					case SourceToken::NEWLINE:
//					case SourceToken::BLANKLINE:
//					case SourceToken::LINECOMMENT:
//						insert_indent = true;
//						break;
//					
//					default:
//						break;
//					}
//					
//					if ( insert_indent && ( i < (len-1) ) )
//					{
//						insertIndentAt( *this->ast, *p, *this->indent, i+1 );   //  |\n | \t| }|
//						insert_indent = false;                                  //    i  i+1| i
//						
//						i   += this->indent->size();
//						len += this->indent->size();
//					}
//				}
//			}
//			delete p;
//		}
//		catch ( NoSuchElementException* ex )
//		{
//			delete ex;
//		}
//	}
//	return insert_indent;
//}

IPosition<SourceToken>*
AST::findFirstMethodPosition()
{
	IPosition<SourceToken>& r = this->ast->getRoot();

	switch ( r.getElement().getTokenType() )
	{
	case SourceToken::OTHER:
		return this->ast->child( r, 0 );
		break;

	case SourceToken::METHOD:
		return this->ast->root();
		break;

	default:
		return null;
	}
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

