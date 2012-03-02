
~!include/astral/MembersList.h~
#ifndef ASTRAL_MEMBERSLIST_H
#define ASTRAL_MEMBERSLIST_H
~

~include/astral/MembersList.h~
#include <astral.h>
#include <astral.tokenizer.h>
#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>
#include <openxds.exceptions/NoSuchElementException.h>
~

~include/astral/MembersList.h~
using astral::Member;
using astral::tokenizer::SourceToken;
using openxds::adt::IDictionary;
using openxds::adt::IIterator;
using openxds::adt::IList;
using openxds::adt::IPosition;
using openxds::adt::ISequence;
using openxds::base::String;
using openxds::exceptions::NoSuchElementException;
~

~include/astral/MembersList.h~
namespace astral {
~

~include/astral/MembersList.h~
class MembersList : openxds::Object
{
private:
	CompilationUnit&          cu;
	IDictionary<Member>* members;
	String*                empty;

public:
	 MembersList( CompilationUnit& aCompilationUnit );
	~MembersList();

	void            initialise( IDictionary<IPosition<SourceToken> >& memberPositions );
	
	void              addBlank();
	void                remove( long index );
	
	Member&               find( const char* memberKey );
	IIterator<Member>*  values();

	Member&                get( long index ) throw (NoSuchElementException*);
	const String& getStringFor( long index );

	long                  size() const;
};
~

~include/astral/MembersList.h~
}; // end namespace
~

~include/astral/MembersList.h~
#endif
~




~!source/cplusplus/MembersList.cpp~
#include "astral/MembersList.h"
#include "astral/Member.h"

#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/String.h>
~

~source/cplusplus/MembersList.cpp~
using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
~

~source/cplusplus/MembersList.cpp~
MembersList::MembersList( CompilationUnit& aCompilationUnit ) : cu( aCompilationUnit )
{
	this->members = new Dictionary<Member>();
	this->empty   = new String();
}
~

~source/cplusplus/MembersList.cpp~
void
MembersList::initialise( IDictionary<IPosition<SourceToken> >& memberPositions )
{	
	IEIterator<IPosition<SourceToken> >* it = memberPositions.entries();
	while ( it->hasNext() )
	{
		IEntry<IPosition<SourceToken> >* e = it->next();
		{
			IPosition<SourceToken>& p = e->getValue();
			Member* member = new Member( this->cu, p );
		
			this->members->insert( e->getKey(), member );
		}
		delete e;
	}
	delete it;

	this->addBlank();
}
~

~source/cplusplus/MembersList.cpp~
MembersList::~MembersList()
{
	delete this->members;
}
~

~source/cplusplus/MembersList.cpp~
void
MembersList::addBlank()
{
	this->members->insert( "", new Member( this->cu ) );
}
~

~source/cplusplus/MembersList.cpp~
void
MembersList::remove( long index )
{
	IEIterator<Member>* ie = this->members->entries();
	{
		for ( long i=0; i < index; i++ ) delete ie->next();

		IEntry<Member>* e = ie->next();
		Member* member = this->members->remove( e );
		{
			member->removeMember();
		}
		delete member;
	}
	delete ie;
}
~




~source/cplusplus/MembersList.cpp~
Member&
MembersList::find( const char* memberKey )
{
	IEntry<Member>* e = this->members->find( memberKey );
	Member& m = e->getValue();
	delete e;
	return m;
}
~

~source/cplusplus/MembersList.cpp~
IIterator<Member>*
MembersList::values()
{
	return this->members->values();
}
~

~source/cplusplus/MembersList.cpp~
Member&
MembersList::get( long index ) throw (NoSuchElementException*)
{
	IIterator<Member>* it = this->values();
	for ( long i=0; i < index; i++ ) it->next();
	Member& m = it->next();
	delete it;
	return m;
}
~

~source/cplusplus/MembersList.cpp~
const String&
MembersList::getStringFor( long index )
{
	if ( index < this->members->size() )
	{
		return this->get( index ).getMember();
	}
	else
	{
		return *this->empty;
	}
}
~

~source/cplusplus/MembersList.cpp~
long
MembersList::size() const
{
	return this->members->size();
}
~
