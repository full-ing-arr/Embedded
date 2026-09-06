#ifndef APP_TASK_H
#define APP_TASK_H

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
  TaskFunction_t callback;
  void *arg;
  bool started;
} app_task_t;

// Prepare only. C callbacks must not return, as required by FreeRTOS.
app_task_t app_task_create(TaskFunction_t callback, void *arg);

// Call from one owner. Stack size is in bytes. Retry is allowed after failure;
// a successful start cannot be repeated, even after the task finishes.
BaseType_t app_task_run(app_task_t *task, uint32_t stack_bytes, UBaseType_t priority);

#endif // APP_TASK_H
