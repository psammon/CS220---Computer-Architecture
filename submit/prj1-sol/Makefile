TARGETS: hamming-encode hamming-decode

hamming-encode: hamming.o main.o
		gcc hamming.o main.o -o $@

hamming-decode: hamming-encode
		ln -s -f $< $@

main.o: main.c hamming.h
hamming.o: hamming.c hamming.h
