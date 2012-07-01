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
	openxds::base::String*                                 location;
	openxds::adt::ITree<astral::tokenizer::SourceToken>*        ast;
	bool                                                      valid;
	openxds::adt::ISequence<astral::tokenizer::SourceToken>* indent;

public:
	         AST();
	virtual ~AST();

	virtual void clearTree();
	virtual void parseFile( const char* aLocation );
	virtual void parseString( const openxds::base::String& content );
	virtual void parseFromTokenizer( astral::tokenizer::SourceTokenizer& tokenizer );

	//virtual void replaceSubtree( openxds::adt::IPosition<astral::tokenizer::SourceToken>& p, const AST& ast );
	virtual void adjustOffsets( openxds::adt::IPosition<astral::tokenizer::SourceToken>& parent );

	virtual AST* copySubtree(       openxds::adt::IPosition<astral::tokenizer::SourceToken>& p );
	virtual AST* copySubtree( const openxds::adt::IPosition<astral::tokenizer::SourceToken>& p ) const;

	void   storeIndent();
	openxds::base::String* getIndent() const;

	void unstoreIndent();

	openxds::adt::IPosition<astral::tokenizer::SourceToken>* findFirstMethodPosition();
	
	openxds::adt::ITree<astral::tokenizer::SourceToken>& getTree() const;

	virtual void recount( openxds::adt::IPosition<astral::tokenizer::SourceToken>& p );
	
	virtual bool isValid() const;
	
private:
	void addWhitespaceTokensToIndent();
	bool matchAndRemoveIndentFromLines( openxds::adt::IPosition<astral::tokenizer::SourceToken>& p, bool extractMatch );
	//bool addIndentAfterNewlines( openxds::adt::IPosition<astral::tokenizer::SourceToken>& p, bool insertIndent );


};
	
};};

#endif
