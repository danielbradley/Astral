#include "astral.tours/MemberDiscoveryTour.h"

#include <astral.tokenizer/SourceToken.h>
#include <openxds.adt/IEntry.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.base/StringTokenizer.h>

using namespace astral::ast;
using namespace astral::tokenizer;
using namespace astral::tours;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;

void
MemberDiscoveryTour::visitPreorder( IPosition<SourceToken>& p, Result& r )
{
	SourceToken& token = p.getElement();
	
	switch ( token.getTokenType() )
	{
	case SourceToken::MEMBER:
		this->addMemberToDirectory( p );
		break;
	default:
		break;
	}
}

void
MemberDiscoveryTour::addMemberToDirectory( IPosition<SourceToken>& p )
{
	String* mod  = new String();
	String* type = new String();
	String* name = new String();
	{
		IPIterator<SourceToken>* it = this->tree.children( p );
		while ( it->hasNext() )
		{
			IPosition<SourceToken>* px = it->next();
			{
				switch ( px->getElement().getTokenType() )
				{
				case SourceToken::TYPE:
					delete type;
					type = new String( px->getElement().getValue() );
					break;
				case SourceToken::VARIABLE:
					delete name;
					name = new String( px->getElement().getValue() );
					break;
				case SourceToken::MODIFIER:
					delete mod;
					mod = new String( px->getElement().getValue() );
					break;
				default:
					break;
				}
			}
			delete px;
		}
		delete it;

		bool strip_parameterization = false;

		if ( strip_parameterization )
		{
			StringTokenizer st( *type );
			st.setDelimiter( '<' );
			if ( st.hasMoreTokens() )
			{
				delete type; type = st.nextToken();
			}
		}

		StringBuffer sb;
		{
//			if ( 0 < mod->getLength() )
//			{
//				sb.append( *mod  );
//				sb.append( ' '   );
//			}
			sb.append( *name );
			sb.append( '|'   );
			sb.append( *type );
			delete this->members.insert( sb.getChars(), p.copy() );
		}
	}
	delete type;
	delete name;
}

