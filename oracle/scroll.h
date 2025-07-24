#ifndef SCROLL_H
#define SCROLL_H

#include <stdint.h> // standard integer library

void display_char(uint8_t display_idx, char ch);
void update_displays();
void scroller();

void show_next_string(void);
void show_full_msg(int idx);

// globals
extern volatile uint32_t press_len;
extern unsigned int scroll_idx;
extern unsigned int string_idx;

#endif
