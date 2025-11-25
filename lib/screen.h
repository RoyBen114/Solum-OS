#ifndef SCREEN_H
#define SCREEN_H

void screen_uproll_once();
void move_cursor_by_XY(uint8_t x,uint8_t y);
void clear_screen();
void vga_init();
void vga_printf(char *input_str,...);

#endif