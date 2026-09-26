#ifndef APP_TASK_H
#define APP_TASK_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
  TaskFunction_t callback;
  void *arg;
} app_task_t;

app_task_t app_task_create(TaskFunction_t callback, void *arg);
void app_task_run(const app_task_t *task, uint32_t stack_bytes, UBaseType_t priority);

#endif // APP_TASK_H
