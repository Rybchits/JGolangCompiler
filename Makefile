build_flex:
	@flex --outfile=lexer/lex.yy.cc lexer/lexer.lpp

build_bison:
	@bison --defines=parser/parser.tab.h --output=parser/parser.tab.cpp parser/parser.y -t --verbose

build_all:
	@bison --defines=parser/parser.tab.h --output=parser/parser.tab.cpp parser/parser.y -t --verbose
	@flex --outfile=lexer/lex.yy.cc lexer/lexer.lpp
# -ferror-limit=100
	@g++ -std=c++17 -o compiler main.cpp ast.cpp parser/*.cpp entities/*.cpp semantic/*.cpp semantic/visitors/*.cpp utils/*.cpp codegen/*.cpp

clean:
	@rm -f *.o *~ lexer/lex.yy.cc parser/parser.tab.cpp parser/parser.tab.h parser/parser.output compiler