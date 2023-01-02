build_flex:
	@flex lexer.lpp

build_bison:
	@bison -d parser.y -t --verbose

build_all:
	@bison -d parser.y -t --verbose
	@flex lexer.lpp
# -ferror-limit=100
	@g++ -std=c++17 -ferror-limit=100 -o parser main.cpp ast.cpp ./semantic/visitors/statements_visitor.cpp ./semantic/semantic.cpp ./semantic/visitors/types_visitor.cpp ./semantic/entities.cpp ./semantic/built_in_functions.cpp ./utils/dot_visitor.cpp ./codegen/constant.cpp ./codegen/codegen.cpp ./codegen/constant_visitor.cpp

run:
	./parser ./tests/types/anonymous_struct.go

clean:
	@rm -f *.o *~ lex.yy.cc parser.tab.c parser.tab.h parser.output parser