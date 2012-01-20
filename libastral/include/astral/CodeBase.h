#ifndef ASTRAL_CODEBASE_H
#define ASTRAL_CODEBASE_H

#include "astral.h"
#include <openxds.base.h>
#include <openxds.adt.h>
#include <openxds/Object.h>

namespace astral {

class CodeBase : openxds::Object {

private:
	openxds::adt::IDictionary<CompilationUnit>* files; // filepath -> compilation unit
	SymbolDB* symbolDB;

public:
	CodeBase();
	virtual ~CodeBase();
	
	virtual       void                                          addBaseDirectory( const char* path );
	virtual       void                                             addSourceFile( const char* path );
	virtual       void                                            refreshImports();
	
//	virtual const CompilationUnit&                            getCompilationUnit( const openxds::base::String& methodSignature ) const;
	virtual const CompilationUnit&                            getCompilationUnit( const MethodSignature& aMethodSignature ) const;

	virtual       MemberSignature*                       completeMemberSignature( const char* cls, const char* member ) const;
	virtual       MethodSignature*                       completeMethodSignature( const char* cls, const char* method, const char* parameters ) const;
	virtual       bool                                              containsType( const char* fqType ) const;

	virtual       openxds::adt::IIterator<CompilationUnit>*  getCompilationUnits();
	virtual const openxds::adt::IIterator<CompilationUnit>*  getCompilationUnits() const;

	virtual       openxds::adt::IDictionary<CompilationUnit>&           getFiles()       { return *this->files; }
	virtual const openxds::adt::IDictionary<CompilationUnit>&           getFiles() const { return *this->files; }

	virtual       SymbolDB&                                          getSymbolDB()       { return *this->symbolDB; }
	virtual const SymbolDB&                                          getSymbolDB() const { return *this->symbolDB; }
};
    
};

#endif

