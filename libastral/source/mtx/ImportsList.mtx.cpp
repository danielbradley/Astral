
~!include/astral/ImportsList.h~
#ifndef ASTRAL_IMPORTSLIST_H
#define ASTRAL_IMPORTSLIST_H
~

~include/astral/ImportsList.h~
#include <astral.h>
#include <astral.tokenizer.h>
#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>
~

~include/astral/ImportsList.h~
using astral::Import;
using astral::tokenizer::SourceToken;
using openxds::adt::IIterator;
using openxds::adt::IList;
using openxds::adt::IPosition;
using openxds::adt::ISequence;
using openxds::base::String;
~

~include/astral/ImportsList.h~
namespace astral {
~

~include/astral/ImportsList.h~
class ImportsList : openxds::Object
{
private:
	CompilationUnit&        cu;
	ISequence<Import>* imports;
	String*              empty;

public:
	 ImportsList( CompilationUnit& aCompilationUnit );
	~ImportsList();

	void initialise( IList<IPosition<SourceToken> >& importPositions );

	void addBlank();
	void remove( long i );

	Import& get( long i );

	const String& getStringFor( long rowIndex );

	      IIterator<Import>* iterator();
	const IIterator<Import>* iterator() const;
	long                         size() const;
};
~

~include/astral/ImportsList.h~
}; // end namespace
~

~include/astral/ImportsList.h~
#endif
~




~!source/cplusplus/ImportsList.cpp~
#include "astral/ImportsList.h"
#include "astral/Import.h"

#include <openxds.adt/IIterator.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/String.h>
~

~source/cplusplus/ImportsList.cpp~
using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
~

~source/cplusplus/ImportsList.cpp~
ImportsList::ImportsList( CompilationUnit& aCompilationUnit ) : cu( aCompilationUnit )
{
	this->imports = new Sequence<Import>();
	this->empty   = new String();
}
~

~source/cplusplus/ImportsList.cpp~
void
ImportsList::initialise( IList<IPosition<SourceToken> >& importPositions )
{	
	IIterator<IPosition<SourceToken> >* it = importPositions.elements();
	while ( it->hasNext() )
	{
		this->imports->insertLast( new Import( this->cu, it->next() ) );
	}
	delete it;
	this->addBlank();
}
~

~source/cplusplus/ImportsList.cpp~
ImportsList::~ImportsList()
{
	delete this->imports;
}
~

~source/cplusplus/ImportsList.cpp~
void
ImportsList::addBlank()
{
	this->imports->insertLast( new Import( this->cu ) );
}
~

~source/cplusplus/ImportsList.cpp~
void
ImportsList::remove( long i )
{
	Import* import = this->imports->remove( this->imports->atRank( i ) );
	import->removeFromCU();
	delete import;
}
~

~source/cplusplus/ImportsList.cpp~
Import&
ImportsList::get( long i )
{
	return this->imports->get( i );
}
~

~source/cplusplus/ImportsList.cpp~
const String&
ImportsList::getStringFor( long i )
{
	if ( i < this->imports->size() )
	{
		return this->imports->get( i ).getContent();
	}
	else
	{
		return *this->empty;
	}
}
~

~source/cplusplus/ImportsList.cpp~
IIterator<Import>*
ImportsList::iterator()
{
	return this->imports->elements();
}
~

~source/cplusplus/ImportsList.cpp~
const IIterator<Import>*
ImportsList::iterator() const
{
	return this->imports->elements();
}
~

~source/cplusplus/ImportsList.cpp~
long
ImportsList::size() const
{
	return this->imports->size();
}
~

