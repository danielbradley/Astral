#ifndef ASTRAL_TOKENIZER_IXTOKENIZER_H
#define ASTRAL_TOKENIZER_IXTOKENIZER_H

#include "astral.tokenizer/SourceTokenizer.h"
#include "astral.tokenizer.h"

#include <openxds.base.h>

namespace astral {
	namespace tokenizer {
	
class IxTokenizer : public SourceTokenizer
{
public:
	         IxTokenizer()                                                : SourceTokenizer()                 {};
	         IxTokenizer( const openxds::base::String& location )         : SourceTokenizer( location )       {};
	         IxTokenizer( openxds::util::ITextTokenizer* aTextTokenizer ) : SourceTokenizer( aTextTokenizer ) {};
	         IxTokenizer( openxds::io::Reader* aReader )                  : SourceTokenizer( aReader )        {};
	virtual ~IxTokenizer()                                                                                    {};

	virtual bool       isKeyword( const openxds::base::String& word ) const;
	virtual bool      isModifier( const openxds::base::String& word ) const;
	virtual bool isPrimitiveType( const openxds::base::String& word ) const;

	virtual		  SourceToken*  parseAnnotation( openxds::base::String* word );

	
	static bool  IsPrimitiveType( const openxds::base::String& word );
};
	
};};

#endif
