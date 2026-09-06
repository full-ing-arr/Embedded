#include "app_gpio.h"

#include <stdint.h>
#include <stdlib.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static bool service_installed;
static uint64_t active_pins;

static _Noreturn void fail(esp_err_t error) {
  ESP_ERROR_CHECK(error);
  abort();
}

static void check(esp_err_t error) {
  if (error != ESP_OK) fail(error);
}

static uint64_t prepare(int pin) {
  if (xPortInIsrContext() ||
      xTaskGetSchedulerState() != taskSCHEDULER_RUNNING ||
      xTaskGetCoreID(NULL) != 0) {
    fail(ESP_ERR_INVALID_STATE);
  }
  if (!GPIO_IS_VALID_GPIO(pin)) fail(ESP_ERR_INVALID_ARG);
  return 1ULL << pin;
}

static gpio_int_type_t interrupt_type(app_gpio_trigger_t trigger) {
  switch (trigger) {
    case GPIO_RISING:  return GPIO_INTR_POSEDGE;
    case GPIO_FALLING: return GPIO_INTR_NEGEDGE;
    case GPIO_ANY_EDGE: return GPIO_INTR_ANYEDGE;
    case GPIO_LOW:     return GPIO_INTR_LOW_LEVEL;
    case GPIO_HIGH:    return GPIO_INTR_HIGH_LEVEL;
    default: fail(ESP_ERR_INVALID_ARG);
  }
}

void app_gpio_interrupt_run(int pin, app_gpio_trigger_t trigger,
  app_gpio_interrupt_cb_t callback, void *arg) {
  const uint64_t mask = prepare(pin);
  const gpio_int_type_t type = interrupt_type(trigger);
  if (callback == NULL) fail(ESP_ERR_INVALID_ARG);
  if (active_pins & mask) fail(ESP_ERR_INVALID_STATE);

  if (!service_installed) {
    check(gpio_install_isr_service(0));
    service_installed = true;
  }

  check(gpio_intr_disable(pin));
  check(gpio_set_intr_type(pin, type));
  check(gpio_isr_handler_add(pin, callback, arg));
  active_pins |= mask;
}

void app_gpio_interrupt_stop(int pin) {
  const uint64_t mask = prepare(pin);
  if (!(active_pins & mask)) return;

  check(gpio_isr_handler_remove(pin));
  check(gpio_set_intr_type(pin, GPIO_INTR_DISABLE));
  active_pins &= ~mask;
}

void app_gpio_interrupt_change(int pin, app_gpio_trigger_t trigger) {
  const uint64_t mask = prepare(pin);
  const gpio_int_type_t type = interrupt_type(trigger);
  if (!(active_pins & mask)) fail(ESP_ERR_INVALID_STATE);

  check(gpio_intr_disable(pin));
  check(gpio_set_intr_type(pin, type));
  check(gpio_intr_enable(pin));
}

void app_gpio_interrupt_enable(int pin) {
  const uint64_t mask = prepare(pin);
  if (!(active_pins & mask)) fail(ESP_ERR_INVALID_STATE);
  check(gpio_intr_enable(pin));
}

void app_gpio_interrupt_disable(int pin) {
  check(gpio_intr_disable(pin));
}
