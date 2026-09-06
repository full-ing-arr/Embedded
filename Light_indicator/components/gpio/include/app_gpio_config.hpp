#ifndef APP_GPIO_CONFIG_HPP
#define APP_GPIO_CONFIG_HPP

extern "C" {
  #include "app_gpio.h"
}

template<int Pin>
struct GpioPin;

template<unsigned Mask>
struct GpioTrigger;

#define GPIO_TRIGGER(Name, Trigger) \
  constexpr auto Name() { return Trigger; } \
  template<> struct GpioTrigger<Trigger> { \
    static constexpr auto value = Trigger; \
  };

#include "gpio.triggers"
#undef GPIO_TRIGGER

#define GPIO_PIN(Pin, Interrupts) \
  template<> struct GpioPin<Pin> { \
    static constexpr int pin = Pin; \
    static constexpr unsigned interrupts = Interrupts; \
  };

#include "gpio.pins"
#undef GPIO_PIN

template<int Pin, auto... Modes>
consteval app_gpio_trigger_t gpio_interrupt_trigger() {
  constexpr unsigned mask = (0u | ... | Modes());
  static_assert((mask & GpioPin<Pin>::interrupts) == mask,
    "Interrupt mode is not supported on this GPIO pin");
  return GpioTrigger<mask>::value;
}

#endif // APP_GPIO_CONFIG_HPP
