CC=gcc
RESULT=dcc
FLAGS=-g

all: *.c
	$(CC) $(FLAGS) *.c -o $(RESULT)
clean:
	rm $(RESULT)
