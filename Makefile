PROGRAM_NAME = 9cc
CC = gcc
CFLAGS = -g3 -O1 -Wall -Wextra -std=c11

main:
	$(CC) 9cc.c $(CFLAGS) -o $(PROGRAM_NAME)

test: 
	@make clean
	@make main
	./9cc -test
	./test.sh

format:
	clang-format 9cc.c -i

clean:
	rm -rf ./*.o ./*.s ./9cc ./tmp

