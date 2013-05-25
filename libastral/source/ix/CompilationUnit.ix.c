namespace astral;

public class CompilationUnit extends Object
{
	@codebase : const Codebase&;

	@ast         : AST;
	
	@location : string;
	@package  : Package;
	@class    : Class;
	@fq_class : FQClass;
	



	@files    : Dictionary<CompilationUnit>*;
	@symbolDB : SymbolDB;
}

