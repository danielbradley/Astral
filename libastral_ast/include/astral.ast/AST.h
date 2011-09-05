#ifndef ASTRAL_AST_AST_H
#define ASTRAL_AST_AST_H

#include "astral.ast.h"
#include <astral.tokenizer.h>

#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>

namespace astral {
	namespace ast {
	
class AST : public openxds::Object
{
private:
	openxds::base::String* location;
	openxds::adt::ITree<astral::tokenizer::SourceToken>* ast;

public:
	         AST( const char* aLocation );
	virtual ~AST();

	virtual void parseFile();
	
	openxds::adt::ITree<astral::tokenizer::SourceToken>& getTree() const;
	
};
	
};};

#endif
