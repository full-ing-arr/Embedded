#include "app_adc.hpp"
#include "app_gpio.hpp"
#include "app_task.hpp"
#include "esp_log.h"
#include "esp_attr.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>

typedef in<20> input_pin_t;
typedef out<8> led_pin_t;
typedef out<36> light_pin_t;
typedef adc<4> sensor_adc_t;

constexpr int light_on_threshold = CONFIG_LIGHT_ON_THRESHOLD;
constexpr int light_off_threshold = CONFIG_LIGHT_OFF_THRESHOLD;

void light_on(light_pin_t *light) { 
  light->write(true); 
}

void light_off(light_pin_t *light) { 
  light->write(false); 
}

void interrupt_callback(led_pin_t out, input_pin_t in) {
  ESP_DRAM_LOGI(DRAM_STR("GPIO"), "Interrupt triggered");
  out.write(!in.read());
}

void adc_sensor_read_task(sensor_adc_t *sensor) {
  while (true) {
    if (auto raw = sensor->latest()) 
      printf(">ADC:%d\n", *raw);
    
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

extern "C" void app_main(void) {
  static sensor_adc_t sensor;
  static input_pin_t input(GPIO_PULL_UP);
  static light_pin_t light;
  static led_pin_t led;

  static auto adc_low = sensor.on_below<light_on>(light_on_threshold, &light);
  static auto adc_high = sensor.on_above<light_off>(light_off_threshold, &light);
  static auto interrupt = input.on_anyedge<interrupt_callback>(led, input);
  static auto reader = app_task_create<adc_sensor_read_task>(&sensor);

  sensor.start_sampling(1000);
  adc_low.run();
  adc_high.run();
  interrupt.run();
  reader.run(2048, 2);
}
