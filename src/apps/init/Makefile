BASEDIR = ../../..
SOURCES = init.o 
CFLAGS = -c -g -I $(BASEDIR)/uclibc-include/
LDFLAGS = -static -nostdlib -L $(BASEDIR)/libs ../../crt0.o 
LIBS = -luclibc
# 15 10

all: $(SOURCES) link

link:
	$(LD) $(LDFLAGS) -o init $(SOURCES) $(LIBS)

