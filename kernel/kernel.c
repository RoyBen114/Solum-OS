#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

extern uint64_t multiboot2_info_addr;

//===================================================↓serial functions↓==================================================
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static int serial_is_transmit_empty() {
    return inb(0x3F8 + 5) & 0x20;
}

void serial_init() {
    outb(0x3F8 + 1, 0x00); 
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00); 
    outb(0x3F8 + 3, 0x03); 
    outb(0x3F8 + 2, 0xC7); 
    outb(0x3F8 + 4, 0x0B); 
}

void serial_putc(char c) {
    while (!serial_is_transmit_empty());
    outb(0x3F8, c);
    
    if (c == '\n') {
        while (!serial_is_transmit_empty());
        outb(0x3F8, '\r');
    }
}

void serial_puts(const char *str) {
    while (*str) {
        serial_putc(*str++);
    }
}

void serial_put_dec(uint64_t value) {
    char buffer[21];
    char *ptr = buffer + 20;
    *ptr = '\0';
    
    if (value == 0) {
        serial_putc('0');
        return;
    }
    
    while (value > 0) {
        *--ptr = '0' + (value % 10);
        value /= 10;
    }
    
    serial_puts(ptr);
}

void serial_put_hex(uint64_t value) {
    const char hex_chars[] = "0123456789ABCDEF";
    char buffer[17];
    buffer[16] = '\0';
    
    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    serial_puts("0x");
    serial_puts(buffer);
}

void serial_put_hex_lower(uint64_t value) {
    const char hex_chars[] = "0123456789abcdef";
    char buffer[17];
    buffer[16] = '\0';
    
    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    serial_puts("0x");
    serial_puts(buffer);
}

void serial_put_ptr(void *ptr) {
    serial_put_hex((uint64_t)(uintptr_t)ptr);
}

void serial_put_dec_signed(int64_t value) {
    if (value < 0) {
        serial_putc('-');
        value = -value;
    }
    serial_put_dec((uint64_t)value);
}

void serial_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    while (*format) {
        if (*format == '%') {
            format++;
            
            int long_count = 0;
            while (*format == 'l') {
                long_count++;
                format++;
            }
            
            switch (*format) {
                case 'c': {
                    char c = (char)va_arg(args, int);
                    serial_putc(c);
                    break;
                }
                
                case 's': {
                    char *str = va_arg(args, char*);
                    if (str == NULL) {
                        serial_puts("(null)");
                    } else {
                        serial_puts(str);
                    }
                    break;
                }
                
                case 'd': 
                case 'i': {
                    if (long_count >= 2) {
                        int64_t num = va_arg(args, int64_t);
                        serial_put_dec_signed(num);
                    } else if (long_count == 1) {
                        long num = va_arg(args, long);
                        serial_put_dec_signed(num);
                    } else {
                        int num = va_arg(args, int);
                        serial_put_dec_signed(num);
                    }
                    break;
                }
                
                case 'u': {
                    if (long_count >= 2) {
                        uint64_t num = va_arg(args, uint64_t);
                        serial_put_dec(num);
                    } else if (long_count == 1) {
                        unsigned long num = va_arg(args, unsigned long);
                        serial_put_dec(num);
                    } else {
                        unsigned int num = va_arg(args, unsigned int);
                        serial_put_dec(num);
                    }
                    break;
                }
                
                case 'x': {
                    if (long_count >= 2) {
                        uint64_t num = va_arg(args, uint64_t);
                        serial_put_hex_lower(num);
                    } else if (long_count == 1) {
                        unsigned long num = va_arg(args, unsigned long);
                        serial_put_hex_lower(num);
                    } else {
                        unsigned int num = va_arg(args, unsigned int);
                        serial_put_hex_lower((uint64_t)num);
                    }
                    break;
                }
                
                case 'X': {
                    if (long_count >= 2) {
                        uint64_t num = va_arg(args, uint64_t);
                        serial_put_hex(num);
                    } else if (long_count == 1) {
                        unsigned long num = va_arg(args, unsigned long);
                        serial_put_hex(num);
                    } else {
                        unsigned int num = va_arg(args, unsigned int);
                        serial_put_hex((uint64_t)num);
                    }
                    break;
                }
                
                case 'p': {
                    void *ptr = va_arg(args, void*);
                    serial_put_ptr(ptr);
                    break;
                }
                
                case '%': {
                    serial_putc('%');
                    break;
                }
                
                default: {
                    serial_putc('%');
                    serial_putc(*format);
                    break;
                }
            }
        } else {
            serial_putc(*format);
        }
        format++;
    }
    
    va_end(args);
}
//===================================================↑serial functions↑==================================================

