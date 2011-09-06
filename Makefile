CC = uclibc/bin/i586-gcc
LD = uclibc/bin/i586-ld
LIBS = uclibc/lib

CFLAGS = -I uclibc/include -fno-builtin -Wall
LDFLAGS = -t -L uclibc/lib -static -lc $(LIBS)/libc.a

KCFLAGS = 
KLDFLAGS = -T src/kernel/linker.ld 


all: main

main: kernel

init:
	mkdir -p build/kernel 2> /dev/null    
    
kernel: init
	echo :: Building kernel > /dev/null
	nasm -felf -o build/kernel/bootstrap.o src/kernel/bootstrap.asm
	$(CC) -c -o build/kernel/entry.o $(CFLAGS) $(KCFLAGS) src/kernel/entry.c 
	$(LD) $(KLDFLAGS) $(LDFLAGS) build/kernel/*.o -o bin/kernel
	
mount: umount
	vmware-mount ~/vmware/MyOS/MyOS-0.vmdk fs
	
umount:		
	vmware-mount -d fs || true
	sleep 1
	vmware-mount -d fs
		
deploy: main
	echo :: Deploying > /dev/null
	vmware-mount ~/vmware/MyOS/MyOS-0.vmdk fs
	sudo cp bin/kernel fs/kernel
	vmware-mount -d fs || true
	sleep 1
	vmware-mount -d fs
	
.PHONY: init mount umount main kernel deploy 
