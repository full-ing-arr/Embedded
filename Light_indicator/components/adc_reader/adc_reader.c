#include "adc_reader.h"

#include "freertos/FreeRTOS.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#define ADC_UNIT ADC_UNIT_1
#define ADC_CHANNEL ADC_CHANNEL_3
#define ADC_ULP_MODE ADC_ULP_MODE_DISABLE
#define ADC_BITWIDTH ADC_BITWIDTH_DEFAULT
#define ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_SAMPLE_RATE 100

#define TAG "ADC"

static adc_oneshot_unit_handle_t adc_handle;

static void adc_init() {
  adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT,
    .ulp_mode = ADC_ULP_MODE};
  
  ESP_ERROR_CHECK(
    adc_oneshot_new_unit(&init_config1, &adc_handle));
  
  adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH,
    .atten = ADC_ATTEN};

  ESP_ERROR_CHECK(
    adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &config));
}

void adc_task() {
  adc_init();

  static int out_raw;

  while(1) {
    ESP_ERROR_CHECK(
      adc_oneshot_read(adc_handle, ADC_CHANNEL, &out_raw));

    float voltage = (float)out_raw / 4095 * 3.3;
    ESP_LOGI(TAG, "%.3fV", voltage);

    vTaskDelay(pdMS_TO_TICKS(ADC_SAMPLE_RATE));
  }
}
