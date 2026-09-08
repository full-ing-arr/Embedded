#ifndef APP_ADC_HPP
#define APP_ADC_HPP

extern "C" {
  #include "app_adc.h"
}

#include "chip.hpp"
#include <optional>
#include <tuple>

template<auto Method, typename... Args>
class AppAdcInterrupt {
public:
  AppAdcInterrupt(adc_channel_t channel, app_adc_trigger_t trigger, int threshold, Args... args) : args_(args...) {
    ::app_adc_interrupt_create(&interrupt_, channel, trigger, threshold, invoke, this);
  }

  AppAdcInterrupt(const AppAdcInterrupt &) = delete;
  AppAdcInterrupt &operator=(const AppAdcInterrupt &) = delete;
  ~AppAdcInterrupt() {
    if (interrupt_.handle != nullptr)
      stop();
  }

  void run() & { ::app_adc_interrupt_run(&interrupt_); }
  void stop() & { ::app_adc_interrupt_stop(&interrupt_); }

  template<auto Mode>
  void change(int threshold) & {
    constexpr app_adc_trigger_t trigger = Mode();
    static_assert(trigger == ADC_ABOVE || trigger == ADC_BELOW, "ADC monitor expects on_above or on_below");
    ::app_adc_interrupt_change(&interrupt_, trigger, threshold);
  }

private:
  static void invoke(void *context) {
    std::apply(Method, static_cast<AppAdcInterrupt *>(context)->args_);
  }

  app_adc_interrupt_t interrupt_;
  [[no_unique_address]] std::tuple<Args...> args_;
};

template<int Pin, adc_atten_t Atten = ADC_ATTEN_DB_6, adc_bitwidth_t Bitwidth = ADC_BITWIDTH_12>
struct adc : private adc_t, public AdcChannel<Pin> {
  using adc_t::raw;

  static_assert(Atten >= ADC_ATTEN_DB_0 && Atten <= ADC_ATTEN_DB_12, "Invalid ADC attenuation");
  static_assert(Bitwidth == AdcChannel<Pin>::bitwidth, "Unsupported ADC bit width for this module");
  static constexpr adc_unit_t unit = AdcChannel<Pin>::unit;
  static constexpr adc_atten_t atten = Atten;
  static constexpr adc_bitwidth_t bitwidth = Bitwidth;

  constexpr adc() : adc_t{} {}
  adc(const adc &) = delete;
  adc &operator=(const adc &) = delete;

  int oneshot() & {
    return ::app_adc_oneshot(this, unit, AdcChannel<Pin>::channel, Atten, Bitwidth);
  }

  std::optional<int> latest() const {
    int result;
    if (::app_adc_latest(this, &result))
      return result;
    return std::nullopt;
  }

  void start_sampling(uint32_t rate_hz) & {
    ::app_adc_start_sampling(this, unit, AdcChannel<Pin>::channel, Atten, Bitwidth, rate_hz);
  }

  void stop_sampling() & { ::app_adc_stop_sampling(AdcChannel<Pin>::channel); }

  template<auto Method, auto Mode, typename... Args>
  auto on_interrupt(int threshold, Args... args) & {
    constexpr app_adc_trigger_t trigger = Mode();
    static_assert(trigger == ADC_ABOVE || trigger == ADC_BELOW, "ADC monitor expects on_above or on_below");
    return AppAdcInterrupt<Method, Args...>(AdcChannel<Pin>::channel, trigger, threshold, args...);
  }

  template<auto Method, typename... Args>
  auto on_below(int threshold, Args... args) & { return on_interrupt<Method, ::on_below>(threshold, args...); }

  template<auto Method, typename... Args>
  auto on_above(int threshold, Args... args) & { return on_interrupt<Method, ::on_above>(threshold, args...); }
};

inline constexpr auto adc_read = []<int Pin, adc_atten_t Atten, adc_bitwidth_t Bitwidth>(adc<Pin, Atten, Bitwidth> *sensor) {
  sensor->oneshot();
};

#endif // APP_ADC_HPP
