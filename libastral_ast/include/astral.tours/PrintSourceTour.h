#ifndef ASTRAL_AST_PRINTSOURCETOUR_H
#define ASTRAL_AST_PRINTSOURCETOUR_H

#include "astral.ast.h"
#include <astral.tokenizer.h>
#include <astral.tokenizer/SourceToken.h>

#include <openxds.adt/ITree.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/GeneralTour.h>
#include <openxds.io/PrintWriter.h>
#include <openxds.base/String.h>

//#include <openxds.base/FormattedString.h>
//#include <openxds.base/StringBuffer.h>

#include <cstdio>
#include <cstring>

using namespace astral::tokenizer;

namespace astral {
	namespace tours {

static openxds::base::String* extract( const char* value );

class PrintSourceTour : public openxds::adt::std::GeneralTour<SourceToken>
{
public:
	openxds::io::PrintWriter& writer;

	PrintSourceTour( const openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, openxds::io::PrintWriter& aWriter )
	: openxds::adt::std::GeneralTour<SourceToken>( tree ), writer( aWriter )
	{}
	
	virtual void visitPreorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
	{}

	virtual void visitPostorder( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
	{}
	
	virtual void visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
	{
	
		astral::tokenizer::SourceToken& token = p.getElement();
		if ( null != &token.getValue() )
		{
			const openxds::base::String& value = token.getValue();
			
			if ( null != value.getChars() )
			{
				writer.print( value );
			}
			else
			{
				writer.print( "null" );
			}
		}
		else
		{
				writer.print( "NULL" );
		}
	}

	virtual void visitPreorder( const openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r ) const
	{}

	virtual void visitPostorder( const openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r ) const
	{}
	
	virtual void visitExternal( const openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r ) const
	{
		const astral::tokenizer::SourceToken& token = p.getElement();
		writer.print( token.getValue() );
	}
};
	
};};

#endif
