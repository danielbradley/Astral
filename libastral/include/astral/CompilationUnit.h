#ifndef ASTRAL_COMPILATIONUNIT_H
#define ASTRAL_COMPILATIONUNIT_H

#include <astral.ast/AST.h>
#include <astral.tokenizer.h>
#include <openxds.io.h>
#include <openxds.adt.h>
#include <openxds.base.h>
#include <openxds/Object.h>

namespace astral {
	
class CompilationUnit : openxds::Object
{
private:
	astral::ast::AST* ast;
	openxds::base::String* packageName;
	openxds::base::String* className;
	openxds::base::String* extendsClass;
	openxds::adt::IList<openxds::base::String>* imports;
	openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >* methods;
	openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >* members;

public:
	         CompilationUnit( const char* location );
	virtual ~CompilationUnit();
	virtual void initialise();
	
	virtual void registerSymbols( openxds::adt::IDictionary<openxds::adt::IEntry<CompilationUnit> >& symbols, const openxds::adt::IEntry<CompilationUnit>& e ) const;
	
	virtual openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >&
		getMethods() const;
	virtual openxds::adt::IDictionary<openxds::adt::IPosition<astral::tokenizer::SourceToken> >&
		getMembers() const;

	virtual openxds::base::String*   resolveMemberType( const char* name ) const;
	virtual openxds::base::String*   resolveMethodType( const char* name ) const;
	
	virtual openxds::base::String&                    getNamespace() const;
	virtual openxds::base::String&                         getName() const;
	virtual openxds::base::String&                   getSuperclass() const;
	virtual openxds::adt::IList<openxds::base::String>& getImports() const;

	virtual void printHTML( openxds::adt::IDictionary<openxds::adt::IEntry<CompilationUnit> >& symbols,
	                        openxds::adt::IDictionary<openxds::base::String>& importedTypes,
							openxds::io::PrintWriter& writer ) const;
	
	virtual openxds::base::String* toXML( openxds::adt::IDictionary<openxds::adt::IEntry<CompilationUnit> >& iSymbols, openxds::adt::IDictionary<openxds::base::String>& iTypes );

	
	virtual void printMethods() const;
	virtual void printMembers() const;
};
	
};

#endif