build_flex:
	@flex -o lexer.cpp lexer.lpp

build_bison:
	@bison -d -o parser.cpp parser.y -t --verbose

build_exec:
# -ferror-limit=100
	g++ -std=c++17 -ferror-limit=100 -o parser main.cpp ast.cpp ./utils/dot_visitor.cpp

build_all:
	@bison -d -o parser.cpp parser.y -t --verbose
	@flex -o lexer.cpp lexer.lpp
# -ferror-limit=100
	@g++ -std=c++17 -ferror-limit=100 -o parser main.cpp ast.cpp ./utils/dot_visitor.cpp

clean:
	@rm -f *.o *~ lexer.cpp parser.cpp parser.hpp parser.output parser