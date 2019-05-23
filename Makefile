EXEC_NAME:=run.out
LEX:=flex
YACC:=bison
CPPFLAGS+=-Isrc -Ibuild
OBJS:=$(addprefix build/, cminus.tab.o lex.yy.o main.o util.o scan.o symtab.o analyze.o)
SHELL:=/bin/bash

all: pre-build $(EXEC_NAME)

pre-build:
	@mkdir -p build 

.PHONY: test
test: all
	./$(EXEC_NAME) sample_inputs/${INPUT_FILE}

$(EXEC_NAME): $(OBJS)
	$(CC) -o $@ $^ -lfl

build/%.o: src/%.c src/globals.h
	$(CC) $(CPPFLAGS) -c -o $@ $<

src/globals.h: build/cminus.tab.h

build/lex.yy.o: build/lex.yy.c
	$(CC) $(CPPFLAGS) -c -o $@ $<

build/lex.yy.c: src/cminus.l
	$(LEX) -o $@ $<;

build/cminus.tab.o: build/cminus.tab.c
	$(CC) $(CPPFLAGS) -c -o $@ $<

build/cminus.tab.h: build/cminus.tab.c

build/cminus.tab.c: src/cminus.y
	bison -d $< -o $@

clean:
	rm -rf build/*.* $(EXEC_NAME)
