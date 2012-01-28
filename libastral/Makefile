all: clean quasi maxtext build

clean:
	rm -rf bin
	rm -rf dep
	rm -rf documentation
	rm -rf include
	rm -rf lib
	rm -rf obj
	rm -rf source/cplusplus

dirs:
	mkdir -p documentation/{html,pdf,tex}

quasi: dirs
	quasi -f . source/mtx/*mtx*

maxtext: dirs
	max2html --out documentation/html/index           source/mtx/{*mtx.txt,20_Classes.mtx.cpp}
	max2html --out documentation/html/CodeBase        source/mtx/*CodeBase.mtx*
	max2html --out documentation/html/CompilationUnit source/mtx/*CompilationUnit.mtx*
	max2html --out documentation/html/Method          source/mtx/Method.mtx*
	max2html --out documentation/html/SymbolDB        source/mtx/SymbolDB.mtx*

build:
	build --force

buildforce:
	build --force

buildforceall:
	build --force --all
