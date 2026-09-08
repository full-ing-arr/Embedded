#ifndef APP_GPIO_H
#define APP_GPIO_H

#include <stdbool.h>
#include "chip.h"

typedef enum {
  GPIO_PULL_NONE,
  GPIO_PULL_UP,
  GPIO_PULL_DOWN,
} app_gpio_pull_t;

typedef void (*app_gpio_interrupt_cb_t)(void *arg);

void app_gpio_input_init(int pin, app_gpio_pull_t pull);
void app_gpio_output_init(int pin, bool level);
bool app_gpio_read(int pin);
void app_gpio_write(int pin, bool level);

void app_gpio_interrupt_run(int pin, app_gpio_trigger_t trigger, app_gpio_interrupt_cb_t callback, void *arg);
void app_gpio_interrupt_stop(int pin);
void app_gpio_interrupt_change(int pin, app_gpio_trigger_t trigger);
void app_gpio_interrupt_enable(int pin);
void app_gpio_interrupt_disable(int pin);

#endif // APP_GPIO_H
