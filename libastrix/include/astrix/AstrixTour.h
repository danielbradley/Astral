#ifndef ASTRIX_ASTRIXTOUR_H
#define ASTRIX_ASTRIXTOUR_H

#include "astrix.h"

#include <astral.tokenizer.h>
#include <astral.tokenizer/SourceToken.h>
#include <openxds.adt.h>
#include <openxds.adt.std.h>
#include <openxds.adt.std/GeneralTour.h>
#include <openxds.base/String.h>
#include <openxds.io.h>
#include <openxds.io/PrintWriter.h>

using astral::tokenizer::SourceToken;

using openxds::base::String;

namespace astrix {

class AstrixTour : public openxds::adt::std::GeneralTour<astral::tokenizer::SourceToken>
{
private:
	openxds::io::PrintWriter& writer;
	bool inClass;
	bool inMethod;

public:
	AstrixTour( openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, openxds::io::PrintWriter& aWriter )
	: openxds::adt::std::GeneralTour<astral::tokenizer::SourceToken>( tree ), writer( aWriter )
	{
		this->inClass  = false;
		this->inMethod = false;
	}

	virtual void visitPreorder( openxds::adt::IPosition<astral::tokenizer::SourceToken>& p, openxds::adt::std::Result& r )
	{
		SourceToken& token = p.getElement();

		switch ( token.getTokenType() )
		{
		case SourceToken::CLASS:
			this->inClass = true;
			break;

		case SourceToken::METHOD:
			this->inMethod = true;
			break;
		}
	}

	virtual void visitPostorder( openxds::adt::IPosition<astral::tokenizer::SourceToken>& p, openxds::adt::std::Result& r )
	{
		SourceToken& token = p.getElement();

		switch ( token.getTokenType() )
		{
		case SourceToken::FILE:
			writer.println( "\n}" );
			break;

		case SourceToken::CLASS:
			this->inClass = false;
			break;
			
		case SourceToken::METHOD:
			this->inMethod = false;
			break;
		}
	}

	virtual void visitExternal( openxds::adt::IPosition<astral::tokenizer::SourceToken>& p, openxds::adt::std::Result& r )
	{
		SourceToken& token = p.getElement();
		const String&  value = token.getValue();
		const char*   _value = value.getChars();
		SourceToken::TokenType      ttype = token.getTokenType();

		switch ( ttype )
		{
		case SourceToken::KEYWORD:
			if      ( value.contentEquals( "namespace" ) ) writer.printf( "package" );
			else if ( value.contentEquals( "new" ) )
			{
				if ( inMethod ) writer.printf( "new" );
				else writer.printf( "CLASS" );
			}
			else writer.print( value );
			break;

		case SourceToken::ENDBLOCK:
			if ( ! inClass ) writer.print( value );
			break;

		case SourceToken::SYMBOL:
			if ( value.contentEquals( "@" ) ) writer.printf( "this." );
			else writer.print( value );
			break;

		default:
			writer.print( value );
		}
	}
};

};

#endif