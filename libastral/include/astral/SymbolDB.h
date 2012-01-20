#ifndef ASTRAL_SYMBOLDB_H
#define ASTRAL_SYMBOLDB_H

#include "astral.h"
#include <openxds.base.h>
#include <openxds.adt.h>
#include <openxds/Object.h>
#include <openxds.exceptions/NoSuchElementException.h>

namespace astral {

class SymbolDB : openxds::Object {

private: openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >* symbols;

private: openxds::adt::IDictionary<openxds::base::String>* name2namespace;
private: openxds::adt::IDictionary<openxds::base::String>* namespace2name;

public: SymbolDB();

public: virtual ~SymbolDB();

public: void registerCU( const openxds::adt::IEntry<CompilationUnit>& anEntry );

public: void deregisterCU( const openxds::adt::IEntry<CompilationUnit>& anEntry );

public: openxds::adt::IDictionary<openxds::base::String>*
          importedTypes( const openxds::adt::IList<openxds::base::String>& imports ) const;

public: openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >*
          importedSymbols( const openxds::adt::IList<openxds::base::String>& imports ) const;

public: openxds::adt::IDictionary<const openxds::adt::IEntry<CompilationUnit> >& getSymbols() const { return *this->symbols; }
public: openxds::adt::IDictionary<openxds::base::String>&                     getN2NS() const { return *this->name2namespace; }
public: openxds::adt::IDictionary<openxds::base::String>&                     getNS2N() const { return *this->namespace2name; }

public: bool                                                             containsType( const char* fqType ) const;

public: const CompilationUnit& getCompilationUnitForSymbolPrefix( const char* prefix ) const
throw (openxds::exceptions::NoSuchElementException*);

};};

#endif

