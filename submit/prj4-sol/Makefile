PROJECT = y86-sim
SRC_FILES = README\
	    ysim.c\
	    ysim.h\
	    main.c
CFLAGS = -g -Wall -std=c11
COURSE = cs220
LDFLAGS = -L $$HOME/$(COURSE)/lib -l cs220 -l y86
CPPFLAGS = -I $$HOME/$(COURSE)/include

$(PROJECT):	ysim.o main.o
		$(CC) ysim.o main.o $(LDFLAGS) -o $@ 

clean:
		rm -f *.o $(PROJECT) $(PROJECT).tar.gz

ysim.o: ysim.c ysim.h

