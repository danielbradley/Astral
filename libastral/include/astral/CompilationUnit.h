#ifndef ASTRAL_COMPILATIONUNIT_H
#define ASTRAL_COMPILATIONUNIT_H

#include <astral.h>
#include <astral.ast/AST.h>
#include <astral.tokenizer.h>
#include <openxds.io.h>
#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>

namespace astral {

class CompilationUnit : openxds::Object {

private:
	astral::ast::AST* ast;
	openxds::base::String* location;
	openxds::base::String* packageName;
	openxds::base::String* className;
	openxds::base::String* fqName;
	openxds::base::String* extendsClass;
	openxds::adt::IList<openxds::base::String>* imports;
	openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >* methods;
	openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >* members;

	openxds::adt::IDictionary<openxds::base::String>* importedTypes;

public:
	         CompilationUnit( const char* location );
	virtual ~CompilationUnit();
	virtual void initialise();

public:
	virtual       void                                   resetImportedTypes( const SymbolDB& symbolDB );

	virtual const   astral::ast::AST&                                getAST() const { return *this->ast;          }
	virtual       openxds::base::String&                       getNamespace() const { return *this->packageName;  }
	virtual       openxds::base::String&                            getName() const { return *this->className;    }
	virtual       openxds::base::String&                          getFQName() const { return *this->fqName;       }
	virtual       openxds::base::String&                      getSuperclass() const { return *this->extendsClass; }
	virtual       openxds::adt::IList<openxds::base::String>&    getImports() const { return *this->imports;      }

	virtual openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >&
		getMethods() const { return *this->methods; }
	virtual openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >&
		getMembers() const { return *this->members; }

public:

	virtual void registerSymbols( openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& symbols, const openxds::adt::IEntry<CompilationUnit>& e ) const;

	virtual openxds::base::String* resolveFQTypeOfName( const char* name, const VariableScopes& scopes ) const;
	virtual openxds::base::String* resolveFQTypeOfType( const char* type ) const;
	virtual openxds::base::String*   resolveTypeOfName( const char* name, const VariableScopes& scopes ) const;
	virtual openxds::base::String*   resolveMemberType( const char* name ) const;
	virtual openxds::base::String*   resolveMethodType( const char* name ) const;
	
	virtual openxds::base::String*   resolveMethodCallParametersToTypes( const openxds::base::String& parameters, const VariableScopes& scopes ) const;


	virtual openxds::base::String*      resolveMethodCallReturnType( const CodeBase& codebase, const openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, const openxds::adt::IPosition<astral::tokenizer::SourceToken>& methodcall, const VariableScopes& scopes, const openxds::base::String& invocationClass ) const;
	virtual openxds::base::String*   resolveMethodCallArgumentTypes( const CodeBase& codebase, const openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, const openxds::adt::IPosition<astral::tokenizer::SourceToken>& methodcall, const VariableScopes& scopes ) const;
	virtual openxds::base::String*           recurseMethodArguments( const CodeBase& codebase, const openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, const openxds::adt::IPosition<astral::tokenizer::SourceToken>& methodcall, const VariableScopes& scopes ) const;
	virtual openxds::base::String*            recurseMethodArgument( const CodeBase& codebase, const openxds::adt::ITree<astral::tokenizer::SourceToken>& tree, const openxds::adt::IPosition<astral::tokenizer::SourceToken>& methodcall, const VariableScopes& scopes ) const;

	virtual openxds::base::String* retrieveMethodContent( const MethodSignature& aMethodSignature ) const;

//	virtual void printHTML( openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& symbols,
//	                        openxds::adt::IDictionary<openxds::base::String>& importedTypes,
//							openxds::io::PrintWriter& writer ) const;
//	
//	virtual openxds::base::String* toXML( openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& iSymbols, openxds::adt::IDictionary<openxds::base::String>& iTypes ) const;
//
//	
//	virtual void printMethods() const;
//	virtual void printMembers() const;

	virtual const openxds::base::String& getLocation() const { return *this->location; }

};

};

#endif