#define COLOR_BLACK     0xFF000000
#define COLOR_WHITE     0xFFFFFFFF
#define COLOR_RED       0xFFFF0000
#define COLOR_GREEN     0xFF00FF00
#define COLOR_BLUE      0xFF0000FF
#define MULTIBOOT2_TAG_TYPE_FRAMEBUFFER 8

// Framebuffer信息结构
struct framebuffer_info {
    uint64_t address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
};

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

// 全局framebuffer信息
static struct framebuffer_info fb_info = {0};
static uint32_t* backbuffer = NULL;

// 初始化framebuffer
struct framebuffer_info* framebuffer_init(uint64_t multiboot_info_addr) {
    // 直接使用物理地址访问multiboot信息（恒等映射）
    struct multiboot2_info* mbi = (struct multiboot2_info*)multiboot_info_addr;
    uint8_t* current_tag = mbi->tags;
    
    serial_printf("Searching for framebuffer tag...\n");
    serial_printf("Multiboot info total size: %u\n", mbi->total_size);
    
    // 查找framebuffer tag
    while (1) {
        struct multiboot2_tag* tag = (struct multiboot2_tag*)current_tag;
        
        if (tag->type == 0) {
            serial_printf("End tag found\n");
            break; // END tag
        }
        
        serial_printf("Tag type: %u, size: %u\n", tag->type, tag->size);
        
        if (tag->type == MULTIBOOT2_TAG_TYPE_FRAMEBUFFER) {
            struct multiboot2_tag_framebuffer* fb_tag = 
                (struct multiboot2_tag_framebuffer*)tag;
            
            fb_info.address = fb_tag->framebuffer_addr;
            fb_info.width = fb_tag->framebuffer_width;
            fb_info.height = fb_tag->framebuffer_height;
            fb_info.pitch = fb_tag->framebuffer_pitch;
            fb_info.bpp = fb_tag->framebuffer_bpp;
            
            serial_printf("Framebuffer found:\n");
            serial_printf("  Address: 0x%llx\n", fb_info.address);
            serial_printf("  Width: %u\n", fb_info.width);
            serial_printf("  Height: %u\n", fb_info.height);
            serial_printf("  Pitch: %u\n", fb_info.pitch);
            serial_printf("  BPP: %u\n", fb_info.bpp);
            
            // 使用汇编中分配的后缓冲区
            extern uint8_t framebuffer_backbuffer[];
            backbuffer = (uint32_t*)framebuffer_backbuffer;
            
            // 验证后缓冲区地址
            serial_printf("Backbuffer at: 0x%llx\n", (uint64_t)backbuffer);
            
            // 计算屏幕大小
            uint32_t screen_size = fb_info.width * fb_info.height;
            serial_printf("Screen size: %u pixels\n", screen_size);
            
            // 清空后缓冲区为黑色
            for (uint32_t i = 0; i < screen_size; i++) {
                backbuffer[i] = COLOR_BLACK;
            }
            
            serial_printf("Framebuffer initialized successfully\n");
            return &fb_info;
        }
        
        // 移动到下一个tag（8字节对齐）
        current_tag += tag->size;
        if ((uint64_t)current_tag % 8 != 0) {
            current_tag += 8 - ((uint64_t)current_tag % 8);
        }
    }
    
    serial_printf("No framebuffer tag found!\n");
    return NULL;
}

