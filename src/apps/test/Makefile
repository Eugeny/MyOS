BASEDIR = /home/eugeny/myos
CC = $(BASEDIR)/local/bin/i586-pc-myos-gcc
LD = $(BASEDIR)/local/bin/i586-pc-myos-ld
SOURCES = test.o ../crt0.o
LIBS = $(BASEDIR)/local/i586-pc-myos/lib
CFLAGS = -c -g -I $(BASEDIR)/src/kernel/ -I $(BASEDIR)/include -fno-builtin -fno-stack-protector -fno-rtti -fno-exceptions -Wall -Wno-write-strings -O0
LDFLAGS = -T linker.ld -L $(LIBS) -lc -static

all: $(SOURCES) link

link:
	$(LD) $(LDFLAGS) -o test $(SOURCES) $(LIBS)/libc.a
