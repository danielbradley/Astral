all: clean build

clean:
#	make -C libastral_tokenizer  clean
#	make -C libastral_ast        clean
#	make -C libastral            clean

build:
	cd libastral_tokenizer; build --force
	cd libastral_ast;       build --force

	make -C libastral buildforce

buildall: quasi
	make -C libastral buildforceall

tools: buildall
	cd astral;    build --force
	cd classinfo; build --force
	cd java2html; build --force
	cd tokenize;  build --force
