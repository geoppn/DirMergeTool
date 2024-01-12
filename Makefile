OBJS = cmpcat.o cmpFunc.o
SOURCE = cmpcat.c cmpFunc.c
OUT = cmpcat
CC = gcc
FLAGS = -g -c

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

cmpcat.o : cmpcat.c
	$(CC) $(FLAGS) cmpcat.c

cmpFunc.o : cmpFunc.c
	$(CC) $(FLAGS) cmpFunc.c

clean:
	rm -f $(OBJS) $(OUT) 