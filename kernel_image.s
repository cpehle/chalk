    .global kernel_image_start
    .global kernel_image_end
    .global kernel_image_size
    .section .rawdata
kernel_image_start:
    .incbin "kernel.elf"
kernel_image_end:
kernel_image_size:
    .int kernel_image_end - kernel_image_start
