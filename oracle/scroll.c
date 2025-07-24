#include <ti/devices/msp/msp.h>
#include <text.h>
#include <string.h>
#include <scroll.h>
#define U6_SEG_A    GPIO_PIN_PA6
#define U6_SEG_B    GPIO_PIN_PB0
#define U6_SEG_C    GPIO_PIN_PB1
#define U6_SEG_D    GPIO_PIN_PA7
#define U6_SEG_E    GPIO_PIN_PB2
#define U6_SEG_F    GPIO_PIN_PB3
#define U6_SEG_G    GPIO_PIN_PB4

#define U5_SEG_A    GPIO_PIN_PB5
#define U5_SEG_B    GPIO_PIN_PA8
#define U5_SEG_C    GPIO_PIN_PA9
#define U5_SEG_D    GPIO_PIN_PA10
#define U5_SEG_E    GPIO_PIN_PA11
#define U5_SEG_F    GPIO_PIN_PB6
#define U5_SEG_G    GPIO_PIN_PB7

#define U9_SEG_A    GPIO_PIN_PB9
#define U9_SEG_B    GPIO_PIN_PB8
#define U9_SEG_C    GPIO_PIN_PA31
#define U9_SEG_D    GPIO_PIN_PA30
#define U9_SEG_E    GPIO_PIN_PA29
#define U9_SEG_F    GPIO_PIN_PA28
#define U9_SEG_G    GPIO_PIN_PA1

#define U4_SEG_A    GPIO_PIN_PA27
#define U4_SEG_B    GPIO_PIN_PA26
#define U4_SEG_C    GPIO_PIN_PB27
#define U4_SEG_D    GPIO_PIN_PB26
#define U4_SEG_E    GPIO_PIN_PB25
#define U4_SEG_F    GPIO_PIN_PA25
#define U4_SEG_G    GPIO_PIN_PA24

#define U3_SEG_A    GPIO_PIN_PA22
#define U3_SEG_B    GPIO_PIN_PB20
#define U3_SEG_C    GPIO_PIN_PB21
#define U3_SEG_D    GPIO_PIN_PB22
#define U3_SEG_E    GPIO_PIN_PB23
#define U3_SEG_F    GPIO_PIN_PB24
#define U3_SEG_G    GPIO_PIN_PA23

#define U2_SEG_A    GPIO_PIN_PA16
#define U2_SEG_B    GPIO_PIN_PA17
#define U2_SEG_C    GPIO_PIN_PA18
#define U2_SEG_D    GPIO_PIN_PB17
#define U2_SEG_E    GPIO_PIN_PB18
#define U2_SEG_F    GPIO_PIN_PB19
#define U2_SEG_G    GPIO_PIN_PA21

//high and lwo functions
//high
#define SET_PIN_HIGH(PORT, PIN)  ((PORT)->DOUTSET31_0 = (1 << (PIN)))
//low
#define SET_PIN_LOW(PORT, PIN)   ((PORT)->DOUTCLR31_0 = (1 << (PIN)))

//seg structure
typedef struct {
    uint8_t seg_a;
    uint8_t seg_b;
    uint8_t seg_c;
    uint8_t seg_d;
    uint8_t seg_e;
    uint8_t seg_f;
    uint8_t seg_g;
} SegmentPattern;

const SegmentPattern segment_lookup[31] = {
    {1,1,1,0,1,1,1}, // A
    {0,0,1,1,1,1,1}, // B
    {1,0,0,1,1,1,0}, // C
    {0,1,1,1,1,0,1}, // D
    {1,0,0,1,1,1,1}, // E
    {1,0,0,0,1,1,1}, // F
    {0,1,1,0,1,1,1}, // H
    {0,0,0,0,1,1,0}, // I
    {0,1,1,1,0,0,0}, // J
    {0,0,0,1,1,1,0}, // L
    {0,0,0,0,0,0,0}, // space, in text will be written as m
    {0,0,1,0,1,0,1}, // N
    {0,0,1,1,1,0,1}, // O
    {1,1,0,0,1,1,1}, // P
    {0,1,1,0,1,1,0}, // ||, in text will be written as q
    {0,0,0,0,1,0,1}, // R
    {1,0,1,1,0,1,1}, // S
    {0,1,1,1,1,1,0}, // U
    {0,0,0,0,0,0,1}, // -, in text will be written as v
    {0,1,1,1,0,1,1}, // Y
    {1,1,0,1,1,0,1}, // Z
    {1,1,1,1,1,1,0}, // 0
    {0,1,1,0,0,0,0}, // 1
    {1,1,0,1,1,0,1}, // 2
    {1,1,1,1,0,0,1}, // 3
    {0,1,1,0,0,1,1}, // 4
    {1,0,1,1,0,1,1}, // 5
    {1,0,1,1,1,1,1}, // 6
    {1,1,1,0,0,0,0}, // 7
    {1,1,1,1,1,1,1}, // 8
    {1,1,1,1,0,1,1} // 9

};

