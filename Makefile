CC = clang 
CFLAGS = -Wall -Wextra -Werror -Wpedantic -Wconversion -Wsign-conversion -Wswitch -Wstrict-overflow
CFLAGS += -Wundef -Wunused -Wmissing-field-initializers -Wimplicit-fallthrough -Wdeprecated -std=c23 -ggdb  
CFLAGS += -fstack-protector-all -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined
CFLAGS += -O2 -Iinc

test: test_acc

test_acc: tests/acc.c
	$(CC) $(CFLAGS) tests/acc.c -o acc
	./acc
	python3 tests/acc.py

main: src/main.c
	$(CC) $(CFLAGS) src/main.c -o main

run: main
	./main