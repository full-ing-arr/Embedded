#ifndef APP_GPIO_HPP
#define APP_GPIO_HPP

#include "app_gpio_config.hpp"

template<int Pin>
class AppGpioInterrupt : GpioPin<Pin> {
public:
  AppGpioInterrupt(app_gpio_trigger_t trigger,
    app_gpio_interrupt_cb_t callback, void *arg)
    : callback_(callback), arg_(arg), trigger_(trigger) {}

  AppGpioInterrupt(const AppGpioInterrupt &) = delete;
  AppGpioInterrupt &operator=(const AppGpioInterrupt &) = delete;

  ~AppGpioInterrupt() {
    stop();
  }

  void run() & {
    if (attached_) {
      ::app_gpio_interrupt_enable(GpioPin<Pin>::pin);
      return;
    }
    ::app_gpio_interrupt_run(GpioPin<Pin>::pin, trigger_, callback_, arg_);
    attached_ = true;
  }

  template<auto... Modes>
  void change() & {
    constexpr auto trigger = gpio_interrupt_trigger<Pin, Modes...>();
    if (attached_) {
      ::app_gpio_interrupt_change(GpioPin<Pin>::pin, trigger);
    }
    trigger_ = trigger;
  }

  void stop() & {
    if (!attached_) return;
    ::app_gpio_interrupt_stop(GpioPin<Pin>::pin);
    attached_ = false;
  }

private:
  app_gpio_interrupt_cb_t callback_;
  void *arg_;
  app_gpio_trigger_t trigger_;
  bool attached_ = false;
};

template<int Pin>
struct in : GpioPin<Pin> {
  explicit in(app_gpio_pull_t pull = GPIO_PULL_NONE) {
    ::app_gpio_input_init(GpioPin<Pin>::pin, pull);
  }

  bool read() const {
    return ::app_gpio_read(GpioPin<Pin>::pin);
  }

  template<auto Method, auto Mode, auto... Modes>
  auto on_interrupt() const {
    return AppGpioInterrupt<Pin>(gpio_interrupt_trigger<Pin, Mode, Modes...>(),
      [](void *) { Method(); }, nullptr);
  }

  template<auto Method, auto Mode, auto... Modes, typename T>
  auto on_interrupt(T *context) const {
    return AppGpioInterrupt<Pin>(gpio_interrupt_trigger<Pin, Mode, Modes...>(),
      [](void *arg) { Method(static_cast<T *>(arg)); },
      const_cast<void *>(static_cast<const volatile void *>(context)));
  }

  template<auto Method, typename... Args>
  auto on_rising(Args... args) const {
    return on_interrupt<Method, ::on_rising>(args...);
  }

  template<auto Method, typename... Args>
  auto on_falling(Args... args) const {
    return on_interrupt<Method, ::on_falling>(args...);
  }

  template<auto Method, typename... Args>
  auto on_anyedge(Args... args) const {
    return on_interrupt<Method, ::on_anyedge>(args...);
  }

  template<auto Method, typename... Args>
  auto on_low(Args... args) const {
    return on_interrupt<Method, ::on_low>(args...);
  }

  template<auto Method, typename... Args>
  auto on_high(Args... args) const {
    return on_interrupt<Method, ::on_high>(args...);
  }

  void disable_interrupt() const {
    ::app_gpio_interrupt_disable(GpioPin<Pin>::pin);
  }
};

template<int Pin>
struct out : GpioPin<Pin> {
  explicit out(bool level = false) {
    ::app_gpio_output_init(GpioPin<Pin>::pin, level);
  }

  void write(bool level) const {
    ::app_gpio_write(GpioPin<Pin>::pin, level);
  }
};

#endif // APP_GPIO_HPP
