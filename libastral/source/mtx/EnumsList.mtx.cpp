

~!include/astral/EnumsList.h~
#ifndef ASTRAL_ENUMSLIST_H
#define ASTRAL_ENUMSLIST_H

#include <astral.h>
#include <astral.ast.h>
#include <astral.tokenizer.h>
#include <openxds.adt.h>
#include <openxds.base.h>

using astral::tokenizer::SourceToken;
using openxds::adt::IDictionary;
using openxds::adt::IPosition;
using openxds::adt::ISequence;

namespace astral {

class EnumsList : openxds::Object
{
private:
	CompilationUnit& cu;

	ISequence<IPosition<SourceToken> >* enumPositions;
	IDictionary<Enum>*                           dict;

public:
	         EnumsList( CompilationUnit& cu );
	virtual ~EnumsList();

	virtual       ISequence<IPosition<SourceToken> >& getEnumPositions()                    { return *this->enumPositions; }
	virtual       IDictionary<Enum>&                          getEnums()                    { return *this->dict; }
	virtual const IDictionary<Enum>&                          getEnums()              const { return *this->dict; }
	virtual const String&                                  getStringFor( long index ) const;
	virtual long                                                  size()              const;

	virtual void initialise();
};

};

#endif
~

~!source/cplusplus/EnumsList.cpp~
#include "astral/Enum.h"
#include "astral/EnumsList.h"

#include <openxds.adt/IDictionary.h>
#include <openxds.adt/IPosition.h>
#include <openxds.adt/ISequence.h>
#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt.std/Sequence.h>
#include <openxds.base/String.h>

using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;

EnumsList::EnumsList( CompilationUnit& cu ) : cu( cu )
{
	this->enumPositions = new Sequence<IPosition<SourceToken> >();
	this->dict          = new Dictionary<Enum>();
}

EnumsList::~EnumsList()
{
	delete this->enumPositions;
	delete this->dict;
}

const String&
EnumsList::getStringFor( long index ) const
{
	const String* str = &String::emptyString();

	long len = this->dict->size();
	const IIterator<Enum>* it = this->dict->values();
	for ( long i=0; i < len; i++ )
	{
		const Enum& enumeration = it->next();
		if ( index == i )
		{
			str = &enumeration.getDeclaration();
		}
	}
	delete it;

	return *str;
}

long
EnumsList::size() const
{
	return this->dict->size();
}

void
EnumsList::initialise()
{
	IIterator<IPosition<SourceToken> >* it = this->enumPositions->elements();
	while ( it->hasNext() )
	{
		Enum* anEnum = new Enum( this->cu, it->next() );
		this->dict->insert( anEnum->getName().getChars(), anEnum );
	}
	delete it;
}
~
