#ifndef ASTRAL_AST_ASTHELPER_H
#define ASTRAL_AST_ASTHELPER_H

#include "astral.ast.h"
#include "astral.tokenizer.h"

#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>

using astral::tokenizer::SourceToken;
using openxds::adt::IPosition;

namespace astral {
	namespace ast {
	
class ASTHelper : public openxds::Object
{
private:
	AST& ast;

public:
	ASTHelper( AST& anAST );

	virtual IPosition<SourceToken>* insertPackageAST( const AST& packageAST );
	virtual IPosition<SourceToken>*  insertImportAST( const AST& importAST  );
	virtual IPosition<SourceToken>*   insertClassAST( const AST& classAST   );
	virtual IPosition<SourceToken>*  insertMemberAST( const AST& memberAST  );
	virtual IPosition<SourceToken>*  insertMethodAST( const AST& methodAST  );
	
	virtual void deleteSubtree( IPosition<SourceToken>* p );
};
	
};};

#endif
