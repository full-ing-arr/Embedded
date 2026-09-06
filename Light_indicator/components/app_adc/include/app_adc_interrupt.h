#ifndef APP_ADC_INTERRUPT_H
#define APP_ADC_INTERRUPT_H

#include "hal/adc_types.h"

typedef enum {
  ADC_ABOVE = 1,
  ADC_BELOW = 2,
} app_adc_trigger_t;

typedef void (*app_adc_interrupt_cb_t)(void *arg);
typedef struct app_adc_interrupt *app_adc_interrupt_handle_t;

app_adc_interrupt_handle_t app_adc_interrupt_run(adc_unit_t unit,
  adc_channel_t channel, app_adc_trigger_t trigger, int threshold,
  app_adc_interrupt_cb_t callback, void *arg);
void app_adc_interrupt_change(app_adc_interrupt_handle_t interrupt,
  app_adc_trigger_t trigger, int threshold);
void app_adc_interrupt_stop(app_adc_interrupt_handle_t interrupt);

#endif // APP_ADC_INTERRUPT_H
