#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "../lib/io/io.h"

extern uint64_t multiboot2_info_addr;
#define MULTIBOOT2_TAG_TYPE_FRAMEBUFFER 8

struct multiboot2_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot2_tag_framebuffer {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t reserved;
};

struct multiboot2_info {
    uint32_t total_size;
    uint32_t reserved;
    uint8_t tags[];
};

// 文本模式颜色
#define TEXT_COLOR_BLACK 0x00
#define TEXT_COLOR_BLUE 0x01
#define TEXT_COLOR_GREEN 0x02
#define TEXT_COLOR_RED 0x04
#define TEXT_COLOR_WHITE 0x07

// 在文本模式下输出字符
void text_mode_putchar(uint16_t* fb, int x, int y, char c, uint8_t color) {
    if (x < 0 || x >= 80 || y < 0 || y >= 25) return;
    fb[y * 80 + x] = (color << 8) | c;
}

// 在文本模式下输出字符串
void text_mode_puts(uint16_t* fb, int x, int y, const char* str, uint8_t color) {
    while (*str) {
        text_mode_putchar(fb, x, y, *str, color);
        x++;
        str++;
        if (x >= 80) {
            x = 0;
            y++;
            if (y >= 25) y = 24;
        }
    }
}

// 清空文本模式屏幕
void text_mode_clear(uint16_t* fb, uint8_t color) {
    for (int i = 0; i < 80 * 25; i++) {
        fb[i] = (color << 8) | ' ';
    }
}

// 图形模式填充测试
void graphics_fill_test(uint64_t fb_addr, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp) {
    serial_printf("Graphics mode: %dx%d, bpp=%d, pitch=%d\n", width, height, bpp, pitch);
        uint8_t* fb = (uint8_t*)fb_addr;
        
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
                fb[offset] = 0;     // R
                fb[offset + 1] = 0; // G
                fb[offset + 2] = 255; // B
            }
        }
}

void kernel_main() 
{
    serial_init();
    serial_printf("\n=== SolumOS Graphics Test ===\n");
    
    // 解析multiboot信息
    struct multiboot2_info* mbi = (struct multiboot2_info*)multiboot2_info_addr;
    uint8_t* current_tag = mbi->tags;
    
    uint64_t fb_addr = 0;
    uint32_t fb_width = 0, fb_height = 0, fb_pitch = 0, fb_bpp = 0;
    int is_graphics_mode = 0;
    
    while (1) {
        struct multiboot2_tag* tag = (struct multiboot2_tag*)current_tag;
        
        if (tag->type == 0) break;
        
        if (tag->type == MULTIBOOT2_TAG_TYPE_FRAMEBUFFER) {
            struct multiboot2_tag_framebuffer* fb_tag = 
                (struct multiboot2_tag_framebuffer*)tag;
            
            fb_addr = fb_tag->framebuffer_addr;
            fb_width = fb_tag->framebuffer_width;
            fb_height = fb_tag->framebuffer_height;
            fb_pitch = fb_tag->framebuffer_pitch;
            fb_bpp = fb_tag->framebuffer_bpp;
            
            serial_printf("Framebuffer: 0x%x, %dx%d, %dbpp\n", 
                         fb_addr, fb_width, fb_height, fb_bpp);
            
            // 判断是图形模式还是文本模式
            if (fb_width > 80 || fb_height > 25 || fb_bpp > 16) {
                is_graphics_mode = 1;
                serial_printf("Graphics mode detected");
            } else {
                is_graphics_mode = 0;
                serial_printf("Text mode detected");
            }
            break;
        }
        
        current_tag += tag->size;
        if ((uint64_t)current_tag % 8 != 0) {
            current_tag += 8 - ((uint64_t)current_tag % 8);
        }
    }
    
    if (fb_addr == 0) {
        serial_printf("No framebuffer found!");
    } else if (is_graphics_mode) {
        // 图形模式测试
        graphics_fill_test(fb_addr, fb_width, fb_height, fb_pitch, fb_bpp);
        serial_printf("Graphics test completed - you should have seen red and green screens");
    } else {
        // 文本模式测试
        uint16_t* text_fb = (uint16_t*)fb_addr;
        text_mode_clear(text_fb, TEXT_COLOR_BLACK);
        text_mode_puts(text_fb, 0, 0, "SolumOS Boot Successful!", TEXT_COLOR_GREEN);
        text_mode_puts(text_fb, 0, 1, "Text Mode: 80x25", TEXT_COLOR_WHITE);
        text_mode_puts(text_fb, 0, 2, "Frame Buffer Test", TEXT_COLOR_RED);
        text_mode_puts(text_fb, 0, 3, "UEFI not available - Using BIOS", TEXT_COLOR_BLUE);
        serial_printf("Text mode test completed");
    }
    
    serial_printf("System ready.");
    
    // 主循环
    while (1) {
        asm volatile ("hlt");
    }
}