#ifndef PORT_H
#define PORT_H

#include <stdint.h>

inline void outb(uint16_t port, uint8_t value);
inline uint8_t inb(uint16_t port);
inline uint16_t inw(uint16_t port);

#endif