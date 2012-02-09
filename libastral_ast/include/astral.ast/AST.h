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
	openxds::base::String*                               location;
	openxds::adt::ITree<astral::tokenizer::SourceToken>*      ast;
	bool                                                    valid;

public:
	         AST();
	virtual ~AST();

	virtual void parseFile( const char* aLocation );
	virtual void parseString( const openxds::base::String& content );
	virtual void parseFromTokenizer( astral::tokenizer::SourceTokenizer& tokenizer );

	virtual AST* copySubtree( const openxds::adt::IPosition<astral::tokenizer::SourceToken>& p ) const;
	
	virtual void replaceSubtree( openxds::adt::IPosition<astral::tokenizer::SourceToken>& p, const AST& ast ) const;
	
	openxds::adt::ITree<astral::tokenizer::SourceToken>& getTree() const;
	
	virtual bool isValid() const;
};
	
};};

#endif
