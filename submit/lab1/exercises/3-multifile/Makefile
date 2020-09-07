PROJECT = quadr
SRC_FILES = main.c\
	Makefile\
 	quadr.c\
	quadr.h\
	README \
	test.data

CFLAGS = -g -Wall -std=c11
LDFLAGS = -lm

$(PROJECT):
		quadr.o main.o
		$(CC) $(LDFLAGS) quadr.o main.o -o $@

clean:
		rm -f *.o $(PROJECT) $(PROJECT).tar.gz

depend:
		$(CC) -MM $(CPPFLAGS) *.c

dist:	$(PROJECT).tar.gz

$(PROJECT).tar.gz:	$(SRC_FILES)
			tar -cf $(PROJECT).tar $(SRC_FILES)
			gzip -f $(PROJECT).tar

main.o: main.c quadr.h
quadr.o: quadr.c quadr.h
#auto generated dependencies
