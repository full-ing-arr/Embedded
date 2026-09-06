#include "app_adc.h"
#include "app_adc_internal.h"

#include <stdlib.h>

#include "esp_err.h"
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali_scheme.h"

#define ADC_ULP_MODE ADC_ULP_MODE_DISABLE
#define ADC_BITWIDTH ADC_BITWIDTH_12
#define ADC_ATTEN ADC_ATTEN_DB_6

static adc_oneshot_unit_handle_t adc_units[SOC_ADC_PERIPH_NUM];

bool app_adc_oneshot_active(adc_unit_t unit) {
  return adc_units[unit] != NULL;
}

adc_t adc_init(adc_unit_t unit, adc_channel_t channel) {
  if ((unsigned)unit >= SOC_ADC_PERIPH_NUM ||
      (unsigned)channel >= SOC_ADC_CHANNEL_NUM(unit)) {
    ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    abort();
  }

  if (unit == ADC_UNIT_1 && app_adc_monitor_active()) {
    ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);
    abort();
  }

  adc_t adc = {0};

  if (adc_units[unit] == NULL) {
    const adc_oneshot_unit_init_cfg_t init_config = {
      .unit_id = unit,
      .ulp_mode = ADC_ULP_MODE };

    ESP_ERROR_CHECK(
      adc_oneshot_new_unit(&init_config, &adc_units[unit]));
  }

  adc.handle = adc_units[unit];
  
  const adc_oneshot_chan_cfg_t adc_config = {
    .bitwidth = ADC_BITWIDTH,
    .atten = ADC_ATTEN };

  ESP_ERROR_CHECK(
    adc_oneshot_config_channel(adc.handle, channel, &adc_config));
  
  const adc_cali_curve_fitting_config_t cali_config = {
    .unit_id = unit,
    .chan = channel,
    .atten = ADC_ATTEN,
    .bitwidth = ADC_BITWIDTH,
  };

  ESP_ERROR_CHECK(
    adc_cali_create_scheme_curve_fitting(&cali_config, &adc.cali_handle));

  return adc;
}

void adc_read_channel(adc_t *adc, adc_channel_t channel) {
  int raw;

  ESP_ERROR_CHECK(
    adc_oneshot_read(adc->handle, channel, &raw));

  ESP_ERROR_CHECK(
    adc_cali_raw_to_voltage(adc->cali_handle, raw, &adc->milivolts));
}
