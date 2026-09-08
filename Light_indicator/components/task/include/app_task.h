#ifndef APP_TASK_H
#define APP_TASK_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
  TaskFunction_t callback;
  void *arg;
} app_task_t;

// Prepare only. C callbacks must not return, as required by FreeRTOS.
app_task_t app_task_create(TaskFunction_t callback, void *arg);

// Call once per object; repeated calls create additional tasks. Stack size is in bytes.
// Valid callback, stack size and priority are caller preconditions; task creation failure aborts.
void app_task_run(const app_task_t *task, uint32_t stack_bytes, UBaseType_t priority);

#endif // APP_TASK_H