//disp pin mapping
typedef struct {
    GPIO_Regs* ports[7];
    uint8_t pins[7];
} Display;

//match displays to their pins, listed in order they are on the board
Display displays[6] = {
    {{GPIOA, GPIOB, GPIOB, GPIOA, GPIOB, GPIOB, GPIOB}, {6,0,1,7,2,3,4}},         // U6
    {{GPIOB, GPIOA, GPIOA, GPIOA, GPIOA, GPIOB, GPIOB}, {5,8,9,10,11,6,7}},       // U5
    {{GPIOB, GPIOB, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA}, {9,8,31,30,29,28,1}},     // U9
    {{GPIOA, GPIOA, GPIOB, GPIOB, GPIOB, GPIOA, GPIOA}, {27,26,27,26,25,25,24}}, // U4
    {{GPIOA, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOA}, {22,20,21,22,23,24,23}}, // U3
    {{GPIOA, GPIOA, GPIOA, GPIOB, GPIOB, GPIOB, GPIOA}, {16,17,18,17,18,19,21}}, // U2

};

//display characters where A = 0, ..., Z = 20 and 0 = 21, ..., 9 = 28
void display_char(uint8_t display_idx, char ch) {
    uint8_t index;

    if (ch >= 'A' && ch <= 'Z') {
        index = ch - 'A';
    } else if (ch >= '0' && ch <= '9') {
        index = 20 + (ch - '0');
    }else{
        return;
    }

    SegmentPattern pattern = segment_lookup[index];
    Display disp = displays[display_idx];

    uint8_t segs[7] = {
        pattern.seg_a, pattern.seg_b, pattern.seg_c,
        pattern.seg_d, pattern.seg_e, pattern.seg_f, pattern.seg_g
    };

    for (int i = 0; i < 7; i++) {
        if (segs[i])
            SET_PIN_HIGH(disp.ports[i], disp.pins[i]);
        else
            SET_PIN_LOW(disp.ports[i], disp.pins[i]);
    }
}

//set scrolling vars
#define DISPLAY_COUNT 6
volatile uint32_t press_len = 0;
unsigned int scroll_idx = 0;
unsigned int string_idx = 0;
uint32_t last_scroll_time = 0;
uint8_t reverse_held = 0;
uint32_t last_reverse_time = 0;

#define SET_PIN_HIGH(PORT,PIN)  ((PORT)->DOUTSET31_0 = (1u << (PIN)))
#define SET_PIN_LOW(PORT,PIN)  ((PORT)->DOUTCLR31_0 = (1u << (PIN)))

//update display when new letter
void update_displays() {
    for (int i = 0; i < DISPLAY_COUNT; i++) {
        char ch = text[string_idx][scroll_idx + i];
        if (ch == '\0') ch = ' ';
        display_char(i, ch);
    }
}

//forward automatic and backwards presses scroll
void scroller(void) {
    //Forward scroll every 500ms
    if (press_len - last_scroll_time >= 500) {
        scroll_idx++;
        if (scroll_idx > strlen(text[string_idx]))
            scroll_idx = 0;
        update_displays();
        last_scroll_time = press_len;
    }

    //Reverse button
    if (!(GPIOB->DIN31_0 & (1 << 15))) {  // button pressed
        if (!reverse_held || press_len - last_reverse_time >= 200) {
            if (scroll_idx > 0) scroll_idx--;
            update_displays();
            last_reverse_time = press_len;
            reverse_held = 1;
        }
    } else {
        reverse_held = 0;
    }
}
//int scrll_idx; // we use both scroll_idx and scrll_idx but i think we should only be using one of these unless i am misunderstanding something
//int button_press_len; // we use both press_len and button_press_len but i think we should only be using one of these unless i am misunderstanding something
void show_next_string() {
    while (!(any_button_on()));
    while (any_button_on());    // button depressed
    while (!(any_button_on())); // button released (debouncing measure)
}

void show_full_msg(int idx) {
    scroll_idx = 0; // changed scrll_idx to scroll_idx
    string_idx = idx;
    update_displays();

    int msg_len = strlen(text[idx]);
    // uint32_t start_time = button_press_len;
    uint32_t start_time = press_len;

    while (press_len - start_time < 4000) {  // scroll for 4 seconds
        scroller(); // auto scroll -- allows for entirety of a single string to be shown to the user
    }

    //this scroll is controlled by the user and waits for them to press the button to show the next string
    show_next_string();
}
