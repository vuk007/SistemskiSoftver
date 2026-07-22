TARGET        = src/asembler
LINKER_TARGET = src/linker

LEX_FILE   = misc/flex.lex
LEX_OUTPUT = src/lex.cpp
PARSER_FILE   = misc/bison.y
PARSER_OUTPUT = src/parser.cpp
PARSER_HEADER = inc/parser.hpp

# fajlovi za asembler (bez lex.cpp/parser.cpp, oni su generisani)
SOURCES = src/assembler.cpp
OBJECTS = $(SOURCES:.cpp=.o) src/lex.o src/parser.o

# fajlovi za linker
LINKER_SOURCES = src/linker.cpp
LINKER_OBJECTS = $(LINKER_SOURCES:.cpp=.o)

CXXFLAGS = -Iinc -std=c++17 -Wall -MMD -MP

all: $(TARGET) $(LINKER_TARGET)

$(TARGET): $(OBJECTS)
	g++ $(OBJECTS) -o $(TARGET)

$(LINKER_TARGET): $(LINKER_OBJECTS)
	g++ $(LINKER_OBJECTS) -o $(LINKER_TARGET)

# generisi lex.cpp (zavisi i od parser.hpp zbog #include "parser.hpp")
$(LEX_OUTPUT): $(LEX_FILE) $(PARSER_HEADER)
	flex -o $(LEX_OUTPUT) $(LEX_FILE)

# bison generise i .cpp i .hpp iz istog poziva
$(PARSER_OUTPUT) $(PARSER_HEADER): $(PARSER_FILE)
	bison -d --defines=$(PARSER_HEADER) -o $(PARSER_OUTPUT) $(PARSER_FILE)

# generalno pravilo: svaki .cpp u src/ -> .o
src/%.o: src/%.cpp
	g++ $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(LEX_OUTPUT) $(PARSER_OUTPUT) $(PARSER_HEADER) $(TARGET) $(LINKER_TARGET) src/*.o src/*.d

rebuild: clean all

-include $(OBJECTS:.o=.d) $(LINKER_OBJECTS:.o=.d)