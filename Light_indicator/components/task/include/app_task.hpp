#ifndef APP_TASK_HPP
#define APP_TASK_HPP

extern "C" {
  #include "app_task.h"
}

template<typename Callback>
class AppTask {
public:
  AppTask(Callback callback) : callback_(callback) {}

  AppTask(const AppTask &) = delete;
  AppTask &operator=(const AppTask &) = delete;

  void run(uint32_t stack_bytes, UBaseType_t priority) & {
    const app_task_t task = ::app_task_create(invoke, this);
    ::app_task_run(&task, stack_bytes, priority);
  }

private:
  static void invoke(void *context) {
    static_cast<AppTask *>(context)->callback_();
    vTaskDelete(nullptr);
  }

  Callback callback_;
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
