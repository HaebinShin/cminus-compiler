EXEC_NAME=run.out
LEX=flex

all: pre-build $(EXEC_NAME)

pre-build:
	mkdir -p build

$(EXEC_NAME): build/main.o build/util.o build/scan.o build/lex.yy.o
	$(CC) -o $@ $? -lfl

build/%.o: src/%.c
	$(CC) -c -o $@ $?

build/lex.yy.o: src/cminus.l
	$(LEX) -t $? | $(CC) -Isrc -c -o $@ -xc -

clean:
	rm -rf build/*.o $(EXEC_NAME)