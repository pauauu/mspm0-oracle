#include <ti/devices/msp/msp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <oracle_helper.h>
#include <scroll.h>
#include <buttons.h>
extern int string_idx;
int timerTicked = 0;
/**
 * main.c
 */
enum current_state_enum {
    AWAKE,
    ASLEEP,
    MENU,
    FORTUNE,
    WISH,
    KARMA,
    OFF,  //other states need to be added (MENU, GAMEPLAY, etc.)
};

int timer = 0;
int rand_idx(int min, int max) {
    //current timer val as seed
    unsigned int seed = time(0);
    return (rand_r(&seed) % (max - min + 1) + min);
}
int karma = 0;

void TIMG0_IRQHandler(void){
    // This wakes up the processor!
    switch (TIMG0->CPU_INT.IIDX) {
        case GPTIMER_CPU_INT_IIDX_STAT_Z: // Counted down to zero event.
            timerTicked = 1; // set a flag so we can know what woke us up.
            break;
        default:
            break;
    }
}

uint32_t generateRandomNum(void) {
    int random_num = 0;
    TRNG->GPRCM.RSTCTL = TRNG_RSTCTL_RESETASSERT_ASSERT | TRNG_RSTCTL_KEY_UNLOCK_W;
    TRNG->GPRCM.PWREN  = TRNG_PWREN_KEY_UNLOCK_W | TRNG_PWREN_ENABLE_ENABLE;
    delay_cycles(POWER_STARTUP_DELAY);

    TRNG->CLKDIVIDE = (uint32_t)TRNG_CLKDIVIDE_RATIO_DIV_BY_2;

    update_reg(&TRNG->CTL, (uint32_t)TRNG_CTL_CMD_NORM_FUNC, TRNG_CTL_CMD_MASK);
    while (!(TRNG->CPU_INT.RIS & TRNG_RIS_IRQ_CMD_DONE_MASK));
    TRNG->CPU_INT.ICLR = TRNG_IMASK_IRQ_CMD_DONE_MASK;

    update_reg(&TRNG->CTL, ((uint32_t)0x3 << TRNG_CTL_DECIM_RATE_OFS), TRNG_CTL_DECIM_RATE_MASK);
    while (!(TRNG->CPU_INT.RIS & TRNG_RIS_IRQ_CAPTURED_RDY_MASK));
    TRNG->CPU_INT.ICLR = TRNG_IMASK_IRQ_CAPTURED_RDY_MASK;
    random_num = TRNG->DATA_CAPTURE;

    TRNG->GPRCM.PWREN = TRNG_PWREN_KEY_UNLOCK_W | TRNG_PWREN_ENABLE_DISABLE;
    return random_num;
}

//KARMA SETUP/HELPER FUNCTIONS
void show_next_string() {
    while (!(any_button_on()));
    while (any_button_on());    // button depressed
    while (!(any_button_on())); // button released (debouncing measure)
}

void show_full_msg(int idx) {
    scrll_idx = 0;
    string_idx = idx;

    int msg_len = strlen(text[idx]);
    uint32_t start_time = button_press_len;

    while (button_press_len - start_time < 4000) {  // scroll for 4 seconds
        scroller(); // auto scroll -- allows for entirety of a single string to be shown to the user
    }

    //this scroll is controlled by the user and waits for them to press the button to show the next string
    show_next_string();
}

//generate a fortune based on the karma score
uint32_t generate_fortune(int karma_score) {
    int base = 0;
    int range = 10;
    uint32_t random_num = generateRandomNum();
    int offset = random_num % range;

    if (karma_score >= 2 && karma_score <= 4) {
        // Good fortunes: 13–22
        base = 13;
    } else if (karma_score >= -4 && karma_score <= -1) {
        // Bad fortunes: 23–32
        base = 23;
    }
    return base + offset;

}

