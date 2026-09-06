#ifndef APP_ADC_H
#define APP_ADC_H

#include "esp_adc/adc_oneshot.h"

typedef struct {
    adc_oneshot_unit_handle_t handle;
    adc_cali_handle_t cali_handle;
    int milivolts;
} adc_t;

adc_t adc_init(adc_unit_t unit, adc_channel_t channel);
void adc_read_channel(adc_t *adc, adc_channel_t channel);

#endif //APP_ADC_H