CC = gcc

flags = -std=c99 -fstack-protector-all -g -Wall -Werror

obj = regalloc.o list.o stack.o

regalloc: $(obj) 
	$(CC) $(obj) -o regalloc -lm


regalloc.o: regalloc.c  
	$(CC) $(flags) -c regalloc.c


list.o: list.c 
	$(CC) $(flags) -c list.c

stack.o: stack.c 
	$(CC) $(flags) -c stack.c
	
clean: 
	rm *.o regalloc