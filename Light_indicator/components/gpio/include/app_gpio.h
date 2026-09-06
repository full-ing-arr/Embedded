#ifndef APP_GPIO_H
#define APP_GPIO_H

#include <stdbool.h>

typedef enum {
  GPIO_PULL_NONE,
  GPIO_PULL_UP,
  GPIO_PULL_DOWN,
} app_gpio_pull_t;

void app_gpio_input_init(int pin, app_gpio_pull_t pull);
void app_gpio_output_init(int pin, bool level);
bool app_gpio_read(int pin);
void app_gpio_write(int pin, bool level);

typedef enum {
  GPIO_RISING = 1 << 0,
  GPIO_FALLING = 1 << 1,
  GPIO_ANY_EDGE = GPIO_RISING | GPIO_FALLING,
  GPIO_LOW = 1 << 2,
  GPIO_HIGH = 1 << 3,
} app_gpio_trigger_t;

typedef void (*app_gpio_interrupt_cb_t)(void *arg);

// Manage listeners from app_main (pinned to CPU0), one active listener per pin.
// Callbacks run in ISR context; keep arg alive until stop returns.
// The first run installs the ISR service for the application lifetime.
// Disable is also callable from ISR, for example to mask a level interrupt.
void app_gpio_interrupt_run(int pin, app_gpio_trigger_t trigger,
  app_gpio_interrupt_cb_t callback, void *arg);
void app_gpio_interrupt_stop(int pin);
void app_gpio_interrupt_change(int pin, app_gpio_trigger_t trigger);
void app_gpio_interrupt_enable(int pin);
void app_gpio_interrupt_disable(int pin);

#endif // APP_GPIO_H
