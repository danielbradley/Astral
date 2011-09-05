#ifndef ASTRAL_TOKENIZER_JAVATOKENIZER_H
#define ASTRAL_TOKENIZER_JAVATOKENIZER_H

#include "astral.tokenizer/SourceTokenizer.h"
#include "astral.tokenizer.h"

#include <openxds.base.h>

namespace astral {
	namespace tokenizer {
	
class JavaTokenizer : public SourceTokenizer
{
public:
	         JavaTokenizer( const openxds::base::String& location ) : SourceTokenizer( location ) {};
	         JavaTokenizer( openxds::util::ITextTokenizer* aTextTokenizer ) : SourceTokenizer( aTextTokenizer ) {};
	virtual ~JavaTokenizer() {};

protected:
	virtual bool  isKeyword( const openxds::base::String& word ) const;
	virtual bool isModifier( const openxds::base::String& word ) const;
	virtual bool     isType( const openxds::base::String& word ) const;
};
	
};};

#endif
