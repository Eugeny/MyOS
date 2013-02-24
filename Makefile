CC = gcc

LIBS = -L libs libs/librote.a
INCLUDES = -I include -I src/kernel
CFLAGS = -c -m32 -g -std=c++0x -DKERNEL -fno-builtin -fno-stack-protector -fno-rtti -fno-exceptions -Wall -Wno-write-strings -O0 $(INCLUDES)

LDWRAP = \
	-Xlinker --wrap=malloc \
#	-Xlinker --wrap=free \

LDFLAGS = -static -m32 -static-libstdc++  -T src/kernel/linker.ld -Xlinker -Map bin/kernel.map $(LDWRAP)
ASFLAGS=-felf32

SOURCES= \
	src/kernel/bootstrap.o 						\
												\
	src/kernel/entry.o 							\
	src/kernel/alloc/malloc.o 					\
												\
	src/kernel/tty/Escape.o 					\
	src/kernel/tty/Terminal.o 					\
	src/kernel/tty/PhysicalTerminalManager.o 	\
												\
	src/kernel/kutil.o 							\
												\
	src/kernel/lang/libc-wrap.o 				\


all: $(SOURCES) link apps

clean:
	@find . -name '*.o' -delete 
	@rm bin/kernel || true

link: $(SOURCES)
	@g++ -o bin/kernel $(LDFLAGS) $(SOURCES) $(LIBS)
	@echo " LD " kernel

apps:
	@#gcc -c src/apps/crt0.c -o src/apps/crt0.o
	@#make -C src/apps/shell
	@#make -C src/apps/init
	@#make -C src/apps/guess

.s.o:
	@nasm $(ASFLAGS) $<
	@echo " ASM" $<

.cc.o:
	@$(CC) $(CFLAGS) $< -o $@
	@echo " CC " $<

mount: umount
	@echo "VMDK mount"
	@vmware-vdiskmanager -R image.vmdk
	@vmware-mount image.vmdk fs

umount:
	@echo "VMDK umount"
	@sleep 1
	@vmware-mount -d fs || true
	@sleep 1
	@vmware-mount -d fs || true

deploy: all
	@make mount
	@sudo cp bin/kernel fs/kernel
	@echo " CP  kernel"
	@#sudo cp src/apps/init/init fs/sbin/
	@#sudo cp src/apps/shell/sh fs/bin/
	@#sudo cp src/apps/guess/guess fs/bin/
	@#sudo cp src/apps/dash/src/dash fs/bin/
	@make umount

run: deploy
	@VirtualBox --startvm VM