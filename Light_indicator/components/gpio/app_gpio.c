#include "app_gpio.h"

#include <stdlib.h>

#include "driver/gpio.h"
#include "esp_err.h"

static bool service_installed;

static void check(esp_err_t error) {
  if (error == ESP_OK)
    return;
  ESP_ERROR_CHECK(error);
  abort();
}

static void configure(int pin, gpio_mode_t mode, app_gpio_pull_t pull) {
  const gpio_config_t config = {
    .pin_bit_mask = 1ULL << pin,
    .mode = mode,
    .pull_up_en = pull == GPIO_PULL_UP ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
    .pull_down_en = pull == GPIO_PULL_DOWN ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  check(gpio_config(&config));
}

void app_gpio_input_init(int pin, app_gpio_pull_t pull) {
  configure(pin, GPIO_MODE_INPUT, pull);
}

void app_gpio_output_init(int pin, bool level) {
  check(gpio_set_level(pin, level));
  configure(pin, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
}

bool app_gpio_read(int pin) {
  return gpio_get_level(pin) != 0;
}

void app_gpio_write(int pin, bool level) {
  check(gpio_set_level(pin, level));
}

static gpio_int_type_t interrupt_type(app_gpio_trigger_t trigger) {
  switch (trigger) {
    case GPIO_RISING:
      return GPIO_INTR_POSEDGE;
    case GPIO_FALLING:
      return GPIO_INTR_NEGEDGE;
    case GPIO_ANY_EDGE:
      return GPIO_INTR_ANYEDGE;
    case GPIO_LOW:
      return GPIO_INTR_LOW_LEVEL;
    case GPIO_HIGH:
      return GPIO_INTR_HIGH_LEVEL;
    default: __builtin_unreachable();
  }
}

void app_gpio_interrupt_run(int pin, app_gpio_trigger_t trigger, app_gpio_interrupt_cb_t callback, void *arg) {
  if (!service_installed) {
    check(gpio_install_isr_service(0));
    service_installed = true;
  }
  check(gpio_set_intr_type(pin, interrupt_type(trigger)));
  check(gpio_isr_handler_add(pin, callback, arg));
}

void app_gpio_interrupt_stop(int pin) {
  check(gpio_isr_handler_remove(pin));
  check(gpio_set_intr_type(pin, GPIO_INTR_DISABLE));
}

void app_gpio_interrupt_change(int pin, app_gpio_trigger_t trigger) {
  check(gpio_intr_disable(pin));
  check(gpio_set_intr_type(pin, interrupt_type(trigger)));
  check(gpio_intr_enable(pin));
}

void app_gpio_interrupt_enable(int pin) {
  check(gpio_intr_enable(pin));
}

void app_gpio_interrupt_disable(int pin) {
  check(gpio_intr_disable(pin));
}
