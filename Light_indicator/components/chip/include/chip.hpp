#ifndef CHIP_HPP
#define CHIP_HPP

#include "chip.h"

template<int Pin>
struct ChipPin {
  static constexpr int pin = Pin;
  static constexpr bool input = false;
  static constexpr bool output = false;
  static constexpr unsigned interrupts = 0;
};

#define CHIP_PIN(Pin, Roles, Interrupts) \
  template<> struct ChipPin<Pin> { \
    static constexpr int pin = Pin; \
    static constexpr bool input = ((Roles) & PIN_IN) != 0; \
    static constexpr bool output = ((Roles) & PIN_OUT) != 0; \
    static constexpr unsigned interrupts = Interrupts; \
  };

#include "gpio.pins"
#undef CHIP_PIN

template<unsigned Mask>
struct GpioTrigger;

#define GPIO_TRIGGER(Name, Trigger) \
  constexpr auto Name() { return Trigger; } \
  template<> struct GpioTrigger<Trigger> { \
    static constexpr auto value = Trigger; \
  };

#include "gpio.triggers"
#undef GPIO_TRIGGER

template<int Pin, auto... Modes>
consteval app_gpio_trigger_t gpio_interrupt_trigger() {
  constexpr unsigned mask = (0u | ... | Modes());
  static_assert((mask & ChipPin<Pin>::interrupts) == mask, "Interrupt mode is not supported on this GPIO pin");
  return GpioTrigger<mask>::value;
}

#define ADC_TRIGGER(Name, Trigger) constexpr app_adc_trigger_t Name() { return Trigger; }
#include "adc.triggers"
#undef ADC_TRIGGER

#define ADC_MODULE(Unit, Bitwidth) APP_ADC_BITS_##Unit = Bitwidth,
enum {
#include "adc.modules"
};
#undef ADC_MODULE

template<int Pin>
struct AdcChannel;

#define ADC_PIN(Pin, Unit, Channel) \
  template<> struct AdcChannel<Pin> { \
    static constexpr int pin = Pin; \
    static constexpr adc_unit_t unit = Unit; \
    static constexpr adc_channel_t channel = Channel; \
    static constexpr adc_bitwidth_t bitwidth = static_cast<adc_bitwidth_t>(APP_ADC_BITS_##Unit); \
  };
#include "adc.pins"
#undef ADC_PIN

#endif // CHIP_HPP
