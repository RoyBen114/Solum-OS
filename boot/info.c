/*
 * Copyright (C) 2025 Roy Roy123ty@hotmail.com
 * 
 * This file is part of Solum OS
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stddef.h>
#include <boot/info.h>

extern uint64_t multiboot2_info_addr;

int is_graphics_mode = 0;
struct multiboot2_info *mbi;
uint8_t *current_tag;
struct multiboot2_tag *tag;
struct multiboot2_tag_framebuffer *fb_info;

void parse_mb_info()
{
    mbi = (struct multiboot2_info *)multiboot2_info_addr;
    current_tag = mbi->tags;

    while (1)
    {
        tag = (struct multiboot2_tag *)current_tag;

        // find framebuffer tag (8)
        if (tag->type == 8)
        {
            fb_info = (struct multiboot2_tag_framebuffer *)tag;

            if (fb_info->fb_width > 80 || fb_info->fb_height > 25 || fb_info->fb_bpp > 16)
            {
                is_graphics_mode = 1;
            }
            else
            {
                is_graphics_mode = 0;
            }
            break;
        }

        current_tag += (tag->size + 7) & ~7;
    }
}