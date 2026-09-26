#include "app_task.h"
#include <stdlib.h>

app_task_t app_task_create(TaskFunction_t callback, void *arg) { return (app_task_t){
    .callback = callback,
    .arg = arg,
  };
}

void app_task_run(const app_task_t *task, uint32_t stack_bytes, UBaseType_t priority) {
  if (xTaskCreate(task->callback, "", stack_bytes, task->arg, priority, NULL) != pdPASS)
    abort();
}
