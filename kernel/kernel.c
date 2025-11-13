unsigned char *videobuf = (unsigned char *)0xb8000;
void cls(){
    for (int i = 0; i < 80 * 25; i++)
    {
        videobuf[i * 2 + 0] = ' ';
        videobuf[i * 2 + 1] = 0x0F;
    }
}
void print(const char *str,int x,int y){
    for (int i = 0; str[i]; i++)
    {
        videobuf[(i+x) * 2 + 0 + y * 160] = str[i];
        videobuf[(i+x) * 2 + 1 + y * 160] = 0x0F;
    }
}
int kernel_main()
{
    print("Welcome to Solum OS",0,2);
    return 0;
}