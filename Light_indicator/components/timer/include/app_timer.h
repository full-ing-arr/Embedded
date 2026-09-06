#ifndef APP_TIMER_H
#define APP_TIMER_H

#include <stdint.h>
#include "esp_timer.h"

typedef struct {
  esp_timer_handle_t handle;
} app_timer_t;

app_timer_t app_timer_create(esp_timer_cb_t callback, void *arg);
void app_timer_run(app_timer_t *timer, uint64_t period_us);
void app_timer_finish(app_timer_t *timer);

#endif // APP_TIMER_H
