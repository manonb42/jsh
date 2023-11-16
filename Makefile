CC = gcc
CFLAGS = -g -Wall
DEPS = jsh.h
O =  jsh.o
EXEC = jsh

all : $(EXEC)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< 

app : $(O)
	$(CC) $(CFLAGS) -o $@ $^

run :
	./$(EXEC)
	
clean :
	rm -rf $(EXEC) *.o

leak :
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC) 
