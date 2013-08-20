#ifndef ASTRAL_AST_HTMLPRINTTOURS_H
#define ASTRAL_AST_HTMLPRINTTOURS_H

#include "astral.ast.h"
#include <astral.tokenizer.h>
#include <astral.tokenizer/SourceToken.h>

#include <openxds.io/PrintWriter.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/GeneralTour.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.base.h>
#include <openxds/Object.h>

#include <cstdio>
#include <cstring>

using namespace astral::tokenizer;

namespace astral {
	namespace tours {

static openxds::base::String* extract( const char* value );

//template <class E>
class HTMLPrintTour : public openxds::adt::std::GeneralTour<SourceToken>
{
public:
	int depth;
	openxds::io::PrintWriter& writer;

	HTMLPrintTour( openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, openxds::io::PrintWriter& aWriter )
	: openxds::adt::std::GeneralTour<SourceToken>( tree ), writer( aWriter )
	{
		this->depth = -1;
	}
	
	virtual void visitPreorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
	{
		astral::tokenizer::SourceToken& token = p.getElement();
		const char* type = token.getTokenTypeString().getChars();
		const char* name = token.getValue().getChars();
		
	switch ( token.getTokenType() )
	{
	case SourceToken::METHOD:
		writer.print( openxds::base::FormattedString( "<span class='%s' name='%s'>", type, name ) );
		break;
	case astral::tokenizer::SourceToken::PARAMETERS:
	case astral::tokenizer::SourceToken::ARGUMENTS:
		writer.print( openxds::base::FormattedString( "<span class='%s inline' name='%s'>", type, name ) );
		break;
	case astral::tokenizer::SourceToken::METHODCALL:
		writer.print( openxds::base::FormattedString( "<span class='%s inline' name='%s'>", type, name ) );
		break;
	case astral::tokenizer::SourceToken::PARAMETER:
	case astral::tokenizer::SourceToken::ARGUMENT:
		writer.print( openxds::base::FormattedString( "<span class='%s inline' name='%s'>", type, name ) );
		break;
	case SourceToken::BLOCK:
		writer.print( openxds::base::FormattedString( "<span class='%s' name='%s'>", type, name ) );
		break;
	case SourceToken::DECLARATION:
		writer.print( openxds::base::FormattedString( "<span class='%s' name='%s'>", type, name ) );
		break;
	default:
		writer.print( openxds::base::FormattedString( "<span class='%s' name='%s'>", type, name ) );
		break;
	}
		
		this->depth++;
	}

	virtual void visitPostorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
	{
		astral::tokenizer::SourceToken& token = p.getElement();

		this->depth--;
		
		switch ( token.getTokenType() )
		{
		default:
			writer.print( "</span>" );
			break;
		}
		
	}
	
	virtual void visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
	{
		astral::tokenizer::SourceToken& token = p.getElement();
		const char* value = token.getValue().getChars();
		const char* ttype = token.getTokenTypeString().getChars();
		openxds::base::String* str = NULL;
		
		String* encoded = String::htmlEncode( value );
		{
			value = encoded->getChars();
		
			switch ( token.getTokenType() )
			{
			case SourceToken::SPACE:
			case SourceToken::WHITESPACE:
				writer.print( " " );
				break;
			case SourceToken::TAB:
				writer.print( openxds::base::FormattedString( "<span class='%s'>\t</span>", ttype ) );
				break;
			case SourceToken::BLANKLINE:
				writer.print( openxds::base::FormattedString( "<span class='%s'>\n</span>", ttype ) );
				break;
			case SourceToken::NEWLINE:
				writer.print( openxds::base::FormattedString( "<span class='%s'>\n</span>", ttype ) );
				break;
			case SourceToken::STARTBLOCK:
				writer.print( "<span>{</span>" );
				break;
			case SourceToken::ENDBLOCK:
				writer.print( "<span>}</span>" );
				break;
			case SourceToken::STARTEXPRESSION:
				writer.print( "<span>(</span>" );
				break;
			case SourceToken::ENDEXPRESSION:
				writer.print( "<span>)</span>" );
				break;
			case SourceToken::COMMENT:
				str = extract( value );
				writer.print( openxds::base::FormattedString( "<pre class='COMMENT'>%s</pre>", str->getChars() ) );
				delete str;
				break;
			case SourceToken::JAVADOC:
				str = extract( value );
				writer.print( openxds::base::FormattedString( "<pre class='JAVADOC'>%s</pre>", str->getChars() ) );
				delete str;
				break;
			default:
				writer.print( openxds::base::FormattedString( "<span class='%s'>%s</span>", ttype, value ) );
			}
		}
		delete encoded;
	}

};

static openxds::base::String* extract( const char* value )
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

	
};};

#endif
