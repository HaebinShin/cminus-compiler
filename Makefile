EXEC_NAME=run.out
LEX=flex
YACC=bison
OBJS=build/main.o build/util.o build/scan.o build/parse.o build/lex.yy.o build/cminus.tab.o

all: pre-build $(EXEC_NAME)

pre-build:
	@mkdir -p build 

.PHONY: test
test: all
	./$(EXEC_NAME) sample_inputs/${INPUT_FILE}


$(EXEC_NAME): $(OBJS)
	$(CC) -o $@ $? -lfl

build/%.o: src/%.c build/cminus.tab.h
	$(CC) -Ibuild -c -o $@ $<

build/lex.yy.o: src/cminus.l build/cminus.tab.h
	$(LEX) -t $< | $(CC) -Isrc -Ibuild -c -o $@ -xc -

build/cminus.tab.o: build/cminus.tab.c
	$(CC) -Isrc -c -o $@ $<

build/cminus.tab.c build/cminus.tab.h: src/cminus.y
	bison -d $< -o build/cminus.tab.c

clean:
	rm -rf build/*.* $(EXEC_NAME)