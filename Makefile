
CC:=x86_64-elf-gcc
#CC:=clang -target x86_64-elf
AS:=x86_64-elf-gas
#CC32:=clang -target i586-elf
CC32:=i686-elf-gcc
CFLAGS32:=-std=c11 -ffreestanding -O2 -Wall -Wextra -fno-builtin -nostdlib -nostdinc -fno-common
CFLAGS:=-std=c11 -mno-red-zone -ffreestanding -O2 -Wall -Wextra -fno-builtin -nostdlib -nostdinc -fno-common
AS32:=i686-elf-as
QEMUFLAGS:=-enable-kvm -serial mon:stdio -smp 2 -m 512 -drive file=disk.img,if=none,id=mydisk -device ich9-ahci,id=ahci -device ide-drive,drive=mydisk,bus=ahci.0 -vga std\
-cpu Haswell,+avx
QEMU:=qemu-system-x86_64

OBJS := main.o start64.o
OBJS32 := load.o start.o mem32.o kernel_image.o console.o pci.o ahci.o e1000.o arena.o vesa.o detect.o vec.o

default: kernel.bin
	mkdir -p isodir
	mkdir -p isodir/boot
	cp kernel.bin isodir/boot/kernel.bin
	mkdir -p isodir/boot/grub
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o kernel.iso isodir
	$(QEMU) $(QEMUFLAGS) -cdrom kernel.iso

kernel.bin: $(OBJS32) linker.ld kernel_image.o
	$(CC32) $(CFLAGS32) -T linker.ld -o kernel.bin $(OBJS32) -lgcc
kernel_image.o: $(OBJS) kernel_image.s linker64.ld
	$(CC) $(CFLAGS) -T linker64.ld -o kernel.elf $(OBJS) -lgcc
	$(CC32) $(CFLAGS32) kernel_image.s -c -o kernel_image.o
start64.o: start64.s
	$(CC) -c start64.s -o start64.o
start.o: start.s
	$(AS32) -as start.s -o start.o
mem32.o: mem.c
	$(CC32) $(CFLAGS32) -c mem.c -o mem32.o
console.o: console.c console.h
	$(CC32) $(CFLAGS32) -c console.c -o console.o
vesa.o: vesa.c vesa.h
	$(CC32) $(CFLAGS32) -c vesa.c -o vesa.o
load.o: load.c
	$(CC32) $(CFLAGS32) -c $<
pci.o: pci.c pci.h
	$(CC32) $(CFLAGS32) -c $<
ahci.o: ahci.c ahci.h
	$(CC32) $(CFLAGS32) -c $<
e1000.o: e1000.c e1000.h
	$(CC32) $(CFLAGS32) -c $<
interrupts.o: interrupts.s
	$(CC32) $(CFLAGS32) interrupts.s -c -o interrupts.o
arena.o: arena.c arena.h
	$(CC32) $(CFLAGS32) -c $<
detect.o: detect.c detect.h u.h dat.h console.h
	$(CC32) $(CFLAGS32) -c detect.c -o detect.o
vec.o: vec.c vec.h
	$(CC32) $(CFLAGS32) -c vec.c -o vec.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

run:
	$(QEMU) -serial mon:stdio -smp 2 -m 512 -cdrom kernel.iso -drive file=disk.img,if=none,id=mydisk -device ich9-ahci,id=ahci -device ide-drive,drive=mydisk,bus=ahci.0

clean:
	rm *.o
	rm *.bin
	rm *.iso
