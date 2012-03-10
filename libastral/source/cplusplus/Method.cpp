#include "astral/CompilationUnit.h"
#include "astral/Method.h"
#include "astral/MethodsList.h"
#include "astral/MethodSignature.h"
#include <astral.ast/AST.h>
#include <astral.ast/ASTHelper.h>
#include <astral.tours/HTMLPrintTour.h>
#include <astral.tours/MethodDiscoveryTour.h>
#include <astral.tours/PrintSourceTour.h>
#include <astral.tokenizer/SourceToken.h>
#include <openxds.adt/IDictionary.h>
#include <openxds.adt/IEntry.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ISequence.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/FormattedString.h>
#include <openxds.base/Math.h>
#include <openxds.base/String.h>
#include <openxds.base/StringBuffer.h>
#include <openxds.io/IO.h>
#include <openxds.io/IOBuffer.h>
#include <openxds.io/OutputStream.h>
#include <openxds.io/PrintWriter.h>
#include <openxds/Exception.h>
#include <openxds.exceptions/NoSuchElementException.h>

#include <stdio.h>

using namespace astral;
using namespace astral::ast;
using namespace astral::tours;
using namespace astral::tokenizer;
using namespace openxds;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;
using namespace openxds::exceptions;
using namespace openxds::io;

class MethodTuple
{
public:
	MethodSignature* signature;
	AST*             ast;
	
	MethodTuple( MethodSignature* sig, AST* ast )
	{
		this->signature = sig;
		this->ast       = ast;
	}
	
	virtual ~MethodTuple()
	{
		delete this->signature;
		delete this->ast;
	}
};

static IDictionary<IPosition<SourceToken> >* findMethodPositions(          AST& ast );
static ISequence<MethodTuple>*                    extractMethods(          AST& methodAST, const String& className ); 
static String*                                         translate(       String* aString );
static long                                     determineMinTabs( const String& aString );

Method::Method( CompilationUnit& cu, MethodsList& ml, const MethodSignature& aMethodSignature ) : cu( cu ), ml( ml )
{
	this->p         = NULL;
	this->methodAST = new AST();
	this->signature = new MethodSignature( aMethodSignature );
	this->modified  = false;
	this->empty     = false;
	
	const char* return_type = aMethodSignature.getReturnType().getChars();
	const char* method_call = aMethodSignature.getMethodCall().getChars();
	
	StringBuffer sb;
	sb.append( FormattedString( "public %s %s\n", return_type, method_call ) );
	sb.append( "{\n" );
	sb.append( "}\n" );
	
	this->reparseMethod( sb.getContent() );
}

Method::Method( CompilationUnit& cu, MethodsList& ml, const MethodSignature& aMethodSignature, IPosition<SourceToken>& aPosition ) : cu( cu ), ml( ml )
{
	this->p         = aPosition.copy();
	this->methodAST = cu.getAST().copySubtree( aPosition );
	this->signature = new MethodSignature( aMethodSignature );
	this->modified  = false;
	this->empty     = false;
}

Method::Method( CompilationUnit& cu, MethodsList& ml, const MethodSignature& aMethodSignature, AST& ast, IPosition<SourceToken>& p ) : cu( cu ), ml( ml )
{
	this->p = NULL;
	this->methodAST = ast.copySubtree( p );
	this->signature = new MethodSignature( aMethodSignature );
	this->modified  = true;
	this->empty     = false;
	
	delete this->methodAST->copySubtree( this->methodAST->getTree().getRoot() );
}

Method::~Method()
{
	delete this->methodAST;
	delete this->signature;
	delete this->p;
}

String*
Method::retrieveMethodContent()
{
	String* ret = new String();

	const AST&                 ast = this->getMethodAST();
	const ITree<SourceToken>& tree = ast.getTree();
	
	if ( tree.size() )
	{
		PrintWriter writer( new OutputStream( new IOBuffer() ) );
		PrintSourceTour print_tour( tree, writer );
		print_tour.doGeneralTour();
		delete ret;
		       ret = translate( dynamic_cast<IOBuffer&>( writer.getOutputStream().getIOEndPoint() ).toString() );
	}
	return ret;
}

void
Method::reparseMethod( const String& content )
{
	AST* pAST = new AST();

	this->empty = content.isOnlyWhitespace() ? true : false;

	if ( ! this->empty )
	{
		try
		{
			pAST->parseString( content );
		}
		catch ( Exception* ex )
		{
			delete ex;
		}
	}
	this->modified = true;
	delete this->methodAST;
	this->methodAST = pAST;
}

