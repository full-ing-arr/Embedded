#include "app_task.h"

app_task_t app_task_create(TaskFunction_t callback, void *arg) {
  app_task_t task = {
    .callback = callback,
    .arg = arg,
  };

  return task;
}

BaseType_t app_task_run(app_task_t *task, uint32_t stack_bytes, UBaseType_t priority) {
  if (task == NULL || task->callback == NULL || task->started ||
      stack_bytes == 0 || priority >= configMAX_PRIORITIES)
    return pdFAIL;

  task->started = true;

  BaseType_t result = xTaskCreate(
    task->callback, "", stack_bytes, task->arg, priority, NULL);

  if (result != pdPASS)
    task->started = false;

  return result;
}
