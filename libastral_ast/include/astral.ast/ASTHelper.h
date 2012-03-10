#ifndef ASTRAL_AST_ASTHELPER_H
#define ASTRAL_AST_ASTHELPER_H

#include "astral.ast.h"
#include "astral.tokenizer.h"

#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>

using astral::tokenizer::SourceToken;
using openxds::adt::IDictionary;
using openxds::adt::IPosition;
using openxds::base::String;

namespace astral {
	namespace ast {
	
class ASTHelper : public openxds::Object
{
private:
	AST& ast;

public:
	ASTHelper( AST& anAST );

	virtual IPosition<SourceToken>* insertPackageAST( const AST& packageAST );
	virtual IPosition<SourceToken>*  insertImportAST( const AST&  importAST );
	virtual IPosition<SourceToken>*   insertClassAST( const AST&   classAST );
	virtual IPosition<SourceToken>*  insertMemberAST( const AST&  memberAST );
	virtual IPosition<SourceToken>*  insertMethodAST( const AST&  methodAST );

	virtual void          reorder( IDictionary<IPosition<SourceToken> >& positions );
	virtual void replaceMethodAST( IPosition<SourceToken>& p, const AST& methodAST );
	virtual AST*    removeSubtree( IPosition<SourceToken>* p );
	virtual void    deleteSubtree( IPosition<SourceToken>* p );

	virtual String* toHTMLString();
	virtual String* toHTMLString( IPosition<SourceToken>& p );
};
	
};};

#endif
