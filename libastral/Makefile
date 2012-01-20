all: quasi maxtext

dirs:
	mkdir -p documentation/{html,pdf,tex}

quasi: dirs
	quasi -f . source/mtx/*mtx*

maxtext:
	max2html --out documentation/html/index           source/mtx/{*mtx.txt,20_Classes.mtx.cpp}
	max2html --out documentation/html/Astral          source/mtx/??_*.mtx*
	max2html --out documentation/html/AstralFactory   source/mtx/AstralFactory.mtx*
	max2html --out documentation/html/SymbolDB        source/mtx/SymbolDB.mtx*
	max2html --out documentation/html/CompilationUnit source/mtx/*CompilationUnit.mtx*
	max2html --out documentation/html/CodeBase        source/mtx/*CodeBase.mtx*

build: all
	build --force

buildforce:
	build --force

buildforceall:
	build --force --all
