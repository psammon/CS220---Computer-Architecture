PROJECT = quadr

CFLAGS = -g -Wall -std=c11
LDFLAGS = 

$(PROJECT):	quadr.o main.o
		$(CC) $(LDFLAGS) quadr.o main.o -o $@

clean:
		rm -f *.o $(PROJECT) $(PROJECT).tar.gz

