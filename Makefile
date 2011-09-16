CC = gcc

LIBS = local/i586-pc-myos/lib

CFLAGS = -c -g -I src/kernel/ -I include -fno-builtin -fno-stack-protector -fno-rtti -fno-exceptions -Wall -Wno-write-strings -O0
LDFLAGS = -t -static -nostdlib -T src/kernel/linker.ld -Map bin/kernel.map
#LDFLAGS = -t -L newlib/newlib/libc -lc -static -nostdlib -T src/kernel/linker.ld -Map bin/kernel.map
ASFLAGS=-felf

SOURCES= \
    src/kernel/bootstrap.o \
    src/kernel/core/ProcessorUtil.o \
    src/kernel/interrupts/IDTUtil.o \
    src/kernel/interrupts/InterruptsUtil.o \
    src/kernel/memory/AddressSpaceUtil.o \
    src/kernel/memory/GDTUtil.o \
\
    src/kernel/entry.o \
    src/kernel/kutils.o \
    src/kernel/core/Processor.o \
    src/kernel/core/Scheduler.o \
    src/kernel/core/TaskManager.o \
    src/kernel/core/TaskManagerUtil.o \
    src/kernel/core/Process.o \
    src/kernel/core/Thread.o \
    src/kernel/elf/ELF.o \
    src/kernel/hardware/ATA.o \
    src/kernel/hardware/ATAUtil.o \
    src/kernel/hardware/Disk.o \
    src/kernel/hardware/Keyboard.o \
    src/kernel/hardware/PIT.o \
    src/kernel/interrupts/IDT.o \
    src/kernel/interrupts/Interrupts.o \
    src/kernel/io/FileObject.o \
    src/kernel/memory/AddressSpace.o \
    src/kernel/memory/Heap.o \
    src/kernel/memory/GDT.o \
    src/kernel/memory/Memory.o \
    src/kernel/syscall/SyscallManager.o\
    src/kernel/syscall/Syscalls.o\
    src/kernel/tty/Terminal.o\
    src/kernel/tty/TTY.o\
    src/kernel/tty/TTYManager.o\
    src/kernel/util/cpp.o \
    src/kernel/util/Lock.o \
    src/kernel/vfs/VFS.o \
    src/kernel/vfs/FS.o \
    src/kernel/vfs/FATFS.o \
    src/kernel/vfs/DevFS.o \
    src/kernel/vfs/RootFS.o \


all: $(SOURCES) link app

clean:
	find -name '*.o' -delete
	rm bin/kernel || true

link:
	ld $(LDFLAGS) -o bin/kernel $(SOURCES)

app:
	local/bin/i586-pc-myos-gcc -c -g -mtune=i386 -I src/kernel/ include -fno-builtin -fno-stack-protector -fno-rtti -fno-exceptions -Wall -Wno-write-strings -O0 src/apps/test/test.c -o src/apps/test/test.o
	local/bin/i586-pc-myos-ld -t -static -T src/apps/test/linker.ld  -L $(LIBS) -lc -o bin/app src/apps/test/test.o local/i586-pc-myos/lib/libc.a src/kernel/syscall/Syscalls.o

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
	sudo cp bin/app fs/app
	vmware-mount -d fs || true
	sleep 2
	vmware-mount -d fs || true
