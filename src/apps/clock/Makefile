BASEDIR = ../../..
SOURCES = main.o ../../kernel/lang/stubs.o
CFLAGS = -c -g -I $(BASEDIR)/uclibc-include/ -std=c99
LDFLAGS = -static -nostdlib -L $(BASEDIR)/libs ../../crt0.o -Ttext-segment=0x1000000 
LIBS = -luclibc

all: $(SOURCES) link

link:
	$(LD) $(LDFLAGS) -o clock $(SOURCES) $(LIBS)

