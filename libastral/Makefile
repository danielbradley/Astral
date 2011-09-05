all: dirs remove maxtext semi

dirs:
	mkdir -p documentation/{html,pdf,tex}

remove:
	rm -f include/astral/AstralFactory.h
	rm -f source/cplusplus/AstralFactory.cpp

maxtext:
	max2html --out documentation/html/AstralFactory source/mtx/AstralFactory.mtx

semi:
	semi -f . source/mtx/AstralFactory.mtx
