
#include "astral/Astral.h"
#include "astral/CompilationUnit.h"
#include "astral/SymbolDB.h"

#include <openxds.adt.std/Dictionary.h>
#include <openxds.adt/IEntry.h>
#include <openxds.base/String.h>

using namespace astral;
using namespace openxds::adt;
using namespace openxds::adt::std;
using namespace openxds::base;

Astral::Astral()
{
	this->files          = new Dictionary<CompilationUnit>();
	this->symbolDB       = new SymbolDB();
}

Astral::~Astral()
{
	delete this->files;
	delete this->symbolDB;
}

void
Astral::add( const String& path )
{
	CompilationUnit* cu = new CompilationUnit( path.getChars() );
	cu->initialise();

	IEntry<CompilationUnit>* entry = this->files->insert( path.getChars(), cu );
	{
		this->symbolDB->registerCU( *entry );
	}
	delete entry;
}
