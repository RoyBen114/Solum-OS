#include <stdint.h>

int xpos = 0;
int ypos = 0;
volatile unsigned char *video = (volatile unsigned char *)0xB8000;

struct boot_info
{
    uint32_t boot_type;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_bpp;
};

//===================================================serial相关函数(串口操作)==================================================
// 端口输入函数
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// 端口输出函数
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// 串口初始化
void serial_init()
{
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x0C);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

void serial_puts(const char *str)
{
    while (*str)
    {
        while ((inb(0x3F8 + 5) & 0x20) == 0)
            ;
        outb(0x3F8, *str++);
    }
}

void serial_put_hex(uint64_t value)
{
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[17];
    buffer[16] = '\0';

    for (int i = 15; i >= 0; i--)
    {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }

    serial_puts("0x");
    serial_puts(buffer);
}

void serial_put_dec(uint32_t value)
{
    char buffer[11];
    char *ptr = buffer + 10;
    *ptr = '\0';

    if (value == 0)
    {
        serial_puts("0");
        return;
    }

    while (value > 0)
    {
        *--ptr = '0' + (value % 10);
        value /= 10;
    }

    serial_puts(ptr);
}

//===================================================kprintf相关函数(VGA操作)==================================================
static void itoa(char *buf, int base, int d)
{
    char *p = buf;
    char *p1, *p2;
    unsigned long ud = d;
    int divisor = 10;

    if (base == 'd' && d < 0)
    {
        *p++ = '-';
        buf++;
        ud = -d;
    }
    else if (base == 'x')
        divisor = 16;

    do
    {
        int remainder = ud % divisor;

        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (ud /= divisor);

    *p = 0;

    p1 = buf;
    p2 = p - 1;
    while (p1 < p2)
    {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

static void putchar(int c)
{
    if (c == '\n' || c == '\r')
    {
    newline:
        xpos = 0;
        ypos++;
        if (ypos >= 24)
            ypos = 0;
        return;
    }

    *(video + (xpos + ypos * 80) * 2) = c & 0xFF;
    *(video + (xpos + ypos * 80) * 2 + 1) = 0x07;

    xpos++;
    if (xpos >= 80)
        goto newline;
}

void kprintf(const char *format, ...)
{
    char **arg = (char **)&format;
    int c;
    char buf[20];

    arg++;

    while ((c = *format++) != 0)
    {
        if (c != '%')
            putchar(c);
        else
        {
            char *p, *p2;
            int pad0 = 0, pad = 0;

            c = *format++;
            if (c == '0')
            {
                pad0 = 1;
                c = *format++;
            }

            if (c >= '0' && c <= '9')
            {
                pad = c - '0';
                c = *format++;
            }

            switch (c)
            {
            case 'd':
            case 'u':
            case 'x':
                itoa(buf, c, *((int *)arg++));
                p = buf;
                goto string;
                break;

            case 's':
                p = *arg++;
                if (!p)
                    p = "(null)";

            string:
                for (p2 = p; *p2; p2++)
                    ;
                for (; p2 < p + pad; p2++)
                    putchar(pad0 ? '0' : ' ');
                while (*p)
                    putchar(*p++);
                break;

            default:
                putchar(*((int *)arg++));
                break;
            }
        }
    }
}

void cls()
{
    int i;
    for (i = 0; i < 80 * 25 * 2; i++)
        *(video + i) = 0;
    xpos = 0;
    ypos = 0;
}

void kernel_main(struct boot_info *info) 
{
    // 初始化串口
    serial_init();
    serial_puts("\n === Debug ===\n");

    // 输出详细的引导信息
    serial_puts("Boot type: ");
    if (info->boot_type == 0)
    {
        serial_puts("BIOS\n");
    }
    else
    {
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

    serial_puts("Kernel initialization complete\n");

    cls();
    kprintf("abcde");
}