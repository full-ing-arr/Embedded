#ifndef APP_ADC_INTERNAL_H
#define APP_ADC_INTERNAL_H

#include <stdbool.h>
#include "hal/adc_types.h"

bool app_adc_oneshot_active(adc_unit_t unit);
bool app_adc_monitor_active(void);

#endif // APP_ADC_INTERNAL_H
