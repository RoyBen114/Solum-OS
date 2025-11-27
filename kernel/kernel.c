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

    parse_mb_info();
    
    serial_printf("请输入一个整数和一个字符串 (例如: 123 hello): ");
    int number;
    char text[32];
    int items = serial_scanf("%d %s", &number, text);
    serial_printf("成功读取 %d 个参数: 数字=%d, 文本=%s\n", items, number, text);
}