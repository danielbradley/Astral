#include "astrix/AstrixTour.h"
#include "astrix/Translator.h"

#include <astral.ast/AST.h>
#include <openxds.io/IO.h>

using namespace astrix;
using namespace astral::ast;
using namespace openxds::io;

Translator::Translator( const char* filepath )
{
	AST ast;
	ast.parseFile( filepath );
	
	AstrixTour tour( ast.getTree(), IO::out() );
	tour.doGeneralTour();
}