//bool
//Method::oldsync()
//{
//	bool status = true;
//	
//	MethodSignature* first_signature = NULL; 
//
//	if ( this->empty )
//	{
//		status &= this->removeFromCUAndSave();
//	}
//	else if ( this->modified )
//	{
//		ISequence<MethodTuple>* method_tuples = extractMethods( *this->methodAST, this->cu.getName() );
//		{
//			bool first = true;
//			IIterator<MethodTuple>* it = method_tuples->elements();
//			while ( it->hasNext() )
//			{
//				MethodTuple& method = it->next();
//				status &= saveMethodTuple( (void*) &method );
//
//				if ( status && first )
//				{
//					delete this->signature;
//					       this->signature = new MethodSignature( *method.signature );
//				}
//				first = false;
//			}
//			delete it;
//		}
//		delete method_tuples;
//
//		if ( status )
//		{
//			this->cu.save();
//			this->modified = false;
//		}
//	}
//	delete first_signature;
//	
//	return status;
//}

bool
Method::sync()
{
	bool status = false;
	
	if ( this->isModified() )
	{
		MethodSignature* method_signature = extractMethodSignature( *this->methodAST );
		{
			const char* modified_key = method_signature->getMethodKey().getChars();
			const char* existing_key =  this->signature->getMethodKey().getChars();

			ASTHelper helper( this->cu.getAST() );

			if ( ! this->p )
			{
				this->p = helper.insertMethodAST( *this->methodAST );
				delete this->ml.getMethodPositions().insert( modified_key, this->p->copy() );
			}
			else
			{
				this->replaceMethod( modified_key, *this->methodAST, existing_key );
			}
			this->modified = false;

			delete this->signature;
			       this->signature = new MethodSignature( *method_signature );

			status = true;
		}
		delete method_signature;
	}
	return status;
}

//bool
//Method::saveMethodTuple( void* _method )
//{
//	bool status = false;
//	
//	MethodTuple& method = *static_cast<MethodTuple*>( _method );
//	
//	if ( method.signature->isValid() )
//	{
//		const char* existing_key = this->signature->getMethodKey().getChars();
//		const char* key          = method.signature->getMethodKey().getChars();
//
//		bool same_key = method.signature->getMethodKey().contentEquals( existing_key );
//		bool good_key = ! this->ml.getMethodPositions().containsKey( key );
//	
//		if ( same_key || good_key )
//		{
//			if ( !this->p )
//			{
//				this->p = this->insertNewMethod( key, *method.ast );
//			}
//			else if ( ! same_key )
//			{
//				delete this->insertNewMethod( key, *method.ast );
//			}
//			else
//			{
//				this->replaceMethod( key, *method.ast, existing_key );
//			}
//
//			status = true;
//		}
//	}
//	return status;
//}

//bool
//Method::removeFromCUAndSave()
//{
//	bool success = false;
//
//	const char* old_method_key =  this->signature->getMethodKey().getChars();
//	if ( this->removeMethod( old_method_key ) )
//	{
//		this->cu.save();
//		this->modified = false;
//		delete this->signature;
//			   this->signature = new MethodSignature();
//		delete this->p;
//		       this->p = NULL;
//		success = true;
//	}
//	return success;
//}

ISequence<Method>*
Method::extractExtraMethods()
{
	ASTHelper helper( *this->methodAST );

	ISequence<Method>* methods = new Sequence<Method>();
	{
		IDictionary<IPosition<SourceToken> >* method_positions = findMethodPositions( *this->methodAST );
		{
			IEIterator<IPosition<SourceToken> >* ie = method_positions->entries();

			if ( ie->hasNext() ) delete ie->next();

			while ( ie->hasNext() )
			{
				IEntry<IPosition<SourceToken> >* e = ie->next();
				MethodSignature* s = MethodSignature::createInContext( e->getKey(), cu.getName() );
				{
					IPosition<SourceToken>* p = method_positions->remove( e );
			
					Method* method = new Method( this->cu, this->ml, *s, *this->methodAST, *p );
					methods->insertLast( method );
					
					helper.deleteSubtree( p );
				}
				delete s;
			}
			delete ie;
		}
		delete method_positions;
	}
	return methods;
}

bool
Method::isValid() const
{
	return this->methodAST->isValid();
}

long
Method::getFirstLine() const
{
	long first_line = 0;
	if ( this->p )
	{
		first_line = this->cu.calculateOffset( *this->p );
	}
	return first_line;
}

//IPosition<SourceToken>*
//Method::insertNewMethod( const char* methodKey, const AST& aMethodAST )
//{
//	ASTHelper helper( this->cu.getAST() );
//	IPosition<SourceToken>* p = helper.insertMethodAST( *this->methodAST );
//	delete this->ml.getMethodPositions().insert( methodKey, p->copy() );
//	
//	return p;
//}

