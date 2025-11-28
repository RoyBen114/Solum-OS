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
    /* Add bounds check to avoid infinite loop if framebuffer tag not present */
    uint8_t *mb_end = (uint8_t *)mbi + mbi->total_size;

    while (current_tag + sizeof(struct multiboot2_tag) <= mb_end) {
        tag = (struct multiboot2_tag *)current_tag;

        /* validate tag size */
        if (tag->size == 0) break;

        /* find framebuffer tag (8) */
        if (tag->type == 8) {
            if ((uint8_t *)tag + tag->size > mb_end) break; /* malformed */

            fb_info = (struct multiboot2_tag_framebuffer *)tag;

            if (fb_info->fb_width > 80 || fb_info->fb_height > 25 || fb_info->fb_bpp > 16) {
                is_graphics_mode = 1;
            } else {
                is_graphics_mode = 0;
            }
            return;
        }

        current_tag += (tag->size + 7) & ~7;
    }

    /* If we reach here, framebuffer not found or malformed - fallback to text mode */
    is_graphics_mode = 0;
}