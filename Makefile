
CC:=x86_64-elf-gcc
AS:=x86_64-elf-gas
CC32:=i686-elf-gcc
CFLAGS32:=-std=c11 -ffreestanding -O2 -Wall -Wextra
CFLAGS:=-std=c11 -mno-red-zone -ffreestanding -O2 -Wall -Wextra
AS32:=i686-elf-as
QEMUFLAGS:=-serial mon:stdio -smp 2 -m 512 -drive file=disk.img,if=none,id=mydisk -device ich9-ahci,id=ahci -device ide-drive,drive=mydisk,bus=ahci.0
QEMU:=qemu-system-x86_64

OBJS := main.o start64.o
OBJS32 := load.o start.o mem32.o kernel_image.o console.o pci.o

default: kernel.bin
	mkdir -p isodir
	mkdir -p isodir/boot
	cp kernel.bin isodir/boot/kernel.bin
	mkdir -p isodir/boot/grub
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o kernel.iso isodir
	$(QEMU) $(QEMUFLAGS) -cdrom kernel.iso

kernel.bin: $(OBJS32) linker.ld kernel_image.o linker.ld
	$(CC32) -T linker.ld -o kernel.bin -ffreestanding -O2 -nostdlib $(OBJS32) -lgcc
kernel_image.o: $(OBJS) kernel_image.s linker64.ld
	$(CC) -T linker64.ld -o kernel.elf -ffreestanding -O2 -nostdlib $(OBJS) -lgcc
	$(CC32) kernel_image.s -c -o kernel_image.o
start64.o: start64.s
	$(CC) -c start64.s -o start64.o
start.o: start.s
	$(AS32) -as start.s -o start.o
mem32.o: mem.c
	$(CC32) $(CFLAGS32) -c mem.c -o mem32.o
console.o: console.c console.h
	$(CC32) $(CFLAGS32) -c console.c -o console.o
load.o: load.c
	$(CC32) $(CFLAGS32) -c $<
pci.o: pci.c
	$(CC32) $(CFLAGS32) -c $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

run:
	$(QEMU) -serial mon:stdio -smp 2 -m 512 -cdrom kernel.iso -drive file=disk.img,if=none,id=mydisk -device ich9-ahci,id=ahci -device ide-drive,drive=mydisk,bus=ahci.0

clean:
	rm *.o
	rm *.bin
	rm *.iso
