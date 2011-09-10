CC = g++

CFLAGS = -c -g -I src/kernel/ -fno-builtin -fno-stack-protector -fno-rtti -fno-exceptions -Wno-write-strings -O0
LDFLAGS = -t -L uclibc/lib -static -T src/kernel/linker.ld -Map bin/kernel.map
ASFLAGS=-felf

SOURCES= \
    src/kernel/bootstrap.o \
    src/kernel/entry.o \
    src/kernel/gdt.o \
    src/kernel/idt.o \
    src/kernel/interrupts.o \
    src/kernel/isr.o \
    src/kernel/kutils.o \
    src/kernel/paging.o \
    src/kernel/tasking.o \
    src/kernel/terminal.o\
    src/kernel/timer.o \
    src/kernel/memory/Heap.o \
    src/kernel/util.o \
    src/kernel/util/cpp.o \
    src/kernel/util/lock.o \

all: $(SOURCES) link

clean:
	find -name '*.o' -delete
	rm bin/kernel || true

link:
	ld $(LDFLAGS) -o bin/kernel $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<

.cc.o:
	$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
	

mount: umount
	vmware-mount ~/vmware/MyOS/MyOS-0.vmdk fs
	
umount:		
	vmware-mount -d fs || true
	sleep 2
	vmware-mount -d fs || true
		
deploy: all
	echo
	vmware-mount ~/vmware/MyOS/MyOS-0.vmdk fs
	sudo cp bin/kernel fs/kernel
	vmware-mount -d fs || true
	sleep 2
	vmware-mount -d fs || true
