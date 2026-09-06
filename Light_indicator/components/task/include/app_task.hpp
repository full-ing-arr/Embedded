#ifndef APP_TASK_HPP
#define APP_TASK_HPP

extern "C" {
#include "app_task.h"
}

// Keep the object and captured objects alive until the callback finishes.
template<typename Callback>
class AppTask {
public:
  AppTask(Callback callback)
    : callback_(callback),
      task_(::app_task_create(invoke, this)) {}

  AppTask(const AppTask &) = delete;
  AppTask &operator=(const AppTask &) = delete;

  BaseType_t run(uint32_t stack_bytes, UBaseType_t priority) & {
    return ::app_task_run(&task_, stack_bytes, priority);
  }

private:
  static void invoke(void *context) {
    static_cast<AppTask *>(context)->callback_();
    vTaskDelete(nullptr);
  }

  Callback callback_;
  app_task_t task_;
};

template<typename Callback>
auto app_task_create(Callback callback) {
  return AppTask<Callback>(callback);
}

template<auto Method, typename... Args>
auto app_task_create(Args... args) {
  return app_task_create([args...]() mutable {
    Method(args...);
  });
}

#endif // APP_TASK_HPP
