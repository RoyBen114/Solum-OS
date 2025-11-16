#include <stdint.h>

struct boot_info {
    uint32_t boot_type;
    void *memory_map_ptr;
    uint32_t memory_map_entries;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_bpp;
};

// 端口输入函数
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// 端口输出函数
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// 串口初始化
void serial_init() {
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x0C);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

void serial_puts(const char* str) {
    while (*str) {
        while ((inb(0x3F8 + 5) & 0x20) == 0);
        outb(0x3F8, *str++);
    }
}

void serial_put_hex(uint64_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[17];
    buffer[16] = '\0';
    
    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    serial_puts("0x");
    serial_puts(buffer);
}

void serial_put_dec(uint32_t value) {
    char buffer[11];
    char* ptr = buffer + 10;
    *ptr = '\0';
    
    if (value == 0) {
        serial_puts("0");
        return;
    }
    
    while (value > 0) {
        *--ptr = '0' + (value % 10);
        value /= 10;
    }
    
    serial_puts(ptr);
}

// 简单的VGA文本输出 (BIOS模式使用)
void vga_puts(const char *str, uint8_t color) {
    volatile uint16_t *vga_buffer = (volatile uint16_t*)0xB8000;
    int i;
    for (i = 0; str[i] != 0; i++) {
        vga_buffer[i] = (color << 8) | str[i];
    }
}

// 清除VGA屏幕
void vga_clear(uint8_t color) {
    volatile uint16_t *vga_buffer = (volatile uint16_t*)0xB8000;
    for (int i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = (color << 8) | ' ';
    }
}

// UEFI帧缓冲区支持
void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color, struct boot_info *info) {
    if (info->framebuffer_addr == 0) return;
    
    uint8_t *fb = (uint8_t*)(uintptr_t)info->framebuffer_addr;
    uint32_t *pixel = (uint32_t*)(fb + y * info->framebuffer_pitch + x * 4);
    *pixel = color;
}

// 简单的矩形绘制
void fb_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color, struct boot_info *info) {
    if (info->framebuffer_addr == 0) return;
    
    for (uint32_t row = y; row < y + height; row++) {
        for (uint32_t col = x; col < x + width; col++) {
            fb_put_pixel(col, row, color, info);
        }
    }
}

void kernel_main(struct boot_info *info) {
    // 初始化串口
    serial_init();
    serial_puts("\n=== UEFI Display Debug ===\n");
    
    // 输出详细的引导信息
    serial_puts("Boot type: ");
    if (info->boot_type == 0) {
        serial_puts("BIOS\n");
    } else {
        serial_puts("UEFI\n");
    }
    
    serial_puts("Framebuffer address: ");
    serial_put_hex(info->framebuffer_addr);
    serial_puts("\n");
    
    serial_puts("Framebuffer width: ");
    serial_put_dec(info->framebuffer_width);
    serial_puts("\n");
    
    serial_puts("Framebuffer height: ");
    serial_put_dec(info->framebuffer_height);
    serial_puts("\n");
    
    serial_puts("Framebuffer pitch: ");
    serial_put_dec(info->framebuffer_pitch);
    serial_puts("\n");
    
    serial_puts("Framebuffer bpp: ");
    serial_put_dec(info->framebuffer_bpp);
    serial_puts("\n");
    
    // BIOS模式：使用VGA文本模式
    if (info->boot_type == 0) {
        serial_puts("Using VGA text mode (BIOS)\n");
        vga_clear(0x00); // 黑色背景
        vga_puts("SolumOS - BIOS Mode", 0x0F); // 白色文本
    }
    // UEFI模式：尝试使用帧缓冲区
    else if (info->framebuffer_addr != 0) {
        serial_puts("Using framebuffer (UEFI)\n");
        
        // 清屏为蓝色
        for (uint32_t y = 0; y < info->framebuffer_height; y++) {
            for (uint32_t x = 0; x < info->framebuffer_width; x++) {
                fb_put_pixel(x, y, 0x000000FF, info); // 蓝色背景
            }
        }
        
        // 绘制一些简单的图形来测试
        fb_draw_rect(100, 100, 200, 150, 0x00FF0000, info); // 红色矩形
        fb_draw_rect(150, 150, 200, 150, 0x0000FF00, info); // 绿色矩形
        fb_draw_rect(200, 200, 200, 150, 0x00FFFFFF, info); // 白色矩形
        
        serial_puts("Framebuffer graphics drawn\n");
    }
    // UEFI模式但没有帧缓冲区
    else {
        serial_puts("No framebuffer available (UEFI)\n");
        // 尝试VGA文本模式（虽然通常不工作）
        vga_clear(0x00);
        vga_puts("UEFI - No Framebuffer", 0x0F);
    }
    
    serial_puts("Kernel initialization complete\n");
    
    // 主循环
    int counter = 0;
    while (1) {
        // 定期输出心跳信号
        for (volatile int i = 0; i < 10000000; i++);
        
        if (counter % 10 == 0) {
            serial_puts("Heartbeat: ");
            serial_put_dec(counter);
            serial_puts("\n");
        }
        
        counter++;
        
        // 安全暂停
        asm volatile ("hlt");
    }
}