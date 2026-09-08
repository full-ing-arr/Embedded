#ifndef APP_GPIO_HPP
#define APP_GPIO_HPP

extern "C" {
  #include "app_gpio.h"
}

#include "chip.hpp"
#include <tuple>

template<int Pin, typename Callback>
class AppGpioInterrupt {
  static_assert(ChipPin<Pin>::input, "This pin does not support GPIO input");

public:
  AppGpioInterrupt(app_gpio_trigger_t trigger, Callback callback) : trigger_(trigger), callback_(callback) {}

  AppGpioInterrupt(const AppGpioInterrupt &) = delete;
  AppGpioInterrupt &operator=(const AppGpioInterrupt &) = delete;

  ~AppGpioInterrupt() {
    if (attached_)
      stop();
  }

  void run() & {
    if (attached_)
      ::app_gpio_interrupt_enable(Pin);
    else {
      ::app_gpio_interrupt_run(Pin, trigger_, invoke, this);
      attached_ = true;
    }
  }

  template<auto... Modes>
  void change() & {
    constexpr auto trigger = gpio_interrupt_trigger<Pin, Modes...>();
    if (attached_)
      ::app_gpio_interrupt_change(Pin, trigger);
    trigger_ = trigger;
  }

  void stop() & {
    ::app_gpio_interrupt_stop(Pin);
    attached_ = false;
  }

private:
  static void invoke(void *context) {
    static_cast<AppGpioInterrupt *>(context)->callback_();
  }

  app_gpio_trigger_t trigger_;
  bool attached_ = false;
  [[no_unique_address]] Callback callback_;
};

template<int Pin>
struct in : ChipPin<Pin> {
  static_assert(ChipPin<Pin>::input, "This pin does not support GPIO input");

  explicit in(app_gpio_pull_t pull = GPIO_PULL_NONE) { ::app_gpio_input_init(Pin, pull); }

  bool read() const { return ::app_gpio_read(Pin); }

  template<auto Method, auto Mode, auto... Modes, typename... Args>
  auto on_interrupt(Args... args) const {
    auto callback = [args = std::tuple<Args...>(args...)]() mutable { std::apply(Method, args); };
    return AppGpioInterrupt<Pin, decltype(callback)>(gpio_interrupt_trigger<Pin, Mode, Modes...>(), callback);
  }

  template<auto Method, typename... Args>
  auto on_rising(Args... args) const { return on_interrupt<Method, ::on_rising>(args...); }

  template<auto Method, typename... Args>
  auto on_falling(Args... args) const { return on_interrupt<Method, ::on_falling>(args...); }

  template<auto Method, typename... Args>
  auto on_anyedge(Args... args) const { return on_interrupt<Method, ::on_anyedge>(args...); }

  template<auto Method, typename... Args>
  auto on_low(Args... args) const { return on_interrupt<Method, ::on_low>(args...); }

  template<auto Method, typename... Args>
  auto on_high(Args... args) const { return on_interrupt<Method, ::on_high>(args...); }

  void disable_interrupt() const { ::app_gpio_interrupt_disable(Pin); }
};

template<int Pin>
struct out : ChipPin<Pin> {
  static_assert(ChipPin<Pin>::output, "This pin does not support GPIO output");

  explicit out(bool level = false) { ::app_gpio_output_init(Pin, level); }

  void write(bool level) const { ::app_gpio_write(Pin, level); }
};

#endif // APP_GPIO_HPP
