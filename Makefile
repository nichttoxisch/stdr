CC = /usr/local/opt/llvm/bin/clang-21 
CFLAGS = -Wall -Wextra -Werror -Wconversion -Wswitch -Wstrict-overflow
CFLAGS += -Wundef -Wunused -Wmissing-field-initializers -Wimplicit-fallthrough -std=c23 -ggdb  
CFLAGS += -fstack-protector-all -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined
CFLAGS += -O0 -Iinc

test: test_acc

test_acc: tests/acc.c
	$(CC) $(CFLAGS) tests/acc.c -o acc
	./acc 
	python3 tests/acc.py

regex: tests/regex.c
	$(CC) $(CFLAGS) tests/regex.c -o regex
	./regex

main: src/main.c
	$(CC) $(CFLAGS) src/main.c -o main
	./main
