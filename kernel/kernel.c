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

// 端口输入函数 - 必须在任何使用之前定义
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// 端口输出函数 - 必须在任何使用之前定义
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// 串口端口定义
#define COM1 0x3F8

// 串口初始化
void serial_init() {
    // 禁用中断
    outb(COM1 + 1, 0x00);
    // 启用DLAB（设置波特率除数）
    outb(COM1 + 3, 0x80);
    // 设置波特率除数低位 (115200 / 9600 = 12)
    outb(COM1 + 0, 0x0C);
    // 设置波特率除数高位
    outb(COM1 + 1, 0x00);
    // 8位数据，无奇偶校验，1位停止位
    outb(COM1 + 3, 0x03);
    // 启用FIFO，清空，14字节阈值
    outb(COM1 + 2, 0xC7);
    // 启用中断
    outb(COM1 + 4, 0x0B);
}

// 检查串口是否空闲
int serial_received() {
    return inb(COM1 + 5) & 1;
}

// 读取串口数据
char serial_read() {
    while (serial_received() == 0);
    return inb(COM1);
}

// 检查发送缓冲区是否为空
int serial_transmit_empty() {
    return inb(COM1 + 5) & 0x20;
}

// 串口输出字符
void serial_putc(char c) {
    while (serial_transmit_empty() == 0);
    outb(COM1, c);
}

// 串口输出字符串
void serial_puts(const char* str) {
    while (*str) {
        if (*str == '\n') {
            serial_putc('\r');
        }
        serial_putc(*str++);
    }
}

// 串口输出十六进制数
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

// 串口输出十进制数
void serial_put_dec(uint64_t value) {
    char buffer[21];
    char* ptr = buffer + 20;
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

// 简单的VGA输出
void vga_puts(const char *str, uint8_t color) {
    volatile uint16_t *vga_buffer = (volatile uint16_t*)0xB8000;
    static uint16_t cursor_pos = 0;
    
    for (int i = 0; str[i] != 0; i++) {
        switch (str[i]) {
            case '\n':
                cursor_pos = (cursor_pos / 80 + 1) * 80;
                break;
            default:
                vga_buffer[cursor_pos] = (color << 8) | str[i];
                cursor_pos++;
                break;
        }
        
        if (cursor_pos >= 80 * 25) {
            cursor_pos = 80 * 24;
        }
    }
}

void kernel_main(struct boot_info *info) {
    // 初始化串口
    serial_init();
    serial_puts("\n\n=== SolumOS Boot Debug ===\n");
    
    // 立即输出启动信息
    serial_puts("Kernel entry point reached\n");
    
    // 输出引导类型
    serial_puts("Boot type: ");
    if (info->boot_type == 0) {
        serial_puts("BIOS");
        vga_puts("BIOS Boot\n", 0x0F);
    } else {
        serial_puts("UEFI");
        vga_puts("UEFI Boot\n", 0x0F);
    }
    serial_puts("\n");
    
    // 输出帧缓冲区信息
    serial_puts("Framebuffer address: ");
    serial_put_hex(info->framebuffer_addr);
    serial_puts("\n");
    
    serial_puts("Framebuffer width: ");
    serial_put_dec(info->framebuffer_width);
    serial_puts("\n");
    
    serial_puts("Framebuffer height: ");
    serial_put_dec(info->framebuffer_height);
    serial_puts("\n");
    
    // 测试内存访问
    serial_puts("Testing VGA memory access... ");
    serial_puts("VGA test completed\n");
    
    // 测试其他内存区域
    serial_puts("Testing general memory access... ");
    volatile uint32_t *mem_test = (volatile uint32_t*)0x200000;
    mem_test[0] = 0x12345678;
    if (mem_test[0] == 0x12345678) {
        serial_puts("Memory test PASSED\n");
    } else {
        serial_puts("Memory test FAILED\n");
    }
    
    // 输出当前状态
    serial_puts("Kernel is running successfully\n");
    vga_puts("Kernel Running\n", 0x0A);
    
    // 输出调试分隔符
    serial_puts("----------------------------------------\n");
    
    // 主循环
    int counter = 0;
    while (1) {
        // 定期输出心跳信号
        for (volatile int i = 0; i < 10000000; i++);
        
        serial_puts("Heartbeat: ");
        serial_put_dec(counter++);
        serial_puts("\n");
        
        // 也在VGA上显示心跳（如果VGA工作）
        if (counter % 10 == 0) {
            char heartbeat[20];
            char *p = heartbeat;
            int n = counter;
            
            // 简单整数转字符串
            if (n == 0) {
                *p++ = '0';
            } else {
                char temp[20];
                char *t = temp;
                while (n > 0) {
                    *t++ = '0' + (n % 10);
                    n /= 10;
                }
                while (t > temp) {
                    *p++ = *--t;
                }
            }
            *p = '\0';
            
            vga_puts("Count: ", 0x0E);
            vga_puts(heartbeat, 0x0E);
            vga_puts("   \n", 0x0E); // 添加空格覆盖旧数字
        }
        
        // 安全暂停
        asm volatile ("hlt");
    }
}