// 绘制矩形
void framebuffer_draw_rect(int x, int y, int width, int height, uint32_t color) {
    if (!backbuffer) {
        serial_printf("Error: backbuffer is NULL\n");
        return;
    }
    
    // 边界检查
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= (int)fb_info.width) {
        serial_printf("Warning: x coordinate out of bounds\n");
        return;
    }
    if (y >= (int)fb_info.height) {
        serial_printf("Warning: y coordinate out of bounds\n");
        return;
    }
    
    int end_x = x + width;
    int end_y = y + height;
    if (end_x > (int)fb_info.width) end_x = fb_info.width;
    if (end_y > (int)fb_info.height) end_y = fb_info.height;
    
    serial_printf("Drawing rect: (%d,%d) to (%d,%d), color=0x%x\n", 
                 x, y, end_x, end_y, color);
    
    for (int row = y; row < end_y; row++) {
        for (int col = x; col < end_x; col++) {
            uint32_t offset = row * fb_info.width + col;
            if (offset < fb_info.width * fb_info.height) {
                backbuffer[offset] = color;
            }
        }
    }
}

// 交换缓冲区
void framebuffer_swap_buffers(void) {
    if (!backbuffer) {
        serial_printf("Error: backbuffer is NULL in swap\n");
        return;
    }
    
    if (!fb_info.address) {
        serial_printf("Error: framebuffer address is 0\n");
        return;
    }
    
    // 直接使用物理地址访问前缓冲区（恒等映射）
    uint32_t* frontbuffer = (uint32_t*)fb_info.address;
    uint32_t screen_size = fb_info.width * fb_info.height;
    
    serial_printf("Swapping buffers:\n");
    serial_printf("  Resolution: %dx%d\n", fb_info.width, fb_info.height);
    serial_printf("  Frontbuffer: 0x%llx\n", (uint64_t)frontbuffer);
    serial_printf("  Backbuffer: 0x%llx\n", (uint64_t)backbuffer);
    serial_printf("  Screen size: %u pixels\n", screen_size);
    
    // 将后缓冲区复制到前缓冲区
    for (uint32_t i = 0; i < screen_size; i++) {
        frontbuffer[i] = backbuffer[i];
    }
    
    serial_printf("Buffer swap completed successfully\n");
}

void kernel_main() 
{
    serial_init();
    serial_printf("=== SolumOS Kernel Started ===\n");
    serial_printf("Multiboot info structure at: 0x%llx\n", multiboot2_info_addr);
    
    // 初始化帧缓冲区
    struct framebuffer_info* fb_info = framebuffer_init(multiboot2_info_addr);
    
    if (!fb_info) {
        serial_printf("CRITICAL ERROR: Framebuffer initialization failed!\n");
        serial_printf("System will now halt...\n");
        while (1) {
            asm volatile ("hlt");
        }
    }
    
    serial_printf("Starting graphics test pattern...\n");
    
    // 先清空整个屏幕为黑色
    framebuffer_draw_rect(0, 0, fb_info->width, fb_info->height, COLOR_BLACK);
    
    // 计算屏幕中心
    int center_x = fb_info->width / 2;
    int center_y = fb_info->height / 2;
    int rect_size = 100; // 100x100像素的方块
    
    serial_printf("Screen center: (%d, %d)\n", center_x, center_y);
    
    // 在屏幕中央绘制一个红色大方块
    framebuffer_draw_rect(
        center_x - rect_size/2, 
        center_y - rect_size/2, 
        rect_size, 
        rect_size, 
        COLOR_RED
    );
    
    // 在红色方块内部绘制一个较小的绿色方块
    framebuffer_draw_rect(
        center_x - rect_size/4, 
        center_y - rect_size/4, 
        rect_size/2, 
        rect_size/2, 
        COLOR_GREEN
    );
    
    // 在绿色方块内部绘制一个更小的蓝色方块
    framebuffer_draw_rect(
        center_x - rect_size/8, 
        center_y - rect_size/8, 
        rect_size/4, 
        rect_size/4, 
        COLOR_BLUE
    );
    
    serial_printf("Test pattern drawn, swapping buffers...\n");
    
    // 交换缓冲区，显示内容
    framebuffer_swap_buffers();
    
    serial_printf("=== Graphics test completed successfully ===\n");
    serial_printf("You should now see concentric colored squares in the center of the screen.\n");
    
    // 主循环
    while (1) {
        asm volatile ("hlt"); // 节省CPU
    }
}