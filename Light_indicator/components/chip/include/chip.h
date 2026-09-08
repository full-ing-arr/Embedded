#ifndef CHIP_H
#define CHIP_H

#include "hal/adc_types.h"

enum {
  PIN_IN = 1 << 0,
  PIN_OUT = 1 << 1
};

typedef enum {
  GPIO_RISING = 1 << 0,
  GPIO_FALLING = 1 << 1,
  GPIO_ANY_EDGE = GPIO_RISING | GPIO_FALLING,
  GPIO_LOW = 1 << 2,
  GPIO_HIGH = 1 << 3,
} app_gpio_trigger_t;

typedef enum {
  ADC_ABOVE = 1,
  ADC_BELOW = 2,
} app_adc_trigger_t;

enum {
  ADC_ONESHOT = 1u << 0,
  ADC_SCAN = 1u << 1,
  ADC_DMA = 1u << 2,
  ADC_ULP = 1u << 3,
  ADC_RF = 1u << 4
};

#define ADC_MONITORS(Resource, Unit, Count, Triggers, NoDriver) + (Count)
enum {
  CHIP_ADC_MONITOR_COUNT = 0
#include "adc.monitors"
};
#undef ADC_MONITORS

#endif // CHIP_H
