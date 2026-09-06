#include "app_gpio.h"

#include <stdlib.h>

#include "driver/gpio.h"
#include "esp_err.h"

static void gpio_check_pin(int pin, bool output) {
  bool valid = output ? GPIO_IS_VALID_OUTPUT_GPIO(pin) : GPIO_IS_VALID_GPIO(pin);

  if (!valid) {
    ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    abort();
  }
}

static void gpio_configure(int pin, gpio_mode_t mode, app_gpio_pull_t pull) {
  const gpio_config_t config = {
    .pin_bit_mask = 1ULL << pin,
    .mode = mode,
    .pull_up_en = pull == GPIO_PULL_UP ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
    .pull_down_en = pull == GPIO_PULL_DOWN ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };

  ESP_ERROR_CHECK(
    gpio_config(&config));
}

void app_gpio_input_init(int pin, app_gpio_pull_t pull) {
  gpio_check_pin(pin, false);

  if ((unsigned)pull > GPIO_PULL_DOWN) {
    ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    abort();
  }

  gpio_configure(pin, GPIO_MODE_INPUT, pull);
}

void app_gpio_output_init(int pin, bool level) {
  gpio_check_pin(pin, true);

  ESP_ERROR_CHECK(
    gpio_set_level(pin, level));

  gpio_configure(pin, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
}

bool app_gpio_read(int pin) {
  gpio_check_pin(pin, false);
  return gpio_get_level(pin) != 0;
}

void app_gpio_write(int pin, bool level) {
  gpio_check_pin(pin, true);

  ESP_ERROR_CHECK(
    gpio_set_level(pin, level));
}
