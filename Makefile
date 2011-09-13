CC = g++

CFLAGS = -c -g -I src/kernel/ -fno-builtin -fno-stack-protector -fno-rtti -fno-exceptions -Wall -Wno-write-strings -O0
LDFLAGS = -t -L uclibc/lib -static -T src/kernel/linker.ld -Map bin/kernel.map
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
    src/kernel/hardware/ATA.o \
    src/kernel/hardware/ATAUtil.o \
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
    src/kernel/vfs/DevFS.o \
    src/kernel/vfs/RootFS.o \


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
