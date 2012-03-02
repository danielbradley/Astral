all: clean quasi build

clean:
#	make -C libastral_tokenizer  clean
#	make -C libastral_ast        clean
#	make -C libastral            clean

quasi:
#	make -C libastral_tokenizer  quasi
#	make -C libastral_ast        quasi
	make -C libastral            quasi

build:
	make -C libastral           buildforce

buildall: quasi
	make -C libastral buildforceall

tools: buildall
	cd astral;    build --force
	cd classinfo; build --force
	cd java2html; build --force
	cd tokenize;  build --force
