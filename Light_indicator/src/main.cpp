#include "app_adc.hpp"
#include "app_gpio.hpp"
#include "esp_log.h"
#include "esp_attr.h"
#include "app_timer.hpp"
#include "app_task.hpp"

typedef in<20> input_pin_t;
typedef out<36> light_pin_t;
typedef adc<4> sensor_adc_t;

void adc_read_task(adc_t *adc) {
  do {
    ESP_LOGI(">ADC", "ADC Value:%d", adc->milivolts);
    vTaskDelay(1000);
  } while (1);
}

void interrupt_callback(light_pin_t light, input_pin_t input) {
  ESP_DRAM_LOGI(DRAM_STR("GPIO"), "Interrupt triggered");
  light.write(!input.read());
}

typedef struct {
  int light_on;
  int light_off;
} light_config_t;

extern "C" void app_main(void) {
  static sensor_adc_t adc;
  static input_pin_t input(GPIO_PULL_UP);
  static light_pin_t light;

  static auto adc_timer = app_timer_create<adc_read>(&adc);
  static auto adc_task = app_task_create<adc_read_task>(&adc);
  static auto interrupt = input.on_anyedge<[] {
    interrupt_callback(light, input);
  }>();

  adc_timer.run(10000);
  adc_task.run(2048, 2);
  interrupt.run();

  while (1) {
    vTaskDelay(1000);
  }
}
