arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/chalk-$(arch).iso

linker_script := chalk/$(arch)/linker.ld
grub_cfg := chalk/$(arch)/grub.cfg

assembly_source_files := $(wildcard chalk/$(arch)/*.asm)
assembly_object_files := $(patsubst chalk/$(arch)/%.asm, \
build/arch/$(arch)/%.o, $(assembly_source_files))

libchalk_source_files := $(wildcard chalk/lib/*.c)
libchalk_object_files := $(patsubst chalk/lib/%.c, \
	build/lib/%.o, $(libchalk_source_files))

cflags := -ffreestanding -O2 -Wall -Wextra -nostdlib --target=x86_64-pc-none-elf

.PHONY: all clean run iso

all: $(kernel)

clean:
	@rm -r build

run: $(iso)
	@qemu-system-x86_64 -serial mon:stdio -smp 2 -cdrom $(iso)

iso: $(iso)

$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	cp $(kernel) build/isofiles/boot/kernel.bin
	cp $(grub_cfg) build/isofiles/boot/grub
	grub-mkrescue -o $(iso) build/isofiles 2> /dev/null


$(kernel): $(assembly_object_files) $(linker_script) build/lib/libchalk.a
	@x86_64-elf-ld -n -T $(linker_script) -o $(kernel) $(libchalk_object_files) $(assembly_object_files)

# compile assembly files
build/arch/$(arch)/%.o: chalk/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -felf64 $< -o $@

build/lib/%.o: chalk/lib/%.c
	@mkdir -p $(shell dirname $@)
	@clang -Ichalk/lib/ $(cflags) -c $< -o $@

build/lib/libchalk.a: $(libchalk_object_files)
	@mkdir -p $(shell dirname $@)
	@ar	rcs $@ $(libchalk_object_files)