bool
Method::replaceMethod( const char* methodKey, const AST& aMethodAST, const char* oldMethodKey )
{
	bool status = false;

	try
	{
		IDictionary<IPosition<SourceToken> >& methods = this->ml.getMethodPositions();
		delete methods.insert( methodKey, methods.remove( methods.find( oldMethodKey ) ) );
		this->cu.getAST().replaceSubtree( *this->p, aMethodAST );

		status = true;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}
	
	return status;
}

bool
Method::removeMethod( const char* methodKey )
{
	bool status = false;
	
	ASTHelper helper( this->cu.getAST() );
	try
	{
		IDictionary<IPosition<SourceToken> >& methods = this->ml.getMethodPositions();
		IEntry<IPosition<SourceToken> >*            e = methods.find( methodKey );

		helper.deleteSubtree( methods.remove( e ) );

		status = true;
	}
	catch ( NoSuchElementException* ex )
	{
		delete ex;
	}

	return status;
}

static IDictionary<IPosition<SourceToken> >* findMethodPositions( AST& ast )
{
	IDictionary<IPosition<SourceToken> >* methods = new Dictionary<IPosition<SourceToken> >();
	MethodDiscoveryTour mdt( ast.getTree(), *methods );
	mdt.doGeneralTour();
	return methods;
}

MethodSignature*
Method::extractMethodSignature( AST& aMethodAST )
{
	MethodSignature* sig = NULL;
	{
		const char* key = NULL;

		IDictionary<IPosition<SourceToken> >* methods = findMethodPositions( aMethodAST );
			
		IEIterator<IPosition<SourceToken> >* ie = methods->entries();
		if ( ie->hasNext() )
		{
			IEntry<IPosition<SourceToken> >* e = ie->next();
			{
				key = e->getKey();
				sig = MethodSignature::createInContext( key, this->cu.getName() );
			}
			delete e;
		}
		delete ie;
	}
	return sig;
}


MethodSignature*
Method::regenerateSignature() const
{
	MethodSignature* sig = NULL;
	{
		const char* key = NULL;

		IDictionary<IPosition<SourceToken> >* methods = findMethodPositions( *this->methodAST );
			
		IEIterator<IPosition<SourceToken> >* ie = methods->entries();
		if ( ie->hasNext() )
		{
			IEntry<IPosition<SourceToken> >* e = ie->next();
			{
				key = e->getKey();
				sig = new MethodSignature( key );
			}
			delete e;
		}
		delete ie;

		if ( sig && !sig->isValid() )
		{
			if ( sig->getMethodName().contentEquals( this->cu.getName() ) )
			{
				delete sig;
					   sig = new MethodSignature( FormattedString( "%s%s", key, this->cu.getName().getChars() ) );
			}
		}
	}
	return sig;
}

static ISequence<MethodTuple>* extractMethods( AST& methodAST, const String& className )
{
	bool status = true;

	ISequence<MethodTuple>* method_tuples = new Sequence<MethodTuple>();
	{
		IDictionary<IPosition<SourceToken> >* methods = findMethodPositions( methodAST );
		IEIterator<IPosition<SourceToken> >* ie = methods->entries();
		while ( ie->hasNext() )
		{
			IEntry<IPosition<SourceToken> >* e = ie->next();
			{
				MethodSignature* signature = MethodSignature::createInContext( e->getKey(), className );
				if ( signature )
				{
					IPosition<SourceToken>& p = e->getValue();
					method_tuples->addLast( new MethodTuple( signature, methodAST.copySubtree( p ) ) );
				}
				else
				{
					status = false;
				}
			}
			delete e;
		}
		delete ie;
	}
	
	if ( !status )
	{
		delete method_tuples;
		method_tuples = NULL;
	}
	
	return method_tuples;
}

static String* translate( String* aString )
{
	long min_tabs = determineMinTabs( *aString );
	
	StringBuffer sb;
	{
		      long  t      = 0;
		      long  max    = aString->getLength();
		const char* string = aString->getChars();
		for ( long i=0; i < max; i++ )
		{
			char ch = string[i];
			switch ( ch )
			{
			case '\n':
				sb.append( ch );
				t = min_tabs;
				while ( t && ('\t' == string[i+1]) )
				{
					t--;
					i++;
				}
				break;
			case '\t':
				if ( 0 != i ) sb.append( ch );
				break;
			default:
				sb.append( ch );
			}
		}
	}
	delete aString;
	return sb.asString();
}

static long determineMinTabs( const String& aString )
{
	long min_tabs = 0;
	long tabs     = 0;

	long max      = aString.getLength();
	for ( long i=0; i < max; i++ )
	{
		char ch = aString.charAt( i );
		switch ( ch )
		{
		case '\t':
			tabs++;
			break;
		case '\n':
			min_tabs = Math::min( min_tabs, tabs );
			break;
		}
	}
	return Math::max( 1, min_tabs );
}

