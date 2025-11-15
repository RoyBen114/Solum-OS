unsigned char *videobuf = (unsigned char *)0xb8000;

void cls(){
    for (int i = 0; i < 80 * 25; i++)
    {
        videobuf[i * 2 + 0] = ' ';
        videobuf[i * 2 + 1] = 0x0F;
    }
}

void print(const char *str, int x, int y){
    for (int i = 0; str[i]; i++)
    {
        videobuf[(i+x) * 2 + 0 + y * 160] = str[i];
        videobuf[(i+x) * 2 + 1 + y * 160] = 0x0F;
    }
}

// 使用System V AMD64 ABI调用约定
__attribute__((sysv_abi))
int kernel_main(unsigned long mb_info)
{
    cls();
    print("Welcome to Solum OS", 0, 1);
    
    // 显示multiboot信息地址
    char info_msg[50];
    const char *hex = "0123456789ABCDEF";
    unsigned long addr = mb_info;
    
    // 构建消息
    char *ptr = info_msg;
    const char *prefix = "MB Info: 0x";
    for (int i = 0; prefix[i]; i++) {
        *ptr++ = prefix[i];
    }
    
    // 转换为十六进制
    for (int i = 60; i >= 0; i -= 4) {
        int nibble = (addr >> i) & 0xF;
        *ptr++ = hex[nibble];
    }
    *ptr = 0;
    
    print(info_msg, 0, 2);
    
    return 0;
}