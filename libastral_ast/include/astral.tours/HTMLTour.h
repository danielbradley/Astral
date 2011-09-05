#ifndef ASTRAL_AST_HTMLPRINTTOURS_H
#define ASTRAL_AST_HTMLPRINTTOURS_H

#include "astral.ast.h"
#include <astral.tokenizer.h>
#include <astral.tokenizer/SourceToken.h>

#include <openxds.adt/ITree.h>

#include <openxds.adt.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/GeneralTour.h>
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

	HTMLPrintTour( openxds::adt::ITree<astral::tokenizer::SourceToken>& tree ) : openxds::adt::std::GeneralTour<SourceToken>( tree )
	{
		this->depth = -1;
	}
	
	virtual void visitPreorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
	{
		astral::tokenizer::SourceToken& token = p.getElement();
		const char* type = token.getTokenTypeString().getChars();
		
		switch ( token.getTokenType() )
		{
		case astral::tokenizer::SourceToken::PARAMETERS:
		case astral::tokenizer::SourceToken::PARAMETER:
			fprintf( stdout, "<div class='%s inline'>", type );
			break;
		default:
			fprintf( stdout, "<div class='%s'>", type );
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
			fprintf( stdout, "</div>" );
			break;
		}
		
	}
	
	virtual void visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
	{
		astral::tokenizer::SourceToken& token = p.getElement();
		const char* value = token.getValue().getChars();
		const char* ttype = token.getTokenTypeString().getChars();
		openxds::base::String* str = NULL;
		
		switch ( token.getTokenType() )
		{
		case SourceToken::SPACE:
		case SourceToken::WHITESPACE:
			fprintf( stdout, " " );
			break;
		case SourceToken::TAB:
			fprintf( stdout, "<span class='%s'></span>", ttype );
			break;
		case SourceToken::BLANKLINE:
			fprintf( stdout, "<span class='%s'><br></span>\n", ttype );
			break;
		case SourceToken::NEWLINE:
			fprintf( stdout, "<span class='%s'><br></span>\n", ttype );
			break;
		case SourceToken::STARTBLOCK:
			fprintf( stdout, "<span>{</span>" );
			break;
		case SourceToken::ENDBLOCK:
			fprintf( stdout, "<span>}</span>" );
			break;
		case SourceToken::STARTEXPRESSION:
			fprintf( stdout, "<span>(</span>" );
			break;
		case SourceToken::ENDEXPRESSION:
			fprintf( stdout, "<span>)</span>" );
			break;
		case SourceToken::COMMENT:
			str = extract( value );
			fprintf( stdout, "<pre class='COMMENT'>%s</pre>", str->getChars() );
			delete str;
			break;
		case SourceToken::JAVADOC:
			str = extract( value );
			fprintf( stdout, "<pre class='JAVADOC'>%s</pre>", str->getChars() );
			delete str;
			break;
		default:
			fprintf( stdout, "<span class='%s'>%s</span>", ttype, value );
		}
	}

};

static openxds::base::String* extract( const char* value )
{
	openxds::base::StringBuffer sb;
	bool add = false;

	int len = strlen( value );
	for ( int i=0; i < len; i++ )
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
