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

#ifndef INFO_H
#define INFO_H

struct multiboot2_tag
{
    uint32_t type;
    uint32_t size;
};

struct multiboot2_tag_framebuffer
{
    uint32_t type;
    uint32_t size;
    uint64_t fb_addr;
    uint32_t fb_pitch;
    uint32_t fb_width;
    uint32_t fb_height;
    uint8_t fb_bpp;
    uint8_t fb_type;
    uint8_t reserved;
};

struct multiboot2_info
{
    uint32_t total_size;
    uint32_t reserved;
    uint8_t tags[];
};

void parse_mb_info(void);

extern int is_graphics_mode;
extern struct multiboot2_info *mbi;
extern uint8_t *current_tag;
extern struct multiboot2_tag *tag;
extern struct multiboot2_tag_framebuffer *fb_info;

#endif

