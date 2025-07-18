#include <ti/devices/msp/msp.h>
#include "buttons.h"
#include "oracle_helper.h"

int button_on(uint32_t SW){
    return !((GPIOB->DIN31_0 & SW) == SW);
}

int any_button_on(){
    return (button_on(SW1) | button_on(SW2) | button_on(SW3));
}

//press ct and debounce vars
int sw3_ct = 0;
uint8_t sw3_last_state = 1;
uint32_t sw3_db_time = 0;

void sw3_counter(uint32_t curr_t) {
    int current_state = button_on(SW3);

    if (current_state != sw3_last_state) {
        sw3_db_time = curr_t;
    }

    if ((curr_t - sw3_db_time) > 20) {
        if (current_state && !sw3_last_state) {
            sw3_ct = sw3_ct + 1;
        }
        sw3_last_state = current_state;
    }
}

int sw3_count() {
    return sw3_ct;
}

void reset_sw3_count() {
    sw3_ct = 0;
}
