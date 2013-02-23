BASEDIR = /home/eugeny/myos
SOURCES = main.o ../crt0.o
#LIBS=/usr/lib/newlib/i686-linux-gnu/lib
CFLAGS = -c -g -I $(BASEDIR)/src/kernel/ -I $(BASEDIR)/local/i586-pc-myos/include -fno-stack-protector -fno-rtti -fno-exceptions -Wall -Wno-write-strings -O0
#LDFLAGS = -T linker.ld -static -lc
LDFLAGS = -static -lc -e _myos_start
LD=gcc

all: $(SOURCES) link

link:
	$(LD) $(LDFLAGS) -o sh $(SOURCES)

