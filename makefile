TARGET = src/asembler

LEX_FILE   = misc/flex.lex
LEX_OUTPUT = src/lex.cpp

PARSER_FILE   = misc/bison.y
PARSER_OUTPUT = src/parser.cpp
PARSER_HEADER = inc/parser.hpp

all: $(TARGET)

# izvrsni fajl nastaje linkovanjem OBA .cpp fajla zajedno
$(TARGET): $(LEX_OUTPUT) $(PARSER_OUTPUT)
	g++ -Iinc $(LEX_OUTPUT) $(PARSER_OUTPUT) -o $(TARGET)

# lex.cpp zavisi i od parser.hpp (zbog #include "parser.hpp" i tokena/yylval)
$(LEX_OUTPUT): $(LEX_FILE) $(PARSER_HEADER)
	flex -o $(LEX_OUTPUT) $(LEX_FILE)

# bison generise i .cpp i .hpp iz istog poziva
$(PARSER_OUTPUT) $(PARSER_HEADER): $(PARSER_FILE)
	bison -d --defines=$(PARSER_HEADER) -o $(PARSER_OUTPUT) $(PARSER_FILE)

clean:
	rm -f $(LEX_OUTPUT) $(PARSER_OUTPUT) $(PARSER_HEADER) $(TARGET)

rebuild: clean all