CC = gcc

LIBS = local/i586-pc-myos/lib

CFLAGS = -c -g -DKERNEL -I src/kernel/ -fno-builtin -fno-stack-protector -fno-rtti -fno-exceptions -Wall -Wno-write-strings -O0
LDFLAGS = -static -nostdlib -T src/kernel/linker.ld -Map bin/kernel.map
#LDFLAGS = -t -L newlib/newlib/libc -lc -static -nostdlib -T src/kernel/linker.ld -Map bin/kernel.map
ASFLAGS=-felf

BASEDIR = /home/eugeny/myos
LIBS = $(BASEDIR)/local/i586-pc-myos/lib


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


all: $(SOURCES) link apps

clean:
	find src/kernel -name '*.o' -delete 
	find src/apps -name '*.o' -delete 
	rm bin/kernel || true

link:
	@ld -o bin/kernel $(LDFLAGS) $(SOURCES)
	@echo "LD  " kernel

apps:
	gcc -c src/apps/crt0.c -o src/apps/crt0.o
	make -C src/apps/shell
	make -C src/apps/init

.s.o:
	@nasm $(ASFLAGS) $<
	@echo "ASM" $<

.cc.o:
	@$(CC) $(CFLAGS) $< -o $@
	@echo "CC " $<

.cpp.o:
	@$(CC) $(CFLAGS) $< -o $@
	@echo "CC " $<



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
	sudo cp src/apps/init/init fs/sbin/init
	sudo cp src/apps/shell/sh fs/bin/sh
	vmware-mount -d fs || true
	sleep 2
	vmware-mount -d fs || true
