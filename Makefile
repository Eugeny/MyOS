CC = gcc

LIBS = -L libs libs/librote.a libs/libfat.a libs/uclibc.a

INCLUDES = -I include -I src/kernel
CFLAGS = -c -g 				\
	-std=c++0x 				\
	-DKERNEL 				\
	-fno-builtin 			\
	-fno-exceptions 		\
	-fno-stack-protector 	\
	-fno-omit-frame-pointer	\
	-fno-rtti 				\
	-ffreestanding 			\
	-mno-red-zone 			\
	-mcmodel=large 			\
	-mno-3dnow				\
	-Wall 					\
	-Wno-write-strings 		\
	-O0 					\
	$(INCLUDES)

LDWRAP = \
	-Xlinker --wrap=malloc \
	-Xlinker --wrap=memcpy \
	-Xlinker --wrap=memset \

LDFLAGS = \
	-static \
	-nostdlib \
	-nodefaultlibs \
	-z max-page-size=0x1000 \
	-T src/kernel/kernel.ld \
	-Xlinker -Map bin/kernel.map \
	$(LDWRAP)


ASFLAGS=-felf64

IMAGE=`readlink -f image.vmdk`


SOURCES= \
	src/kernel/bootstrap.o 						\
	src/kernel/entry.o 							\
	src/kernel/hardware/pmutil.o 				\
												\
	src/kernel/alloc/malloc.o 					\
												\
	src/kernel/core/CPU.o 						\
	src/kernel/core/Debug.o 					\
	src/kernel/core/MQ.o 						\
	src/kernel/core/Process.o 					\
	src/kernel/core/Scheduler.o 				\
	src/kernel/core/Thread.o 					\
	src/kernel/core/Wait.o 						\
												\
	src/kernel/elf/ELF.o 						\
												\
	src/kernel/fs/devfs/DevFS.o 				\
	src/kernel/fs/devfs/PTY.o 					\
	src/kernel/fs/devfs/RandomSource.o 			\
	src/kernel/fs/fat32/libfat-glue.o 			\
	src/kernel/fs/fat32/FAT32FS.o 				\
	src/kernel/fs/procfs/ProcFS.o 				\
	src/kernel/fs/vfs/VFS.o 					\
	src/kernel/fs/Directory.o 					\
	src/kernel/fs/FS.o 							\
	src/kernel/fs/File.o 						\
	src/kernel/fs/Pipe.o 						\
												\
	src/kernel/hardware/io.o 					\
	src/kernel/hardware/pm.o 					\
	src/kernel/hardware/ata/ATA.o 				\
	src/kernel/hardware/cmos/CMOS.o 			\
	src/kernel/hardware/keyboard/Keyboard.o 	\
	src/kernel/hardware/pit/PIT.o 				\
	src/kernel/hardware/vga/VGA.o 				\
												\
	src/kernel/interrupts/IDT.o 				\
	src/kernel/interrupts/IDTUtil.o				\
	src/kernel/interrupts/Interrupts.o 			\
	src/kernel/interrupts/InterruptsUtil.o 		\
												\
	src/kernel/memory/AddressSpace.o 			\
	src/kernel/memory/FrameAlloc.o 				\
	src/kernel/memory/Memory.o 					\
												\
	src/kernel/syscall/Syscalls.o 				\
	src/kernel/syscall/SyscallEntry.o 			\
												\
	src/kernel/tty/Escape.o 					\
	src/kernel/tty/Terminal.o 					\
	src/kernel/tty/PhysicalTerminalManager.o 	\
												\
	src/kernel/kutil.o 							\
												\
	src/kernel/lang/libc-wrap.o 				\
	src/kernel/lang/libc/libc-ext.o 			\
	src/kernel/lang/libc/memcpy.o 				\
	src/kernel/lang/stubs.o 					\


all: $(SOURCES) kernel apps

clean: umount
	@find src -name '*.o' -delete 
	@rm src/apps/init/init || true
	@rm src/apps/test/testapp || true
	@rm bin/kernel || true

kernel: $(SOURCES)
	@echo " LD  " kernel
	@g++ -o bin/kernel $(LDFLAGS) $(SOURCES) $(LIBS)

crt0:
	@gcc -c src/crt0.c -o src/crt0.o

apps: crt0
	@make -C src/apps/init
	@make -C src/apps/test

.s.o:
	@echo " ASM " $<
	@nasm $(ASFLAGS) $<

.cc.o:
	@echo " CC  " $<
	@$(CC) $(CFLAGS) $< -o $@

mount: umount
	@echo "VDI mount"
	@sudo qemu-nbd -c /dev/nbd0 $(IMAGE)
	@sudo chmod 777 /dev/nbd0
	@sudo mount /dev/nbd0p1 fs

umount:
	@echo "VDI umount"
	@sudo umount fs || true
	@sudo qemu-nbd -d /dev/nbd0 || true

deploy: all
	@make mount
	@echo " CP  kernel"
	@sudo cp bin/kernel fs/kernel
	@sudo cp grub.cfg fs/boot/grub/
	@sudo cp src/apps/init/init fs/bin/
	@sudo cp src/apps/test/testapp fs/bin/
	@#sudo cp src/apps/guess/guess fs/bin/
	@#sudo cp src/apps/dash/src/dash fs/bin/
	@make umount

run: deploy
	VirtualBox --startvm VM

bochs: deploy
	bochs -f bochsrc -q



# nano :
# make CFLAGS="-I ../../uclibc-include" LDFLAGS="-nostdlib -static -L ../../libs" LIBS="../../crt0.o ../../src/kernel/lang/stubs.o -Wl,-static -Wl,-Ttext-segment=0x1000000 -lncurses -luclibc"
