CC = gcc

LIBS = -L libs libs/librote.a
INCLUDES = -I include -I src/kernel
CFLAGS = -c -std=c++0x -DKERNEL -fno-builtin -fno-stack-protector -mno-red-zone -fno-rtti -fno-exceptions -Wall -Wno-write-strings -O0 $(INCLUDES)

LDWRAP = \
	-Xlinker --wrap=malloc \
#	-Xlinker --wrap=free \

LDFLAGS = -static -static-libstdc++ -z max-page-size=0x1000 -T src/kernel/kernel.ld -Xlinker -Map bin/kernel.map $(LDWRAP)
ASFLAGS=-felf64


SOURCES= \
	src/kernel/bootstrap.o 							\
	src/kernel/entry.o 							\
	src/kernel/alloc/malloc.o 					\
												\
	src/kernel/hardware/io.o 					\
	src/kernel/hardware/cmos/CMOS.o 			\
	src/kernel/hardware/pit/PIT.o 				\
	src/kernel/hardware/vga/vga.o 				\
												\
	src/kernel/interrupts/IDT.o 				\
	src/kernel/interrupts/IDTUtil.o				\
	src/kernel/interrupts/Interrupts.o 			\
	src/kernel/interrupts/InterruptsUtil.o 		\
												\
												\
	src/kernel/tty/Escape.o 					\
	src/kernel/tty/Terminal.o 					\
	src/kernel/tty/PhysicalTerminalManager.o 	\
												\
	src/kernel/kutil.o 							\
												\
	src/kernel/lang/libc-wrap.o 				\


all: $(SOURCES) kernel  apps

clean:
	@find . -name '*.o' -delete 
	@rm bin/kernel || true

kernel: $(SOURCES)
	@echo " LD  " kernel
	@g++ -o bin/kernel $(LDFLAGS) $(SOURCES) $(LIBS)


apps:
	@#gcc -c src/apps/crt0.c -o src/apps/crt0.o
	@#make -C src/apps/shell
	@#make -C src/apps/init
	@#make -C src/apps/guess

.s.o:
	@echo " ASM " $<
	@nasm $(ASFLAGS) $<

.cc.o:
	@echo " CC  " $<
	@$(CC) $(CFLAGS) $< -o $@

mount: umount
	@echo "VMDK mount"
	@vmware-vdiskmanager -R image.vmdk
	@vmware-mount image.vmdk fs

umount:
	@echo "VMDK umount"
	@sudo umount fs || true
	@vmware-mount -d fs || true
	@sleep 1
	@vmware-mount -d fs || true

deploy: all
	@make mount
	@echo " CP  kernel"
	@sudo cp bin/kernel fs/kernel
	@sudo cp grub.cfg fs/boot/grub/
	@#sudo cp src/apps/init/init fs/sbin/
	@#sudo cp src/apps/shell/sh fs/bin/
	@#sudo cp src/apps/guess/guess fs/bin/
	@#sudo cp src/apps/dash/src/dash fs/bin/
	@make umount

run: deploy
	@VirtualBox --startvm VM