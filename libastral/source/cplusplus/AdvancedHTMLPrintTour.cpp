#include "astral/AdvancedHTMLPrintTour.h"
#include "astral/CompilationUnit.h"

#include <openxds.io/PrintWriter.h>
#include <openxds.adt/IDictionary.h>
#include <openxds.adt/IEntry.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ISequence.h>
#include <openxds.adt/ITree.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt.std/GeneralTour.h>
#include <openxds.adt.std/Sequence.h>
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

static openxds::base::String* extract( const char* value );

AdvancedHTMLPrintTour::AdvancedHTMLPrintTour(
	      ITree<SourceToken>& tree,
	const CompilationUnit& aCu,
	const IDictionary<IEntry<CompilationUnit> >& symbols,
	const IDictionary<String>& importedTypes,
	PrintWriter& aWriter
)
: openxds::adt::std::GeneralTour<SourceToken>( tree ), cu( aCu ), symbols( symbols ), importedTypes( importedTypes ), writer( aWriter )
{
	this->depth = -1;
	
	this->scopes = new Sequence<IDictionary<String> >();
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
		delete this->scopes;
		this->scopes = new Sequence<IDictionary<String> >();
		this->scopes->addLast( new Dictionary<String>() );
		writer.print( openxds::base::FormattedString( "<div class='%s'>", type ) );
		break;
	case astral::tokenizer::SourceToken::PARAMETERS:
		writer.print( openxds::base::FormattedString( "<div class='%s inline' name='%s'>", type, token.getValue().getChars() ) );
		break;
	case astral::tokenizer::SourceToken::METHODCALL:
		writer.print( openxds::base::FormattedString( "<div class='%s inline'>", type ) );
		break;
	case astral::tokenizer::SourceToken::PARAMETER:
		writer.print( openxds::base::FormattedString( "<div class='%s inline'>", type ) );
		break;
	case SourceToken::BLOCK:
		this->scopes->addLast( new Dictionary<String>() );
		writer.print( openxds::base::FormattedString( "<div class='%s'>", type ) );
		break;
	case SourceToken::DECLARATION:
		this->addDeclarationToScope( p );
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
		delete this->scopes;
		this->scopes = new Sequence<IDictionary<String> >();
		writer.print( "</div>" );
		break;
	case SourceToken::BLOCK:
		if ( ! this->scopes->isEmpty() )
		{
			delete this->scopes->removeLast();
		}
		writer.print( "</div>" );
		break;
	case SourceToken::METHODCALL:
		delete this->lastType;
		this->lastType = this->methodCallTypes->removeLast();
		writer.print( "</div>" );

		fprintf( stdout, "retrieving: %s\n", this->lastType->getChars() );

		break;
	default:
		writer.print( "</div>" );
	}
}
	
void
AdvancedHTMLPrintTour::visitExternal( openxds::adt::IPosition<SourceToken>& p, openxds::adt::std::Result& r )
{
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
		writer.print( "<span>(</span>" );
		break;
	case SourceToken::ENDEXPRESSION:
		writer.print( "<span>)</span>" );
		break;
	case SourceToken::COMMENT:
	case SourceToken::JAVADOC:
		str = extract( value );
		writer.print( openxds::base::FormattedString( "<pre class='%s'>%s</pre>", ttype, str->getChars() ) );
		delete str;
		break;
	case SourceToken::METHODCALL:
	case SourceToken::NAME:
		{
			openxds::base::String* link = NULL;

			bool is_method_call = (SourceToken::METHODCALL == token.getTokenType());
			if ( is_method_call )
			{
				link = this->resolveMethodCall( value, *this->lastType );
				this->methodCallTypes->addLast( this->lastType->asString() );

				fprintf( stdout, "%s --> %s\n", value, this->lastType->getChars() );
			}
			else
			{
				link = this->resolveLink( value, *this->lastType );
			}
			{
				if ( link->contentEquals( "" ) )
				{
					writer.print( openxds::base::FormattedString( "<span class='%s'>%s</span>", ttype, value ) );
				}
				else if ( is_method_call )
				{
					StringBuffer sb;
					sb.append( *link );
					sb.append( ".html" );
					sb.append( "#" );
					sb.append( value );
				
					writer.print( openxds::base::FormattedString( "<a class='%s' href='%s'>%s</a>", ttype, sb.getChars(), value ) );
				}
				else
				{
					writer.print( openxds::base::FormattedString( "<a class='%s' href='%s.html'>%s</a>", ttype, link->getChars(), value ) );
				}
			}
			delete this->lastType;
			this->lastType = link;
		}
		break;
	default:
		writer.print( openxds::base::FormattedString( "<span class='%s'>%s</span>", ttype, value ) );
	}
}

void
AdvancedHTMLPrintTour::addDeclarationToScope( openxds::adt::IPosition<SourceToken>& p )
{
	String* type = new String();
	String* name = new String();
	{
		IPIterator<SourceToken>* it = this->tree.children( p );
		while ( it->hasNext() )
		{
			IPosition<SourceToken>* pos = it->next();
			{
				switch ( pos->getElement().getTokenType() )
				{
				case SourceToken::TYPE:
					delete type;
					type = new String( pos->getElement().getValue() );
					break;
				case SourceToken::VARIABLE:
					delete name;
					name = new String( pos->getElement().getValue() );
					break;
				}
			}
			delete pos;
		}
		delete it;
		
		if ( ! this->scopes->isEmpty() )
		{
			this->scopes->getLast().insert( name->getChars(), new String( *type ) );
		}
	}
	delete type;
	delete name;
}

