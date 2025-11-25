#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "../boot/info.h"
#include "../lib/serial.h"
#include "../lib/screen.h"

void kernel_main() 
{
    serial_init();
    vga_init();
    serial_printf("\n=== SolumOS Graphics Test ===\n");

    parse_mb_info();
    
    if (fb_info->fb_addr == 0) {
        serial_printf("No framebuffer found!\n");
    } else if (is_graphics_mode) {
        // 图形模式测试
        serial_printf("Graphics mode\n");
    } else {
        vga_printf("hello");
    }
    
    serial_printf("System ready\n");
    
    // 主循环
    while (1) {
        asm volatile ("hlt");
    }
}