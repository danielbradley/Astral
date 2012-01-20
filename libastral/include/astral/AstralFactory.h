#ifndef ASTRAL_ASTRALFACTORY_H
#define ASTRAL_ASTRALFACTORY_H

#include "astral.h"

#include <openxds.adt.h>
#include <openxds.io.h>
#include <openxds/Object.h>

class AstralFactory : openxds::Object
{
public:
	static astral::CodeBase*
	createAstral(
		const openxds::adt::IList<openxds::io::Path>& pathList,
		const char*                                   extension
	);

	static astral::CodeBase*
	createAstralExport(
		const openxds::adt::IList<openxds::io::Path>& pathList,
		const char*                                   extension
	);

	static void
	recursivelySearch(
		           astral::CodeBase& astral,
		const openxds::io::Path&   path,
		const char*                extension );
};

#endif