String*
AdvancedHTMLPrintTour::resolveInvocationClass( const String& lastType )
{
	String* invocation_class = NULL;

	if ( lastType.contentEquals( "" ) )
	{
		StringBuffer fq;
		fq.append( this->cu.getNamespace() );
		fq.append( '.' );
		fq.append( this->cu.getName() );
		invocation_class = fq.asString();
	}
	else
	{
		invocation_class = new String( lastType );
	}

	return invocation_class;
}

String*
AdvancedHTMLPrintTour::resolveReturnType( const char* invokee, const char* name )
{
	String* ret = new String();
	{
		StringBuffer call;
		call.append( name );
		call.append( "()" );

		StringBuffer full;
		full.append( invokee );
		full.append( '.' );
		full.append( call.getChars() );

		try
		{
			IEntry<IEntry<CompilationUnit> >* entry = symbols.startsWith( full.getChars() );
			{
				CompilationUnit& cu = entry->getValue().getValue();
				String* rtype = cu.resolveMethodType( call.getChars() );
				{
					StringBuffer sb;
					sb.append( invokee );
					sb.append( '|' );
					sb.append( *rtype );

					delete ret;
					ret = sb.asString();
				}
				delete rtype;
			}
			delete entry;
		}
		catch ( NoSuchElementException* ex )
		{
			delete ex;
			
			try
			{
				IEntry<IEntry<CompilationUnit> >* entry = symbols.startsWith( invokee );
				{
					CompilationUnit& cu = entry->getValue().getValue();
					const String& superclass = cu.getSuperclass();
					String* super = this->resolveFQType( superclass.getChars() );
					if ( ! super->contentEquals( "" ) )
					{
						delete ret;
						ret = this->resolveReturnType( super->getChars(), name );
					}
					delete super;
				}
				delete entry;
			}
			catch ( NoSuchElementException* ex )
			{
				delete ex;
			}
		}
	}
	return ret;
}


String*
AdvancedHTMLPrintTour::resolveMethodCall( const char* name, const String& lastType )
{
	String* link = new String();
	{
		//	1)	Get type invoked upon from either lastType or this cu.
		
		//	2)	Append method call and retrieve cu.
		
		//		a)	If cu is returned (a match) complete.
		//		b)	If no cu is returned:
		//			1.	relookup cu using type invoked upon.
		//			2.	get superclass
		//			3.  resolve to full type using resolveFQType
		//			4.	Go to 2
	
		String* invocation_class = this->resolveInvocationClass( lastType );
		String* return_type      = this->resolveReturnType( invocation_class->getChars(), name );
		
		StringTokenizer st( *return_type );
		st.setDelimiter( '|' );

		if ( st.hasMoreTokens() )
		{
			delete link;
			link = st.nextToken();
			
			if ( st.hasMoreTokens() )
			{
				String* rtype = st.nextToken();
				{
					delete this->lastType;
					this->lastType = this->resolveFQType( rtype->getChars() );
				}
				delete rtype;
			}
		}
		delete return_type;
		delete invocation_class;
	}
	return link;
}
	
openxds::base::String*
AdvancedHTMLPrintTour::resolveLink( const char* name, const String& lastType )
{
	String* link = new String();

	if ( lastType.contentEquals( "" ) )
	{
		String* type = this->resolveTypeOfName( name );

		if ( ! type->contentEquals( "" ) )
		{
			delete link;
			link = this->resolveFQType( type->getChars() );
		}
		delete type;
	}
	else
	{
		// lastType = com.demo.other.Other
		//	name = identifier
		StringBuffer sb;
		sb.append( lastType );
		sb.append( '.' );
		sb.append( name );
		
//		if ( isMethodCall )
//		{
//			sb.append( "()" );
//		}
		
		try
		{
			IEntry<IEntry<CompilationUnit> >* e = this->symbols.find( sb.getChars() );
			{
				CompilationUnit& cu = e->getValue().getValue();
				
				String* type = NULL;

//				if ( isMethodCall )
//				{
//					printf( "AHPT: Searching for: %s\n", sb.getChars() );
//					type = cu.resolveMethodType( name );
//				}
//				else
				{
					type = cu.resolveMemberType( name );
				}
				if ( ! type->contentEquals( "" ) )
				{
					printf( "AHPT: found: %s\n", type->getChars() );

					delete link;
					link = this->resolveFQType( type->getChars() );
				}
				delete type;
			}
			delete e;
		}
		catch ( NoSuchElementException* ex )
		{
			delete ex;
		}
	}
	
	return link;
}

openxds::base::String*
AdvancedHTMLPrintTour::resolveTypeOfName( const char* name )
{
	String* type = this->resolveTypeIfLocal( name );

	if ( type->contentEquals( "" ) )
	{
		delete type;
		type = this->cu.resolveMemberType( name );
	}
	return type;
}

openxds::base::String*
AdvancedHTMLPrintTour::resolveTypeIfLocal( const char* name )
{
	String* type = new String();

	int nr = this->scopes->size() - 1;
	while ( 0 <= nr )
	{
		try
		{
			IEntry<String>* e = this->scopes->get( nr ).find( name );
			{
				delete type;
				type = new String( e->getValue() );
			}
			delete e;
			nr = -1;
		}
		catch ( NoSuchElementException* ex )
		{
			delete ex;
		}
		nr--;
	}

	return type;
}

String*
AdvancedHTMLPrintTour::resolveFQType( const char* type )
{
	StringBuffer sb;

	try
	{
		IEntry<String>* e = this->importedTypes.find( type );
		{
			sb.append( e->getValue() );
			sb.append( "." );
			sb.append( type );
		}
		delete e;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}

	return sb.asString();
}

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
