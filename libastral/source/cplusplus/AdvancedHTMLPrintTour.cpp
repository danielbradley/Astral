#include "astral/AdvancedHTMLPrintTour.h"
#include "astral/CodeBase.h"
#include "astral/CompilationUnit.h"
#include "astral/InvocationClass.h"
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
	
	this->scopes          = new VariableScopes( this->tree );
	this->invocationClass = new InvocationClass();
}

AdvancedHTMLPrintTour::~AdvancedHTMLPrintTour()
{
	delete this->scopes;
	delete this->invocationClass;
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
		break;
		
	case SourceToken::BLOCK:
		this->scopes->addScope();
		break;

	case SourceToken::DECLARATION:
		this->scopes->addVariableDeclaration( p );
		break;

	case SourceToken::ARGUMENTS:
		this->invocationClass->pushReturnType();

	default:
		break;
	}
	
	switch ( token.getTokenType() )
	{
	case SourceToken::METHOD:
		writer.print( openxds::base::FormattedString( "<div class='%s' name='%s'>", type, token.getValue().getChars() ) );
		writer.print( openxds::base::FormattedString( "<a name='%s'></a>", token.getValue().getChars() ) );
		break;
		
	case astral::tokenizer::SourceToken::PARAMETERS:
	case astral::tokenizer::SourceToken::ARGUMENTS:
	case astral::tokenizer::SourceToken::METHODCALL:
	case astral::tokenizer::SourceToken::CONSTRUCTOR:
	case astral::tokenizer::SourceToken::PARAMETER:
	case astral::tokenizer::SourceToken::ARGUMENT:
		writer.print( openxds::base::FormattedString( "<div class='%s inline' name='%s'>", type, token.getValue().getChars() ) );
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
		break;

	case SourceToken::BLOCK:
		this->scopes->removeInnermostScope();
		break;

	case SourceToken::ARGUMENTS:
		this->invocationClass->popReturnType();
		break;

	default:
		break;
	}
	writer.print( "</div>" );
}
	
