#include "app_timer.h"
#include <stddef.h>

app_timer_t app_timer_create(esp_timer_cb_t callback, void *arg) {
  app_timer_t timer = {0};

  const esp_timer_create_args_t timer_args = {
    .callback = callback,
    .arg = arg,
    .dispatch_method = ESP_TIMER_TASK,
  };

  ESP_ERROR_CHECK(
    esp_timer_create(&timer_args, &timer.handle));

  return timer;
}

void app_timer_run(app_timer_t *timer, uint64_t period_us) {
  ESP_ERROR_CHECK(
    esp_timer_start_periodic(timer->handle, period_us));
}

void app_timer_finish(app_timer_t *timer) {
  ESP_ERROR_CHECK(
    esp_timer_stop(timer->handle));

  ESP_ERROR_CHECK(
    esp_timer_delete(timer->handle));

  timer->handle = NULL;
}
