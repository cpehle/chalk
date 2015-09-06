HARDWAREIRQ( 0);
HARDWAREIRQ( 1);
HARDWAREIRQ( 2);
HARDWAREIRQ( 3);
HARDWAREIRQ( 4);
HARDWAREIRQ( 5);
HARDWAREIRQ( 6);
HARDWAREIRQ( 7);




void interruptctrlinit() {
        // Mask interrupts on PIC1 and PIC2
        {
                outb(0x21, 0xff);
                outb(0xa1, 0xff);
        }

}