void
AdvancedHTMLPrintTour::visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
{
	bool constructor = false;

	SourceToken& token = p.getElement();
	const char*  value = token.getValue().getChars();
	const char*  ttype = token.getTokenTypeString().getChars();
	String*      str   = NULL;
	
	switch ( token.getTokenType() )
	{
	case SourceToken::WORD:
	case SourceToken::NAME:
	case SourceToken::SELECTOR:
	case SourceToken::METHODCALL:
	case SourceToken::CONSTRUCTOR:
	case SourceToken::KEYWORD:
		break;

	default:
		this->invocationClass->clearTypes();
	}

	switch ( token.getTokenType() )
	{
	case SourceToken::SPACE:
	case SourceToken::WHITESPACE:
		writer.print( "&nbsp;" );
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

	case SourceToken::KEYWORD:
		{
			String this_keyword( "this" );
			if ( this_keyword.contentEquals( value ) )
			{
				this->invocationClass->setLastType( this->cu.resolveFQTypeOfName( value, *this->scopes ) );
			}
			writer.print( openxds::base::FormattedString( "<span class='%s' title='%s'>%s</span>", ttype, this->invocationClass->getLastType().getChars(), value ) );
		}
		break;
	
	case SourceToken::NAME:
		{
			String* fq_class_of_value = NULL;
			{
				if ( this->invocationClass->hasEnclosingType() )
				{
					const char* enclosing_type = this->invocationClass->getEnclosingType().getChars();

					MemberSignature* member_signature = this->codebase.completeMemberSignature( enclosing_type, value );
					if ( member_signature->isValid() )
					{
						fq_class_of_value = this->cu.resolveFQTypeOfType( member_signature->getType().getChars() );
						this->invocationClass->setLastType( *fq_class_of_value );
					}
					else
					{
						this->invocationClass->setUnknownName( value );
						fq_class_of_value = new String();
					}
					delete member_signature;
				}
				else
				{
					fq_class_of_value = this->cu.resolveFQTypeOfName( value, *this->scopes );
					if ( ! fq_class_of_value->getLength() )
					{
						delete fq_class_of_value;
						fq_class_of_value = this->cu.resolveFQTypeOfType( value );
					}
					
					if ( fq_class_of_value->getLength() )
					{
						this->invocationClass->setLastType( *fq_class_of_value );
					}
					else
					{
						this->invocationClass->setUnknownName( value );
					}
				}

				if ( ! fq_class_of_value->contentEquals( "" ) )
				{
					const char* _fq_class_of_value = fq_class_of_value->getChars();
					writer.printf( "<a class='%s' title='%s' href='%s.html'>%s</a>", ttype, _fq_class_of_value, _fq_class_of_value, value );
				}
				else
				{
					const char* compound_string = this->invocationClass->getCompoundString().getChars();
					writer.printf( "<span class='%s' title='%s' style='color:#555'>%s</span>", ttype, compound_string, value );
				}
			}
			delete fq_class_of_value;
		}
		break;

	case SourceToken::CONSTRUCTOR:
		constructor = true;
		//	Intentional fall-through.
	case SourceToken::METHODCALL:
		{
			IPosition<SourceToken>* parent = this->tree.parent( p );
			{
				String* parameters  = this->cu.resolveMethodCallArgumentTypes( codebase, this->tree, *parent, *scopes );
				{
					//	The invocation type can either be the result of:
					//	1)	A compound namespace constructor, e.g. com.islabs.util.StringBuffer
					//	2)	The type of the last name, or methodcall return type, e.g. name.methodcall()
					//	3)	A method resolved within the current scope, e.g. StringBuffer() or methodcall()
				
					String* invocation_class = NULL;
					{
						if ( this->invocationClass->hasEnclosingType() )
						{
							invocation_class = this->invocationClass->getEnclosingType().asString();
						}
						else if ( constructor )
						{
							invocation_class = this->cu.resolveFQTypeOfType( value );
						}
						else
						{
							invocation_class = this->cu.getFQName().asString();
						}
						
						MethodSignature* method_signature = this->codebase.completeMethodSignature( invocation_class->getChars(), value, parameters->getChars() );
						if ( method_signature->isValid() )
						{
							const char* _fq_class    = method_signature->getFQClass().getChars();
							const char* _method_name = method_signature->getMethodName().getChars();
							const char* _parameters  = parameters->getChars();
						
							writer.printf( "<a class='%s' href='%s.html#%s(%s)'>%s</a>", ttype, _fq_class, _method_name, _parameters, value );

							this->invocationClass->setLastType( this->cu.resolveFQTypeOfType( method_signature->getReturnType().getChars() ) );
						}
						else
						{
							String* title = new String( value );
							{
								if ( invocation_class->getLength() )
								{
									delete title;
									       title = new FormattedString( "%s.%s", invocation_class->getChars(), value );
								}
								writer.printf( "<span class='%s' title='%s' style='color:red;'>%s</span>", ttype, title->getChars(), value );
							}
							delete title;

							this->invocationClass->clearTypes();
						}
						delete method_signature;
					}
					delete invocation_class;
				}
				delete parameters;
			}
			delete parent;
		}
		break;

	case SourceToken::METHODNAME:
		writer.print( openxds::base::FormattedString( "<span class='%s'>%s</span>", ttype, value ) );
		break;

	case SourceToken::TYPE:
		{
			String* fq_class = this->cu.resolveFQTypeOfType( value );
			if ( fq_class->getLength() )
			{
				writer.print( openxds::base::FormattedString( "<a class='%s' title='%s' href='%s.html'>%s</a>", ttype, fq_class->getChars(), fq_class->getChars(), value ) );
			}
			else
			{
				FormattedString suggested_class( "%s.%s", this->cu.getNamespace().getChars(), value );
				const char* _sc = suggested_class.getChars();

				writer.print( openxds::base::FormattedString( "<a class='%s' title='%s' href='%s.html'>%s</a>", ttype, _sc, _sc, value ) );
			}
			delete fq_class;
		}
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

