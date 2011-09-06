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

using namespace astral::ast;
using namespace astral::tokenizer;
using namespace openxds::io;
using namespace openxds::io::exceptions;
using namespace openxds::base;
using namespace openxds::adt;
using namespace openxds::adt::std;

static void                parseAll( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void          parseStatement( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void          parseStatementX( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void           handleKeyword( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void              handleStop( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void        handleStartBlock( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void        handleMethodCall( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void   handleStartParameters( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static String*       parseParameter( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void              parseBlock( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void         parseParameters( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );
static void parseTrailingWhitespace( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );

static SourceToken* parseType( SourceTokenizer& tokenizer );
static void pushbackTokens( SourceTokenizer& tokenizer, Sequence<SourceToken>& tokens );

static bool           isMemberLevel( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer );

AST::AST( const char* location )
{
	this->location = new String( location );
	this->ast      = new Tree<SourceToken>();

	delete ast->addRoot( new SourceToken( SourceToken::FILE, this->location->asString() ) );
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
AST::parseFile()
{
	try
	{
		SourceTokenizer* tokenizer = new JavaTokenizer( *this->location );
		{
			IPosition<SourceToken>* root = this->ast->root();
			{
				parseAll( *this->ast, *root, *tokenizer );
			}
			delete root;
		}
		delete tokenizer;
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

static void
parseAll( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	while ( tokenizer.hasMoreTokens() )
	{
		SourceToken::TokenType type = tokenizer.peekNextToken().getTokenType();
		switch ( type )
		{
		case SourceToken::NEWLINE:
		case SourceToken::BLANKLINE:
		case SourceToken::COMMENT:
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;
		default:
			SourceToken* token = new SourceToken( SourceToken::STATEMENT, new String() );
			IPosition<SourceToken>* p = ast.addChild( parent, token );
			{
				parseStatement( ast, *p, tokenizer );
			}
			delete p;
		}
	}
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

enum Looksie
{
	UNKNOWN,
	BLOCK,
	BLOCKSTATEMENT,
	LVAL,
	RVAL
};

static void
parseStatement( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	SourceToken::TokenType looksie = tokenizer.sneakyPeek();
	parent.getElement().setType( looksie );

	bool parsed_type   = false;
	//bool parsed_params = false;
	int  words         = 0;

	SourceToken* st_type = NULL;
	
	bool loop = true;
	while ( loop && tokenizer.hasMoreTokens() )
	{
		SourceToken::TokenType type = tokenizer.peekNextToken().getTokenType();

		switch ( type )
		{
		case SourceToken::STOP:
			handleStop( ast, parent, tokenizer );
			loop = false;
			break;
		case SourceToken::STARTBLOCK:
			handleStartBlock( ast, parent, tokenizer );
			loop = false;
			break;
		case SourceToken::KEYWORD:
			handleKeyword( ast, parent, tokenizer );
			break;
		case SourceToken::STARTEXPRESSION:
			switch ( looksie )
			{
			case SourceToken::METHOD:
				handleStartParameters( ast, parent, tokenizer );
				break;
			default:
				delete ast.addChild( parent, tokenizer.nextToken() );
			}
			break;
		case SourceToken::TYPE:
		case SourceToken::WORD:
			if ( !parsed_type )
			{
				switch ( looksie )
				{
				case SourceToken::METHOD:
				case SourceToken::DECLARATION:
					{
						st_type = parseType( tokenizer );
						st_type->setType( SourceToken::METHODNAME );
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
				default:
					if ( isMethodCall( tokenizer ) )
					{
						handleMethodCall( ast, parent, tokenizer );
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
				case SourceToken::DECLARATION:
					{
						SourceToken* token = tokenizer.nextToken();
						token->setType( SourceToken::NAME );
						if ( 1 == words )
						{
							if ( st_type ) st_type->setType( SourceToken::TYPE );
							token->setType( SourceToken::VARIABLE );
						}
						delete ast.addChild( parent, token );
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
						}
						delete ast.addChild( parent, token );
					}
					break;
				default:
					if ( isMethodCall( tokenizer ) )
					{
						handleMethodCall( ast, parent, tokenizer );
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
}

static void
parseStatementX( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	bool keyword_used    = false;
	bool selector_used   = false;
	bool assignment_used = false;
	bool package_used    = false;
	bool import_used     = false;
	bool class_used      = false;
	bool extends_used    = false;
	//bool implements_used = false;
	bool expression_used = false;

	SourceToken* word1 = NULL;
	SourceToken* word2 = NULL;
	SourceToken* wordX = NULL;

	bool loop = true;
	while ( loop && tokenizer.hasMoreTokens() )
	{
		SourceToken::TokenType type = tokenizer.peekNextToken().getTokenType();
		//const char* value = tokenizer.peekNextToken().getValue().getChars();
		//const char* stype = tokenizer.peekNextToken().getTokenTypeString().getChars();
		//fprintf( stdout, "%s (%s)\n", value, stype );

		switch ( type )
		{
		case SourceToken::SPACE:
		case SourceToken::TAB:
		case SourceToken::WHITESPACE:
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;
		case SourceToken::NUMBER:
		case SourceToken::QUOTE:
		case SourceToken::DOUBLEQUOTE:
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;
		case SourceToken::SYMBOL:
		case SourceToken::NEWLINE:
		case SourceToken::MODIFIER:
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;
		case SourceToken::ASSIGNMENTOP:
			assignment_used = true;
			// Intentional fall-through
		case SourceToken::PREFIXOP:
		case SourceToken::INFIXOP:
		case SourceToken::PREPOSTFIXOP:
		case SourceToken::PREINFIXOP:
			delete ast.addChild( parent, tokenizer.nextToken() );
			break;
		case SourceToken::SELECTOR:
			delete ast.addChild( parent, tokenizer.nextToken() );
			selector_used = true;
			break;
		case SourceToken::KEYWORD:
			package_used |= tokenizer.peekNextToken().getValue().contentEquals( "package"    );
			import_used  |= tokenizer.peekNextToken().getValue().contentEquals( "import"     );
			class_used   |= tokenizer.peekNextToken().getValue().contentEquals( "class"      );
			class_used   |= tokenizer.peekNextToken().getValue().contentEquals( "interface"  );
			extends_used |= tokenizer.peekNextToken().getValue().contentEquals( "extends"    );
			extends_used |= tokenizer.peekNextToken().getValue().contentEquals( "implements" );
			handleKeyword( ast, parent, tokenizer );
			keyword_used = true;
			break;
		case SourceToken::LINECOMMENT:
			delete ast.addChild( parent, tokenizer.nextToken() );
			parseTrailingWhitespace( ast, parent, tokenizer );
			break;
		case SourceToken::COMMENT:
			delete ast.addChild( parent, tokenizer.nextToken() );
			parseTrailingWhitespace( ast, parent, tokenizer );
			loop = false;
			break;
		case SourceToken::JAVADOC:
			delete ast.addChild( parent, tokenizer.nextToken() );
			parseTrailingWhitespace( ast, parent, tokenizer );
			break;
//		case SourceToken::ENDEXPRESSION:
//			//handleEndExpression( ast, parent, tokenizer );
//			break;
//		case SourceToken::NEWLINE:
//			//handleNewline( ast, parent, tokenizer );
//			break;
		case SourceToken::STARTBLOCK:
			if ( !class_used )
			{
				if      ( word2 ) word2->setType( SourceToken::METHODNAME );
				else if ( word1 ) word1->setType( SourceToken::METHODNAME );
			}
		
			handleStartBlock( ast, parent, tokenizer );

			if ( !keyword_used )
			{
				parent.getElement().setType( SourceToken::METHOD );
			}
			loop = false;
			break;
		case SourceToken::ENDBLOCK:
			loop = false;
			break;
		case SourceToken::STOP:
			handleStop( ast, parent, tokenizer );
			loop = false;
			break;
		case SourceToken::ENDEXPRESSION:
			delete ast.addChild( parent, tokenizer.nextToken() );
			//handleEndExpression( ast, parent, tokenizer );
			break;
		case SourceToken::STARTEXPRESSION:
			if ( !assignment_used )
			{
				if ( word2 ) parent.getElement().setType( SourceToken::IMETHOD );
				handleStartParameters( ast, parent, tokenizer );
			} else {
				delete ast.addChild( parent, tokenizer.nextToken() );
			}
			expression_used = true;
			break;
		default:
			bool is_method_call = isMethodCall( tokenizer );
			
			if ( keyword_used )
			{
				wordX = parseType( tokenizer ); //tokenizer.nextToken();
				wordX->setType( SourceToken::NAME );
				delete ast.addChild( parent, wordX );

				if ( class_used && !word1 )
				{
					word1 = wordX;
					word1->setType( SourceToken::CLASSNAME );
				}
			}
			else if ( expression_used )
			{
				wordX = tokenizer.nextToken();
				wordX->setType( SourceToken::NAME );
				if ( is_method_call )
				{
					handleMethodCall( ast, parent, tokenizer );
				}
				else
				{
					delete ast.addChild( parent, wordX );
				}
			}
			else if ( NULL == word1 )
			{
				word1 = parseType( tokenizer );
				wordX = word1;
				word1->setType( SourceToken::NAME );
				if ( is_method_call )
				{
					handleMethodCall( ast, parent, tokenizer );
				}
				else if ( class_used )
				{
					word1->setType( SourceToken::CLASSNAME );
				}
				else
				{
					delete ast.addChild( parent, word1 );
				}
			}
			else if ( !assignment_used && !selector_used && !class_used && !extends_used )
			{
				word1->setType( SourceToken::TYPE );

				word2 = tokenizer.nextToken();
				wordX = word2;
				if ( extends_used )
				{
					word2->setType( SourceToken::NAME );
				} else {
					parent.getElement().setType( SourceToken::DECLARATION );
					word2->setType( SourceToken::VARIABLE );
				}
				delete ast.addChild( parent, word2 );
			}
			else
			{
				wordX = tokenizer.nextToken();
				wordX->setType( SourceToken::NAME );
				if ( is_method_call )
				{
					handleMethodCall( ast, parent, tokenizer );
				}
				else
				{
					delete ast.addChild( parent, wordX );
				}
			}
			
//			if ( !assignment_used && !expression_used )
//			{
//				if ( isMemberLevel( ast, parent, tokenizer ) )
//				{
//					if ( tokenizer.hasMoreTokens() && (SourceToken::STARTEXPRESSION == tokenizer.peekNextToken().getTokenType()) )
//					{
//						wordX->setType( SourceToken::METHODNAME );
//					} else {
//						wordX->setType( SourceToken::MEMBERNAME );
//					}
//				}
//			}
		}
	}
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

static void handleStartBlock( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	IPosition<SourceToken>* block = ast.addChild( parent, new SourceToken( SourceToken::BLOCK, new String() ) );
	{
		delete ast.addChild( *block, tokenizer.nextToken() );
		parseTrailingWhitespace( ast, *block, tokenizer );
		parseBlock( ast, *block, tokenizer );
		parseTrailingWhitespace( ast, *block, tokenizer );
	}
	delete block;
}

	static void parseBlock( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
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
			case SourceToken::COMMENT:
			case SourceToken::NEWLINE:
			case SourceToken::TAB:
				delete ast.addChild( parent, tokenizer.nextToken() );
				break;
			case SourceToken::ENDBLOCK:
				delete ast.addChild( parent, tokenizer.nextToken() );
				loop = false;
				break;
			default:
				stmt = ast.addChild( parent, new SourceToken( SourceToken::STATEMENT, new String() ) );
				parseStatement( ast, *stmt, tokenizer );
				delete stmt;
			}
		}
	}

static void handleMethodCall( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	IPosition<SourceToken>* methodcall = ast.addChild( parent, new SourceToken( SourceToken::METHODCALL, new String() ) );
	{
		SourceToken* token = tokenizer.nextToken();
		token->setType( SourceToken::METHODCALL );
		delete ast.addChild( *methodcall, token );
		parseTrailingWhitespace( ast, *methodcall, tokenizer );
		handleStartParameters( ast, *methodcall, tokenizer );
	}
	delete methodcall;
}

static void handleStartParameters( ITree<SourceToken>& ast, IPosition<SourceToken>& parent, SourceTokenizer& tokenizer )
{
	IPosition<SourceToken>* params = ast.addChild( parent, new SourceToken( SourceToken::PARAMETERS, new String() ) );
	{
		delete ast.addChild( *params, tokenizer.nextToken() );
		parseTrailingWhitespace( ast, *params, tokenizer );
		parseParameters( ast, *params, tokenizer );
		parseTrailingWhitespace( ast, *params, tokenizer );
	}
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
				sb.append( *param_type );
				sb.append( "," );
				delete param_type;
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
						}
						break;
					case SourceToken::NUMBER:
						delete parameter_type;
						parameter_type = new String( "VALUE" );
						delete ast.addChild( parent, tokenizer.nextToken() );
						break;
					case SourceToken::DOUBLEQUOTE:
						delete parameter_type;
						parameter_type = new String( "QUOTE" );
						delete ast.addChild( parent, tokenizer.nextToken() );
						break;
					case SourceToken::QUOTE:
						delete parameter_type;
						parameter_type = new String( "CHAR" );
						delete ast.addChild( parent, tokenizer.nextToken() );
						break;
					default:
						delete ast.addChild( parent, tokenizer.nextToken() );
					}
				}

				if ( 0 == parameter_type->getLength() )
				{
					delete parameter_type;
					parameter_type = (word1 != NULL) ? word1->getValue().asString() : new String();
				}
			}
			
			return parameter_type;
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
