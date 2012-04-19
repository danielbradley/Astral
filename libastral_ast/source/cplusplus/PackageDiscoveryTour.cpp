#include "astral.tours/PackageDiscoveryTour.h"

#include <astral.tokenizer/SourceToken.h>
#include <openxds.adt/IEntry.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.adt.std/Sequence.h>

using namespace astral::ast;
using namespace astral::tokenizer;
using namespace astral::tours;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;

PackageDiscoveryTour::PackageDiscoveryTour( ITree<SourceToken>& tree, IList<String>& imports, IList<IPosition<SourceToken> >& importPositions )
: openxds::adt::std::GeneralTour<SourceToken>( tree ), imports( imports ), importPositions( importPositions )
{
	this->packageName  = new String();
	this->className    = new String();
	this->genericName  = new StringBuffer();
	this->extendsClass = new String();
	
	this->inClassname = false;
	this->capture     = false;
}
	
PackageDiscoveryTour::~PackageDiscoveryTour()
{
	delete this->packageName;
	delete this->className;
	delete this->genericName;
	delete this->extendsClass;
}

void
PackageDiscoveryTour::visitPreorder( IPosition<SourceToken>& p, Result& r )
{
	SourceToken& token = p.getElement();
	
	switch ( token.getTokenType() )
	{
	case SourceToken::PACKAGE:
		this->setPackageName( p );
		break;
	case SourceToken::IMPORT:
		this->addImport( p );
		break;
	default:
		break;
	}
}

void
PackageDiscoveryTour::visitPostorder( IPosition<SourceToken>& p, Result& r )
{
}

void
PackageDiscoveryTour::visitExternal( IPosition<SourceToken>& p, Result& r )
{
	SourceToken& token = p.getElement();
	
	switch ( token.getTokenType() )
	{
	case SourceToken::CLASSNAME:
		delete this->className;
		this->className = token.getValue().asString();
		this->genericName->append( *this->className );
		this->inClassname = true;
		break;

	case SourceToken::INFIXOP:
		if ( this->inClassname )
		{
			char ch = token.getValue().charAt( 0 );
			switch ( ch )
			{
			case '<':
				this->genericName->append( ch );
				break;

			case '>':
				this->genericName->append( ch );
				this->inClassname = false;
				break;

			default:
				break;
			}
		}
		break;

	case SourceToken::KEYWORD:
		if ( token.getValue().contentEquals( "extends" ) )
		{
			this->capture = true;
		}
		break;

	case SourceToken::NAME:
		if ( this->inClassname )
		{
			this->genericName->append( token.getValue() );
		}
		else if ( this->capture )
		{
			const char* _token = token.getValue().getChars();
			delete this->extendsClass;
			this->extendsClass = new String( _token );
			this->capture = false;
		}
		break;
	default:
		break;
	}
}

const String&
PackageDiscoveryTour::getPackageName() const
{
	return *this->packageName;
}

const String&
PackageDiscoveryTour::getClassName() const
{
	return *this->className;
}

const String&
PackageDiscoveryTour::getGenericName() const
{
	return this->genericName->getContent();
}

const String&
PackageDiscoveryTour::getExtendsClass() const
{
	printf( "%s\n", this->extendsClass->getChars() );
	return *this->extendsClass;
}

void
PackageDiscoveryTour::setPackageName( IPosition<SourceToken>& p )
{
	delete this->packageName;
	this->packageName = this->extractSelection( p );
}

void
PackageDiscoveryTour::addImport( IPosition<SourceToken>& p )
{
	this->importPositions.insertLast( p.copy() );

	String* selection = extractSelection( p );
	{
		bool dont_remove_asterisk = true;
	
		if ( dont_remove_asterisk )
		{
			delete this->imports.insertLast( selection->asString() );
		}
		else
		{
			long len = selection->getLength();
			if ( '*' == selection->charAt( len-1 ) )
			{
				delete this->imports.insertLast( selection->substring( 0, len-3 ) );
			} else {
				delete this->imports.insertLast( selection->asString() );
			}
		}
	}
	delete selection;
}

String*
PackageDiscoveryTour::extractSelection( IPosition<SourceToken>& p )
{
	StringBuffer sb;

	IPIterator<SourceToken>* it = this->tree.children( p );
	while ( it->hasNext() )
	{
		IPosition<SourceToken>* p1 = it->next();
		{
			switch ( p1->getElement().getTokenType() )
			{
			case SourceToken::NAME:
				sb.append( p1->getElement().getValue() );
				break;
			case SourceToken::SELECTOR:
				sb.append( p1->getElement().getValue() );
				break;
			default:
				break;
			}
		}
		delete p1;
	}
	delete it;

	return sb.asString();
}


