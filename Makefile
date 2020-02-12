#
# Makefile - makefile for fast binary reader example program, fast_reader.c
#
# Currently, this code uses an -ansii C compiler.

#

LIBS = -lfst -L fst -lz

CPP1 = clang++
### Comment out the above line and uncomment the following line instead
### if you are compiling under HPUX
#CC = CC 

CFLAGS = -I. -ggdb -O3 -Ifst -I /usr/local/include
CXXFLAGS = -I. -ggdb -O3 -Ifst -I /usr/local/include

TARGETS =  fastread

OBJECTS = \
	AxisX.o \
	Periodic.o \
	TimeTags.o \
	PagedIntegralData.o \
	IntegralArray.o \
	Reorderer.o \
	Extractor.o \
	BitBlock.o \
	BitBlockData.o \
	StringData.o \
	BitPackedData.o \
	IntegralData.o \
	Utils.o \
	SharedObject.o \
	Label.o \
	DataSet.o \
	DataGroup.o \
	ReadFastOut.o

all: $(TARGETS)

fastread: $(OBJECTS)
	$(CPP1) -o fastread $(OBJECTS) $(LIBS) 

%.o: %.c
	$(CPP1) -c -o $@ $< $(CFLAGS)
	
clean:
	rm -f *.o 

clobber:	clean
	rm -f $(TARGETS)


