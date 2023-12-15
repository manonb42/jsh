CC = gcc
CFLAGS = -g -Wall
EXEC = redirectionner
TEST1 = test1.sh
TEST2 = test2.sh

.PHONY: build clean leak test1 test2

build: clean $(EXEC)

test1 : build
	./$(TEST1)

test2 : build
	./$(TEST2)

$(EXEC): redirectionner.c
	$(CC) -Wall -o $@ $^

clean :
	rm -rf $(EXEC) alocp error

leak :
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC) 
