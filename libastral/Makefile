all: dirs remove semi maxtext

dirs:
	mkdir -p documentation/{html,pdf,tex}

remove:
	rm -f include/astral/AstralFactory.h
	rm -f source/cplusplus/AstralFactory.cpp
	rm -f source/include/astral/SymbolDB.h
	rm -f source/cplusplus/SymbolDB.cpp

semi:
	semi -f . source/mtx/AstralFactory.mtx
	semi -f . source/mtx/SymbolDB.mtx

maxtext:
	max2html --out documentation/html/AstralFactory source/mtx/AstralFactory.mtx
	max2html --out documentation/html/SymbolDB      source/mtx/SymbolDB.mtx

