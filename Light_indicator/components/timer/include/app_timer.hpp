#ifndef APP_TIMER_HPP
#define APP_TIMER_HPP

extern "C" {
  #include "app_timer.h"
}

template<typename Callback>
class AppTimer {
public:
  AppTimer(Callback callback)
    : callback_(callback),
      timer_(::app_timer_create(invoke, this)) {}

  AppTimer(const AppTimer &) = delete;
  AppTimer &operator=(const AppTimer &) = delete;

  void run(uint64_t period_us) {
    ::app_timer_run(&timer_, period_us);
  }

  void finish() & {
    ::app_timer_finish(&timer_);
    timer_.handle = nullptr;
  }

private:
  static void invoke(void *context) {
    static_cast<AppTimer *>(context)->callback_();
  }

  Callback callback_;
  app_timer_t timer_;
};

template<typename Callback>
auto app_timer_create(Callback callback) {
  return AppTimer<Callback>(callback);
}

template<auto Method, typename... Args>
auto app_timer_create(Args... args) {
  return app_timer_create([args...]() mutable {
    Method(args...);
  });
}

#endif // APP_TIMER_HPP
