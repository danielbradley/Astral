#include "astral/AdvancedHTMLPrintTour.h"
#include "astral/CodeBase.h"
#include "astral/CompilationUnit.h"
#include "astral/MemberSignature.h"
#include "astral/MethodSignature.h"
#include "astral/VariableScopes.h"

#include <openxds.io/PrintWriter.h>
#include <openxds.adt/IDictionary.h>
#include <openxds.adt/IEntry.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ISequence.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt.std/GeneralTour.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/Character.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.base/StringTokenizer.h>

using namespace astral;
using namespace astral::tokenizer;
using namespace openxds;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::exceptions;
using namespace openxds::io;

static openxds::base::String* extractJavadocWithoutFirstTab( const char* value );

AdvancedHTMLPrintTour::AdvancedHTMLPrintTour(
	      ITree<SourceToken>& tree,
	const    CompilationUnit& aCu,
    const           CodeBase& codebase,
	             PrintWriter& aWriter
)
: openxds::adt::std::GeneralTour<SourceToken>( tree ), cu( aCu ), codebase( codebase), writer( aWriter )
{
	this->depth = -1;
	
	this->scopes = new VariableScopes( this->tree );
	this->lastType = new String();
	this->methodCallTypes = new Sequence<String>();

}

AdvancedHTMLPrintTour::~AdvancedHTMLPrintTour()
{
	delete this->scopes;
	delete this->lastType;
	delete this->methodCallTypes;
}
	
void
AdvancedHTMLPrintTour::visitPreorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
{
	astral::tokenizer::SourceToken& token = p.getElement();
	const char* type = token.getTokenTypeString().getChars();
	
	switch ( token.getTokenType() )
	{
	case SourceToken::METHOD:
		this->scopes->reset();
		this->scopes->addScope();
		this->scopes->addMethodParameters( p );

		writer.print( openxds::base::FormattedString( "<div class='%s' name='%s'>", type, token.getValue().getChars() ) );
		writer.print( openxds::base::FormattedString( "<a name='%s'></a>", token.getValue().getChars() ) );
		break;
		
	case astral::tokenizer::SourceToken::PARAMETERS:
	case astral::tokenizer::SourceToken::ARGUMENTS:
		writer.print( openxds::base::FormattedString( "<div class='%s inline' name='%s'>", type, token.getValue().getChars() ) );
		break;

	case astral::tokenizer::SourceToken::METHODCALL:
	case astral::tokenizer::SourceToken::CONSTRUCTOR:
		{
//			String* parameters = this->cu.resolveMethodCallArgumentTypes( codebase, this->tree, p, *scopes );
//			writer.print( openxds::base::FormattedString( "<div class='%s inline' name='%s' title='%s'>", type, token.getValue().getChars(), parameters->getChars() ) );
//			token.setValue( parameters );
			writer.print( openxds::base::FormattedString( "<div class='%s inline' name='%s'>", type, token.getValue().getChars() ) );
		}
		break;

	case astral::tokenizer::SourceToken::PARAMETER:
	case astral::tokenizer::SourceToken::ARGUMENT:
		writer.print( openxds::base::FormattedString( "<div class='%s inline' name='%s'>", type, token.getValue().getChars() ) );
		break;

	case SourceToken::BLOCK:
		this->scopes->addScope();
		writer.print( openxds::base::FormattedString( "<div class='%s'>", type ) );
		break;

	case SourceToken::DECLARATION:
		this->scopes->addVariableDeclaration( p );
		writer.print( openxds::base::FormattedString( "<div class='%s'>", type ) );
		break;

	default:
		writer.print( openxds::base::FormattedString( "<div class='%s'>", type ) );
		break;
	}
	
	this->depth++;
}

void
AdvancedHTMLPrintTour::visitPostorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
{
	astral::tokenizer::SourceToken& token = p.getElement();

	this->depth--;
	
	switch ( token.getTokenType() )
	{
	case SourceToken::METHOD:
		this->scopes->reset();
		writer.print( "</div>" );
		break;
	case SourceToken::BLOCK:
		this->scopes->removeInnermostScope();
		writer.print( "</div>" );
		break;
	case SourceToken::METHODCALL:
	case SourceToken::CONSTRUCTOR:
		delete this->lastType;
		this->lastType = this->methodCallTypes->removeLast();
		writer.print( "</div>" );
		break;
	default:
		writer.print( "</div>" );
	}
}
	
