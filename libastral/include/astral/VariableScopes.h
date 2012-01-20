#ifndef ASTRAL_VARIABLESCOPES_H
#define ASTRAL_VARIABLESCOPES_H

#include <astral.tokenizer/SourceToken.h>

#include <openxds/Object.h>
#include <openxds.adt.h>
#include <openxds.base.h>

namespace astral {

class VariableScopes : openxds::Object {

private:       openxds::adt::ISequence<openxds::adt::IDictionary<openxds::base::String> >* scopes;
private: const openxds::adt::ITree<astral::tokenizer::SourceToken>& ast;

public:              VariableScopes( const openxds::adt::ITree<astral::tokenizer::SourceToken>& ast );
public:             ~VariableScopes();
public: void                  reset();
public: void               addScope();
public: void   removeInnermostScope();
public: void    addMethodParameters( const openxds::adt::IPosition<astral::tokenizer::SourceToken>& p );
public: void addVariableDeclaration( const openxds::adt::IPosition<astral::tokenizer::SourceToken>& p );

public: openxds::base::String* searchForTypeOfName( const char* name ) const;
public: openxds::base::String* searchForTypeOfName( const openxds::base::String& name ) const;

};};

#endif

