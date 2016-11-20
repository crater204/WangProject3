CC=gcc -g -Wall
CFLAGS=-I.
DEPS=main.h
OBJ=main.o process.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

a.out: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o a.out