void
AdvancedHTMLPrintTour::visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
{
	bool constructor = false;

	astral::tokenizer::SourceToken& token = p.getElement();
	const char* value = token.getValue().getChars();
	const char* ttype = token.getTokenTypeString().getChars();
	openxds::base::String* str = NULL;
	
	switch ( token.getTokenType() )
	{
	case SourceToken::WORD:
	case SourceToken::SELECTOR:
	case SourceToken::METHODCALL:
		break;
	default:
		if ( ! this->lastType->contentEquals( "" ) )
		{
			delete this->lastType;
			this->lastType = new String();
		}
	}

	switch ( token.getTokenType() )
	{
	case SourceToken::SPACE:
	case SourceToken::WHITESPACE:
		writer.print( " " );
		break;
	case SourceToken::TAB:
		writer.print( openxds::base::FormattedString( "<span class='%s'></span>", ttype ) );
		break;
	case SourceToken::BLANKLINE:
	case SourceToken::NEWLINE:
		writer.print( openxds::base::FormattedString( "<span class='%s'><br></span>\n", ttype ) );
		break;
	case SourceToken::STARTBLOCK:
		writer.print( "<span>{</span>" );
		break;
	case SourceToken::ENDBLOCK:
		writer.print( "<span>}</span>" );
		break;
	case SourceToken::STARTEXPRESSION:
		writer.print( "<span class='STARTEXPRESSION'>(</span>" );
		break;
	case SourceToken::ENDEXPRESSION:
		writer.print( "<span class='ENDEXPRESSION'>)</span>" );
		break;
	case SourceToken::COMMENT:
	case SourceToken::JAVADOC:
		str = extractJavadocWithoutFirstTab( value );
		writer.print( openxds::base::FormattedString( "<pre class='%s'>%s</pre>", ttype, str->getChars() ) );
		delete str;
		break;
	case SourceToken::CONSTRUCTOR:
		constructor = true;
	case SourceToken::KEYWORD:
	case SourceToken::METHODCALL:
	case SourceToken::NAME:
		{
			String this_keyword( "this" );
			openxds::base::String* fq_type = NULL;

			bool is_keyword     = (SourceToken::KEYWORD    == token.getTokenType());
			bool is_method_call = (SourceToken::METHODCALL == token.getTokenType()) || (SourceToken::CONSTRUCTOR == token.getTokenType());

			if ( is_keyword )
			{
				if ( this_keyword.contentEquals( value ) )
				{
					fq_type = this->cu.resolveFQTypeOfName( value, *this->scopes );
				} else {
					fq_type = new String();
				}
			}
			else if ( ! is_method_call )
			{
				//	If not a SourceToken::METHOD_CALL determines the fully qualified type of the SourceToken::NAME.
				if ( this->lastType->contentEquals( "" ) )
				{
					fq_type = this->cu.resolveFQTypeOfName( value, *this->scopes );
				}
				else
				{
					MemberSignature* member_signature = this->codebase.completeMemberSignature( this->lastType->getChars(), value );
					{
						fq_type = this->cu.resolveFQTypeOfType( member_signature->getType().getChars() );
					}
					delete member_signature;
				}
			}
			else if ( is_method_call )
			{
				IPosition<SourceToken>* parent = this->tree.parent( p );
				{
					if ( p.getElement().getValue().contentEquals( "createReaderFromFile" ) )
					{
						fprintf( stderr, "AHPT: Break\n" );
					}

					String* parameters  = this->cu.resolveMethodCallArgumentTypes( codebase, this->tree, *parent, *scopes );
					{
						String* invocation_class = this->lastType->contentEquals( "" ) ? new String( this->cu.getFQName() ) : new String( *this->lastType );
						{
							if ( constructor )
							{
								delete invocation_class;
								invocation_class = this->cu.resolveFQTypeOfType( value );
								//fprintf( stderr, "Astral::visitExternal: Constructor: %s.%s\n", invocation_class->getChars(), value );
							}
							MethodSignature* method_signature = this->codebase.completeMethodSignature( invocation_class->getChars(), value, parameters->getChars() );
							{
								this->lastType = this->cu.resolveFQTypeOfType( method_signature->getReturnType().getChars() );
								this->methodCallTypes->addLast( this->lastType->asString() );
								fq_type = new String( method_signature->getFQClass() );
							}
							delete method_signature;

							if ( 0 < fq_type->getLength() )
							{
								FormattedString str( "%s.html#%s(%s)", fq_type->getChars(), value, parameters->getChars() );
								writer.print( openxds::base::FormattedString( "<a class='%s' href='%s'>%s</a>", ttype, str.getChars(), value ) );
							} else {
								writer.print( openxds::base::FormattedString( "<a class='%s'>%s</a>", ttype, value ) );
							}
						}
						delete invocation_class;
					}
					delete parameters;
				}
				delete parent;
			}

			{
				if ( is_method_call )
				{
				}
				else if ( this_keyword.contentEquals( value ) )
				{
					writer.print( openxds::base::FormattedString( "<span class='%s' title='%s'>%s</span>", ttype, fq_type->getChars(), value ) );
				}
				else if ( fq_type->contentEquals( "" ) )
				{
					writer.print( openxds::base::FormattedString( "<span class='%s'>%s</span>", ttype, value ) );
				}
				else if ( this->codebase.containsType( fq_type->getChars() ) )
				{
					writer.print( openxds::base::FormattedString( "<a class='%s' href='%s.html'>%s</a>", ttype, fq_type->getChars(), value ) );
				}
				else
				{
					writer.print( openxds::base::FormattedString( "<span class='%s'>%s</span>", ttype, value ) );
				}
			}
			delete this->lastType;
			this->lastType = fq_type;
		}
		break;
	case SourceToken::METHODNAME:
		writer.print( openxds::base::FormattedString( "<span class='%s'>%s</span>", ttype, value ) );
		break;
	default:
		writer.print( openxds::base::FormattedString( "<span class='%s'>%s</span>", ttype, value ) );
	}
}

static openxds::base::String* extractJavadocWithoutFirstTab( const char* value )
{
	openxds::base::StringBuffer sb;
	bool add = false;

	long len = strlen( value );
	for ( long i=0; i < len; i++ )
	{
		switch( value[i] )
		{
		case '\t':
			if ( add ) sb.append( value[i] );
			break;
		case '\n':
			sb.append( value[i] );
			add = false;
			break;
		default:
			sb.append( value[i] );
			add = true;
		}
	}
	
	return sb.asString();
}


