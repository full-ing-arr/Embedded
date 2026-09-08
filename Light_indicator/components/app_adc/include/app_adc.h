#ifndef APP_ADC_H
#define APP_ADC_H

#include <stdbool.h>
#include <stdint.h>
#include "chip.h"
#include "esp_adc/adc_monitor.h"

typedef struct {
  unsigned sample;
  int raw;
} adc_t;

typedef void (*app_adc_interrupt_cb_t)(void *arg);

typedef struct {
  adc_channel_t channel;
  adc_monitor_handle_t handle;
  app_adc_interrupt_cb_t callback;
  void *arg;
  app_adc_trigger_t trigger;
  int16_t threshold;
  uint8_t slot;
} app_adc_interrupt_t;

adc_t adc_init(void);

int app_adc_oneshot(adc_t *source, adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bitwidth_t bitwidth);
void app_adc_start_sampling(adc_t *source, adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bitwidth_t bitwidth, uint32_t rate_hz);
void app_adc_stop_sampling(adc_channel_t channel);
bool app_adc_latest(const adc_t *source, int *raw);
void app_adc_interrupt_create(app_adc_interrupt_t *interrupt, adc_channel_t channel, app_adc_trigger_t trigger, int threshold, app_adc_interrupt_cb_t callback, void *arg);
void app_adc_interrupt_run(app_adc_interrupt_t *interrupt);
void app_adc_interrupt_change(app_adc_interrupt_t *interrupt, app_adc_trigger_t trigger, int threshold);
void app_adc_interrupt_stop(app_adc_interrupt_t *interrupt);

#endif // APP_ADC_H
