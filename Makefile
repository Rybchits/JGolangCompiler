build_flex:
	@flex lexer.lpp

build_bison:
	@bison -d parser.y -t --verbose

build_exec:
# -ferror-limit=100
	g++ -std=c++17 -ferror-limit=100 -o parser main.cpp ast.cpp ./utils/dot_visitor.cpp

build_all:
	@bison -d parser.y -t --verbose
	@flex lexer.lpp
# -ferror-limit=100
	@g++ -std=c++17 -ferror-limit=100 -o parser main.cpp ast.cpp ./semantic/visitors/loops_visitor.cpp ./semantic/semantic.cpp ./semantic/visitors/type_check_visitor.cpp ./semantic/java_entity.cpp ./semantic/built_in_functions.cpp ./utils/dot_visitor.cpp

run:
	./parser ./tests/types/anonymous_struct.go

clean:
	@rm -f *.o *~ lex.yy.cc parser.tab.c parser.tab.h parser.output parser