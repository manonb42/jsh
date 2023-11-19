CC = gcc
CFLAGS = -g -Wall
DEPS = jsh.h
O =  jsh.o
EXEC = jsh

build : $(O)
	gcc -Wall -o jsh jsh.c -lreadline

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

run : build
	./$(EXEC)
	
clean :
	rm -rf $(EXEC) *.o

leak :
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC) 
