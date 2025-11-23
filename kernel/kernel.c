#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "../boot/info.h"
#include "../lib/io.h"

// 图形模式填充测试
void graphics_fill_test(uint64_t fb_addr1, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp) {
    serial_printf("Graphics mode: %dx%d, bpp=%d, pitch=%d\n", width, height, bpp, pitch);
        uint8_t* fb = (uint8_t*)fb_addr1;
        
        // 测试1: 尝试RGB格式
        serial_printf("Test 1: BGR format (Red=255, Green=0, Blue=0)\n");
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                uint32_t offset = y * pitch + x * 3;
                fb[offset] = 255;     // R
                fb[offset + 1] = 0; // G
                fb[offset + 2] = 0; // B
            }
        }
        
        // 延迟
        for (volatile int i = 0; i < 1000000000; i++);
        
        // 测试2: 尝试RGB格式
        serial_printf("Test 2: BGR format (Red=0, Green=255, Blue=0)\n");
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                uint32_t offset = y * pitch + x * 3;
                fb[offset] = 0;   // R
                fb[offset + 1] = 255; // G
                fb[offset + 2] = 0; // B
            }
        }
        
        // 延迟
        for (volatile int i = 0; i < 1000000000; i++);
        
        // 测试3: 蓝色屏幕
        serial_printf("Test 3: BGR format (Red=0, Green=0, Blue=255)\n");
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                uint32_t offset = y * pitch + x * 3;
                fb[offset] = 0;   // R
                fb[offset + 1] = 0; // G
                fb[offset + 2] = 255; // B
            }
        }
}

void kernel_main() 
{
    serial_init();
    serial_printf("\n=== SolumOS Graphics Test ===\n");

    parse_mb_info();
    
    if (fb_info->fb_addr == 0) {
        serial_printf("No framebuffer found!\n");
    } else if (is_graphics_mode) {
        // 图形模式测试
        graphics_fill_test(fb_info->fb_addr, fb_info->fb_width, fb_info->fb_height, fb_info->fb_pitch, fb_info->fb_bpp);
        serial_printf("Graphics test completed\n");
    } else {
        // 文本模式测试
        text_mode_clear(fb_info->fb_addr, TEXT_COLOR_BLACK);
        text_mode_puts(fb_info->fb_addr, 0, 0, "SolumOS Boot Successful!", TEXT_COLOR_GREEN);
        text_mode_puts(fb_info->fb_addr, 0, 1, "Text Mode: 80x25", TEXT_COLOR_WHITE);
        text_mode_puts(fb_info->fb_addr, 0, 2, "Frame Buffer Test", TEXT_COLOR_RED);
        text_mode_puts(fb_info->fb_addr, 0, 3, "UEFI not available - Using BIOS", TEXT_COLOR_BLUE);
        serial_printf("Text mode test completed\n");
    }
    
    serial_printf("System ready.\n");
    
    // 主循环
    while (1) {
        asm volatile ("hlt");
    }
}