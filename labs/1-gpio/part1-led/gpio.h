#ifndef __GPIO_H__
#define __GPIO_H__

// List of functions for GPIO pins.
// The ordering of ALT values is weird, but correct to the spec.
typedef enum {
    GPIO_FUNC_INPUT  = 0b000,
    GPIO_FUNC_OUTPUT = 0b001,
    GPIO_FUNC_ALT0   = 0b100,
    GPIO_FUNC_ALT1   = 0b101,
    GPIO_FUNC_ALT2   = 0b110,
    GPIO_FUNC_ALT3   = 0b111,
    GPIO_FUNC_ALT4   = 0b011,
    GPIO_FUNC_ALT5   = 0b010,
} gpio_func_t;

// set <pin> to be an output pin.
void gpio_set_output(unsigned pin);

// set <pin> to input.
void gpio_set_input(unsigned pin);

// set GPIO <pin> on.
void gpio_set_on(unsigned pin);

// set GPIO <pin> off
void gpio_set_off(unsigned pin);

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v);

// return the value of <pin>.
int gpio_read(unsigned pin);

#endif
