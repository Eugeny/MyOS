BASEDIR = /home/eugeny/myos
#CC = $(BASEDIR)/local/bin/i586-pc-myos-gcc
#LD = $(BASEDIR)/local/bin/i586-pc-myos-ld
LD=gcc
SOURCES = init.o ../crt0.o
#LIBS = $(BASEDIR)/local/i586-pc-myos/lib
#LIBS=/usr/lib/newlib/i686-linux-gnu/lib
CFLAGS = -c -g -I $(BASEDIR)/src/kernel/ -fno-stack-protector -fno-rtti -fno-exceptions -Wall -Wno-write-strings -O0
LDFLAGS = -T linker.ld -static 
LDFLAGS = -static -lc -e _myos_start

all: $(SOURCES) link

link:
	$(LD) $(LDFLAGS) -o init $(SOURCES) # $(LIBS)/libc.a  $(LIBS)/libm.a

