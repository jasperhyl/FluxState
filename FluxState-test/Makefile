CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -I.

OBJS = lexer.o parser.o ast.o main.o

all: fluxc

fluxc: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

lexer.o: lexer.cpp lexer.h
parser.o: parser.cpp parser.h lexer.h ast.h
ast.o: ast.cpp ast.h
main.o: main.cpp lexer.h parser.h

clean:
	rm -f fluxc $(OBJS)

.PHONY: all clean