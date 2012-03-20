#ifndef ASTRAL_TOKENIZER_SOURCETOKENIZER_H
#define ASTRAL_TOKENIZER_SOURCETOKENIZER_H

#include "astral.tokenizer/SourceToken.h"
#include "astral.tokenizer.h"

#include <openxds.adt/ISequence.h>
#include <openxds.util.h>
#include <openxds.io.exceptions/IOException.h>
#include <openxds.base.h>
#include <openxds.exceptions/NoSuchElementException.h>
#include <openxds/Object.h>

namespace astral {
	namespace tokenizer {
	
class SourceTokenizer : public openxds::Object
{
private:
	openxds::util::ITextTokenizer* tt;
	openxds::adt::ISequence<SourceToken>* tokenQueue;

public:
	         SourceTokenizer( const openxds::base::String& location );
	         SourceTokenizer( openxds::util::ITextTokenizer* aTextTokenizer );
	         SourceTokenizer( openxds::io::Reader* reader );
	virtual ~SourceTokenizer();

	virtual       SourceToken*     nextToken() throw (openxds::io::exceptions::IOException*,openxds::exceptions::NoSuchElementException*);
	virtual const SourceToken& peekNextToken() throw (openxds::io::exceptions::IOException*,openxds::exceptions::NoSuchElementException*);
	virtual       bool         hasMoreTokens() throw (openxds::io::exceptions::IOException*);

	virtual void               pushback( SourceToken* token );

	virtual       SourceToken::TokenType sneakyPeek();

protected:
	virtual       SourceToken*  parseWhitespace( const openxds::util::ITextToken& textToken );
	virtual		  SourceToken*   parseWordToken( openxds::base::String* word );
	virtual		  SourceToken* parseSymbolToken( openxds::base::String* word );
	virtual		  SourceToken* parseLineComment( openxds::base::String* word );
	virtual		  SourceToken*     parseComment( openxds::base::String* word );
	virtual		  SourceToken*       parseQuote( openxds::base::String* word );
	virtual		  SourceToken* parseDoubleQuote( openxds::base::String* word );
	virtual		  SourceToken*  parseAnnotation( openxds::base::String* word );

	
	virtual       bool  isKeyword( const openxds::base::String& word ) const { return false; };
	virtual       bool isModifier( const openxds::base::String& word ) const { return false; };
	virtual       bool     isType( const openxds::base::String& word ) const { return false; };
};
	
};};

#endif
