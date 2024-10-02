#include "pins.h"
#include "driver/gpio.h"
#include <sys/types.h>

#define LED_PIN_1 GPIO_NUM_12
#define LED_PIN_2 GPIO_NUM_13
#define BUTTON_PIN GPIO_NUM_18
bool buttonPressed = false;



void initPins() {
    gpio_config_t config;
    config.pin_bit_mask = (u_int64_t)1 << LED_PIN_1;
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_down_en = 0;
    config.pull_up_en = 0;
    // set the config, this takes care of the registers
    gpio_config(&config);
    config.pin_bit_mask = (u_int64_t)1 << LED_PIN_2;
    gpio_config(&config);
    config.pin_bit_mask = (u_int64_t)1 << BUTTON_PIN;
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = 1;
    gpio_config(&config);

}
/* switches LED A on if level!=0 or off if level==0*/
void setLEDPin(uint8_t level) {
        // set the pin of LED A to ON
        // you probably need to use gpio_set_level()
        gpio_set_level(LED_PIN_1, level);
}   

/* switches LED B on if level!=0 or off if level==0*/
void setLEDPin2(uint8_t level) {
        // set the pin of LED B to ON
        // you probably need to use gpio_set_level()
        gpio_set_level(LED_PIN_2, level);
}
void setLED(uint8_t pinN, uint8_t level){
    gpio_set_level(pinN, level);
}