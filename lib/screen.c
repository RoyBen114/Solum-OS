#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "screen.h"
#include "string.h"
#include "port.h"

static uint8_t tab_length=4;

typedef enum char_color {
    black = 0,
    blue = 1,
    green = 2,
    cyan = 3,
    red = 4,
    magenta = 5,
    brown = 6,
    light_grey = 7,
    dark_grey = 8,
    light_blue = 9,
    light_green = 10,
    light_cyan = 11,
    light_red = 12,
    light_magenta = 13,
    yellow  = 14,   
    white = 15
} vga_color_t;

static uint8_t *video_memory = (uint8_t *)0xB8000;

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

static void move_cursor()
{
    uint16_t cursorLocation = cursor_y * 80 + cursor_x;
    outb(0x3D4, 14);                   
    outb(0x3D5, cursorLocation >> 8);  
    outb(0x3D4, 15);                   
    outb(0x3D5, cursorLocation);    
}

void move_cursor_by_XY(uint8_t x,uint8_t y){
	if(x>79||y>24)
		return ;
	cursor_x=x;
	cursor_y=y;
	move_cursor();
}

void clear_screen(){
	cursor_x=0;
	cursor_y=0;
	for(int j=0;j<25;j++){
		for(int i=0;i<80;i++)
			vga_putc('\0',black,white);
	}
	cursor_x=0;
	cursor_y=0;
	move_cursor();
}

void screen_uproll_once(){ 
	for(int j=1;j<25;j++){
		for(int i=0;i<80;i++){
			uint16_t cursorLocation = j * 80 + i;
			uint16_t targetCursorLocation = (j-1) * 80 + i;
			*((uint16_t *)video_memory+targetCursorLocation)=*((uint16_t *)video_memory+cursorLocation);										
		}
	}
	for(int i=0;i<80;i++){
		uint16_t lastRowCur=24*80+i;
		*((char *)video_memory+2*lastRowCur)='\0';
		*((char *)video_memory+2*lastRowCur+1)=0x0F;									

	}
	if(cursor_y!=0){
		cursor_y--;
		move_cursor();
	}
}
void vga_putc(char input,vga_color_t back,vga_color_t fore){
	uint16_t cursorLocation = cursor_y * 80 + cursor_x;
	*((char *)video_memory+2*cursorLocation)=input;
	*((char *)video_memory+2*cursorLocation+1)=(back<<4)|(fore&0x0F);
	if(cursor_x==79){
		if(cursor_y==24){
			screen_uproll_once();
			cursor_x=0;
			cursor_y=24;
		}
		else{
			cursor_x=0;
			cursor_y++;
		}
	}
	else{
		cursor_x++;
	}
	move_cursor();
}
void putc_color(char input,vga_color_t back,vga_color_t fore){
	switch (input)
	{
		case '\t':{
			for(int i=0;i<tab_length;i++)
				vga_putc('\0',back,fore);
			break;
		}
		case '\n':{
			uint8_t temp_num = 80-cursor_x;
			if(temp_num==0)
				temp_num=80;
			for(int i=0;i<temp_num;i++)
			vga_putc('\0',back,fore);
			break;
		}
		default:
			vga_putc(input,back,fore);	
	}
}

void putc(char input){
	putc_color(input,black,white);
}

void puts_color(char * input_str,vga_color_t back,vga_color_t fore){
	char * probe=input_str;
	while(*probe!='\0')
		putc_color(*probe++,back,fore);
}

void puts(char *input_str){
	puts_color(input_str,black,white);
}

void vga_init(){
	clear_screen();
}

void insert_str(char *inserted_str,char *inserting_str,uint32_t offset) 
{
	char m[100]={0};
	char *afterInsetedPositionStr=m;
	strcpy(afterInsetedPositionStr,inserted_str+offset+2);
	memcpy(inserted_str+offset,inserting_str,strlen(inserting_str));
	*(inserted_str+offset+strlen(inserting_str))='\0';
	strcat(inserted_str,afterInsetedPositionStr);
	return inserted_str;
}

void vga_printf(char *input_str,...)
{
	static char staticArry[100]={0};
	char *output_str=staticArry;
	strcpy(output_str,input_str);
	va_list ptr;
	va_start(ptr,output_str);
	int offset=0;
	for(;*(output_str+offset)!='\0';offset++)
	{
		char *charptr=output_str+offset;
		if (*charptr=='%')
		{
			if (*(charptr+1)=='s')
			{
				char *arg_str_ptr=va_arg(ptr,char*);

				insert_str(output_str,arg_str_ptr,offset);
				
				offset=offset+strlen(arg_str_ptr)-1;
				
			}
			else if(*(charptr+1)=='d')
			{
				int arg_int=va_arg(ptr,int);

				char *temp_ptr=uintTostring(arg_int);

				insert_str(output_str,temp_ptr,offset);
				
				offset=offset+strlen(temp_ptr)-1;
			}
			else if(*(charptr+1)=='c')
			{
				;
			}
			else if(*(charptr+1)=='H')
			{
				int arg_int=va_arg(ptr,int);

				char*hexstrptr=num2hexstr(arg_int,1);
				
				insert_str(output_str,hexstrptr,offset);

				offset=offset+strlen(hexstrptr)-1;
			} 
			else if(*(charptr+1)=='h')
			{
				int arg_int=va_arg(ptr,int);

				char*hexstrptr=num2hexstr(arg_int,0);
				
				insert_str(output_str,hexstrptr,offset);

				offset=offset+strlen(hexstrptr)-1;
			}
		}
	}
	va_end(ptr);
	puts(output_str);
}

void printbasic(char *format_str,char *m)
{
	char *formatStr=format_str;
	int i=0;
	for(char *head=formatStr;*(head+i)!='\0';i++)
	{
		if (*(head+i)=='%'){
			if(*(head+i+1)=='s')
			{
				insert_str(format_str,m,i);
				
			}
			else if(*(head+i+1)=='d')
			{

			}
			else;
		}
	}
	
}

void printk_color(char *input_str,vga_color_t back,vga_color_t fore,...)
{
	static char staticArry[100]={0};
	char *output_str=staticArry;
	strcpy(output_str,input_str);
	va_list ptr;
	va_start(ptr,output_str);
	int offset=0;
	for(;*(output_str+offset)!='\0';offset++)
	{
		char *charptr=output_str+offset;
		if (*charptr=='%')
		{
			if (*(charptr+1)=='s')
			{
				char *arg_str_ptr=va_arg(ptr,char*);

				insert_str(output_str,arg_str_ptr,offset);
				
				offset=offset+strlen(arg_str_ptr)-1;
				
			}
			else if(*(charptr+1)=='d')
			{
				int arg_int=va_arg(ptr,int);

				char *temp_ptr=uintTostring(arg_int);

				insert_str(output_str,temp_ptr,offset);
				
				offset=offset+strlen(temp_ptr)-1;
			}
			else if(*(charptr+1)=='c')
			{
			
			}
			else if(*(charptr+1)=='H')
			{
				int arg_int=va_arg(ptr,int);

				char*hexstrptr=num2hexstr(arg_int,1);
				
				insert_str(output_str,hexstrptr,offset);

				offset=offset+strlen(hexstrptr)-1;
			} 
			else if(*(charptr+1)=='h')
			{
				int arg_int=va_arg(ptr,int);

				char*hexstrptr=num2hexstr(arg_int,0);
				
				insert_str(output_str,hexstrptr,offset);

				offset=offset+strlen(hexstrptr)-1;
			}
		}
	}
	va_end(ptr);
	puts_color(output_str,back,fore);
}