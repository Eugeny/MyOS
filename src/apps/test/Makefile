BASEDIR = ../../..
SOURCES = main.o 
CFLAGS = -c -g -I $(BASEDIR)/uclibc-include/
LDFLAGS = -static -nostdlib -L $(BASEDIR)/libs ../../crt0.o -Ttext-segment=0x1000000 
LIBS = -luclibc


all: $(SOURCES) link

link:
	$(LD) $(LDFLAGS) -o testapp $(SOURCES) $(LIBS)

