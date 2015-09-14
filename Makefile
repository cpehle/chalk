OBJDUMP32:=i686-elf-objdump
OBJCOPY32:=i686-elf-objcopy
LD32:=i686-elf-ld
CC:=x86_64-elf-gcc
#CC:=clang -target x86_64-elf
AS:=x86_64-elf-as
#CC32:=clang -target i586-elf
CC32:=i686-elf-gcc
CFLAGS32:=-std=c11 -ffreestanding -O2 -Wall -Wextra -fno-builtin -nostartfiles -nostdlib -nostdinc -fno-common
CFLAGS:=-std=c11 -mno-red-zone -ffreestanding -O2 -Wall -Wextra -fno-builtin -nostartfiles -nostdlib -nostdinc -fno-common -m64 -mcmodel=kernel
AS32:=i686-elf-as
QEMUFLAGS:=-hda chalk.img -serial mon:stdio -smp 2 -m 512 -drive file=chalk.img,if=none,id=mydisk -device ich9-ahci,id=ahci -device ide-drive,drive=mydisk,bus=ahci.0 \
   -drive file=chalknvme.img,if=none,id=D22 \
   -device nvme,drive=D22,serial=1234 \
-vga std\
	-cpu Haswell,+avx,+vmx

QEMU:=qemu-system-x86_64

#OBJS := main.o start64.o mem.o console.o
OBJS32 := load.o start.o mem32.o console32.o pci.o ahci.o e1000.o arena.o vesa.o detect.o vec.o font8x16.o graphics.o acpi.o relptr.o nvme.o delay.o

default: loader.bin chalk.img
	# mkdir -p isodir
	# mkdir -p isodir/boot
	# cp loader.bin isodir/boot/loader.bin
	# cp kernel.elf isodir/boot/kernel.elf
	# mkdir -p isodir/boot/grub
	# cp grub.cfg isodir/boot/grub/grub.cfg
	# grub-mkrescue -o kernel.iso isodir
	$(QEMU) $(QEMUFLAGS)

# chalk.img: bootblock kernel.elf
# 	dd if=/dev/zero of=chalk.img count=10000
# 	dd if=bootblock of=chalk.img conv=notrunc
# 	dd if=kernel.elf of=chalk.img seek=1

chalk.img: bootblock loader.bin
	dd if=/dev/zero of=chalk.img count=10000
	dd if=bootblock of=chalk.img conv=notrunc
	dd if=loader.bin of=chalk.img seek=1

bootblock: boot.s bootmain.c
	$(CC32) $(CFLAGS32) -Os -I. -o bootmain.o -c bootmain.c
	$(CC32) $(CFLAGS32) -Os -I. -o boot.o -c boot.s
	$(LD32) -m elf_i386 -nodefaultlibs -N -e start -Ttext 0x7c00 -o bootblock.o boot.o bootmain.o
	$(OBJDUMP32) -S bootblock.o > bootblock.asm
	$(OBJCOPY32) -S -O binary -j .text bootblock.o bootblock
	./sign.pl bootblock

loader.bin: $(OBJS32) linker.ld
	$(CC32) $(CFLAGS32) -T linker.ld -o loader.bin $(OBJS32) -lgcc

# kernel.elf: $(OBJS) init32.o
# 	$(CC) $(CFLAGS)	-T linker64.ld -o kernel.elf $(OBJS) init32.o -lgcc


init32.o: init32.c
	$(CC32) $(CFLAGS32) -c init32.c -o init32.o
	objcopy -O elf64-x86-64 init32.o

start64.o: start64.s
	$(CC) -c start64.s -o start64.o
start.o: start.s
	$(AS32) -as start.s -o start.o
mem32.o: mem.c
	$(CC32) $(CFLAGS32) -c mem.c -o mem32.o
console32.o: console.c console.h
	$(CC32) $(CFLAGS32) -c console.c -o console32.o
vesa.o: vesa.c vesa.h
	$(CC32) $(CFLAGS32) -c vesa.c -o vesa.o
delay.o: delay.c delay.h u.h io.h
	$(CC32) $(CFLAGS32) -c delay.c -o delay.o
load.o: load.c
	$(CC32) $(CFLAGS32) -c $<
acpi.o: acpi.c acpi.h
	$(CC32) $(CFLAGS32) -c $<
pci.o: pci.c pci.h
	$(CC32) $(CFLAGS32) -c $<
nvme.o: nvme.c nvme.h
	$(CC32) $(CFLAGS32) -c $<
ahci.o: ahci.c ahci.h
	$(CC32) $(CFLAGS32) -c $<
relptr.o: relptr.c relptr.h	u.h
	$(CC32) $(CFLAGS32) -c $<
e1000.o: e1000.c e1000.h
	$(CC32) $(CFLAGS32) -c $<
arena.o: arena.c arena.h
	$(CC32) $(CFLAGS32) -c $<
detect.o: detect.c detect.h u.h dat.h console.h
	$(CC32) $(CFLAGS32) -c detect.c -o detect.o
vec.o: vec.c vec.h
	$(CC32) $(CFLAGS32) -c vec.c -o vec.o
graphics.o: graphics.c graphics.h
	$(CC32) $(CFLAGS32) -c graphics.c -o graphics.o
font8x16.o: font8x16.h font8x16.c
	$(CC32) $(CFLAGS32) -c font8x16.c -o font8x16.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

run:
	$(QEMU) -serial mon:stdio -smp 2 -m 512 -cdrom kernel.iso -drive file=disk.img,if=none,id=mydisk -device ich9-ahci,id=ahci -device ide-drive,drive=mydisk,bus=ahci.0

clean:
	rm *.o
	rm *.bin
	rm *.iso
	rm *.img