int main(void)
{
            InitializeProcessor();
            InitializeGPIO();
            InitializeTimerG0();
            InitializeTimerA1_PWM();
            /* Code to initialize GPIO PORT */
                //Reset GPIO port (the one(s) for pins that you will use) figure out which ones
                GPIOA->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETSTKYCLR_CLR | GPIO_RSTCTL_RESETASSERT_ASSERT);
                GPIOB->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETSTKYCLR_CLR | GPIO_RSTCTL_RESETASSERT_ASSERT);
                //Enable power on LED GPIO port
                GPIOA->GPRCM.PWREN = (GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE);
                GPIOB->GPRCM.PWREN = (GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE);
                delay_cycles(POWER_STARTUP_DELAY);
                //initialize specific GPIO PINS
                //U4
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM54)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U4 G --> PA24
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM55)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U4 F --> PA25
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM56)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U4 E --> PB25
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM57)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U4 D --> PB26
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM58)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U4 C --> PB27
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM59)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U4 B --> PA26
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM60)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U4 A --> PA27
                //U5
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM18)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U5 A --> PB5
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM19)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U5 B --> PA8
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM20)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U5 C --> PA9
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM21)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U5 D --> PA10
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM22)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U5 E --> PA11
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM23)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U5 F --> PB6
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM24)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U5 G --> PB7
                //U2
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM38)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // A --> PA16
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM39)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // B --> PA17
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM40)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // C --> PA18
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM43)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // D --> PB17
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM44)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // E --> PB18
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM45)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // F --> PB19
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM46)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // G --> PA21
                //U3
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM47)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // A --> PA22
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM48)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // B --> PB20
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM49)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // C --> PB21
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM50)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // D --> PB22
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM51)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // E --> PB23
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM52)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // F --> PB24
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM53)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // G --> PA23
                // U6
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM17)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U6 G --> PB4
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM16)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U6 F --> PB3
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM15)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U6 E --> PB2
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM14)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U6 D --> PA7
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM13)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U6 C --> PB1
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM12)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U6 B --> PB0
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM11)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U6 A --> PA6
                // BUZZER
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM27)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // BUZZER --> PB10
                // U9
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM26)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U9 A --> PB9
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM25)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U9 B --> PB8
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM6)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U9 C --> PA31
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM5)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U9 D --> PA30
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM4)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U9 E --> PA29
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM3)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U9 F --> PA28
                IOMUX->SECCFG.PINCM[(IOMUX_PINCM2)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // U9 G --> PA1
                //define PORT A output bitmask
                #define GPIOA_MASK ((1 << 1) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 16) | (1 << 17) | (1 << 18) | (1 << 21) | (1 << 22) | (1 << 23) | (1 << 24) | (1 << 25) | (1 << 26) | (1 << 27) | (1 << 28) | (1 << 29) | (1 << 30) | (1 << 31))
                #define GPIOB_MASK ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 17) | (1 << 18) | (1 << 19) | (1 << 20) | (1 << 21) | (1 << 22) | (1 << 23) | (1 << 24) | (1 << 25) | (1 << 26) | (1 << 27))
                //set segment and buzzer outputs low (initial state)
                GPIOA->DOUTCLR31_0 = GPIOA_MASK;
                GPIOB->DOUTCLR31_0 = GPIOB_MASK;
                //set direction as output for all the relevant pins
                GPIOA->DOESET31_0 = GPIOA_MASK;
                GPIOB->DOESET31_0 = GPIOB_MASK;
                uint8_t currSpace = 0;  // Tracks the current spacing needed between elements
                // Functional
                //ensure no unroll
                #pragma nounroll
            while(1){
                enum current_state_enum next_state;
                next_state = ASLEEP;
                while (1) {
                    while (!timerTicked)
                        __WFI();
                    timerTicked = 0;
                    timer = timer + 1;
                    int SW1_on = button_on(SW1);
                    int SW2_on = button_on(SW2);
                    int SW3_on = button_on(SW3);
                    switch(next_state) {
                        case ASLEEP:
                            string_idx = 1;
                            scroller();
                            if (any_button_on()) {
                                next_state = AWAKE;
                            }else {
                                next_state = ASLEEP;
                                break;
                            }break;
                        case AWAKE:
                            string_idx = 2;
                            scroller();
                            //if 5s have passed, and the user has not pressed any buttons, blink
                            if (timer%200 > 150) {
                                string_idx = 1;
                                scroller();
                            } else if (any_button_on()) {
                                next_state = MENU;
                            } else {
                                next_state = AWAKE;
                                break;
                            }break;
                        case MENU:

                            if (SW1_on){
                                next_state = AWAKE;
                            } else{
                                string_idx = 3;
                                show_full_msg(3);
                                show_full_msg(4);
                                if (timer%100 > 150){
                                if (sw3_count() == 3){
                                    show_full_msg(1);
                                    if (timer%200 > 150){
                                        next_state = ASLEEP;}
                                    reset_sw3_count();
                                }else if (sw3_count() == 2){
                                    show_full_msg(7);
                                    if (timer%200 > 150){
                                        next_state = WISH;}
                                    reset_sw3_count();
                                }else if (sw3_count() == 1){
                                    show_full_msg(5);
                                    if (timer%200 > 150){
                                        next_state = FORTUNE;}
                                    reset_sw3_count();
                                }
                                else{
                                    show_full_msg(4);
                                    if (timer%100 > 150){
                                        next_state = AWAKE;
                                    }
                            }}else{
                                next_state = AWAKE;
                            }

                            break;
                        case FORTUNE:
                            if (SW1_on){
                                next_state = AWAKE;
                            }else{
                                string_idx = 6;
                                scroller();
                                if (timer%100 > 150){
                                    if (sw3_count() == 1){
                                        int rd_idx = rand_idx(13,22);
                                        show_full_msg(4);
                                        if (timer%100 > 150){
                                            next_state = MENU;
                                        }
                                        reset_sw3_count();
                                    }else{
                                        int rd_idx = rand_idx(23,32);
                                        show_full_msg(4);
                                        if (timer%100 > 150){
                                            next_state = MENU;
                                        }
                                        reset_sw3_count();
                                    }
                                }
                            }
                            break;

                            break;
                        case OFF:
                            string_idx = 0;
                            break;
                        default:
                            break;
                    };
                }
                return 0;
            }
            while (!timerTicked) // Wait for timer wake up
                __WFI();
            timerTicked = 0;
    return 0;
}
}
