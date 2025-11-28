#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <boot/info.h>
#include <lib/serial.h>
#include <lib/screen.h>

void kernel_main() 
{
    serial_init();
    vga_init();
    parse_mb_info();

    vga_printk("Welcome to Solum OS!\n");
    serial_printk("Welcome to Solum OS!\n");
    vga_printk("Version (a0.01)\n");
    serial_printk("Version (a0.01)\n");
    vga_printk("By Roy - 2025\n");
    serial_printk("By Roy - 2025\n");
}