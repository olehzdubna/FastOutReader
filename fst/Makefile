#
# Makefile - makefile for fast binary reader example program, fast_reader.c
#
# Currently, this code uses an -ansii C compiler.

#

LIBS =

CC = cc
### Comment out the above line and uncomment the following line instead
### if you are compiling under HPUX
#CC = CC 

CFLAGS =  -O3 -I. -I/usr/local/include

TARGETS =  fst

OBJECTS = \
		fstapi.o \
		fastlz.o \
		lz4.o 

all: $(TARGETS)

fst: $(OBJECTS)
	ar -r libfst.a $(OBJECTS)
#	$(CC) -c $(OBJECTS) 

fstapi.o: fstapi.c
fastlz.o: fastlz.c
lz4.o : lz4.c

clean:
	rm -f *.o 

clobber:	clean
	rm -f $(TARGETS)


