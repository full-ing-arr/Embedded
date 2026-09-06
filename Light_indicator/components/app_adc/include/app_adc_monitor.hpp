#ifndef APP_ADC_MONITOR_HPP
#define APP_ADC_MONITOR_HPP

#include "app_adc.hpp"

extern "C" {
  #include "app_adc_interrupt.h"
}

constexpr app_adc_trigger_t on_above() { return ADC_ABOVE; }
constexpr app_adc_trigger_t on_below() { return ADC_BELOW; }

template<int Pin>
class AppAdcInterrupt {
 public:
  AppAdcInterrupt(app_adc_trigger_t trigger, int threshold,
    app_adc_interrupt_cb_t callback, void *arg = nullptr)
    : trigger_(trigger), threshold_(threshold), callback_(callback), arg_(arg) {}

  AppAdcInterrupt(const AppAdcInterrupt &) = delete;
  AppAdcInterrupt &operator=(const AppAdcInterrupt &) = delete;
  AppAdcInterrupt(AppAdcInterrupt &&) = delete;
  AppAdcInterrupt &operator=(AppAdcInterrupt &&) = delete;

  ~AppAdcInterrupt() { stop(); }

  void run() & {
    if (handle_ != nullptr) return;
    handle_ = ::app_adc_interrupt_run(AdcPin<Pin>::unit, AdcPin<Pin>::channel,
      trigger_, threshold_, callback_, arg_);
  }

  template<auto Mode>
  void change(int threshold) & {
    constexpr auto trigger = Mode();
    static_assert(trigger == ADC_ABOVE || trigger == ADC_BELOW,
      "ADC monitor expects on_above or on_below");
    if (handle_ != nullptr) {
      ::app_adc_interrupt_change(handle_, trigger, threshold);
    }
    trigger_ = trigger;
    threshold_ = threshold;
  }

  void stop() & {
    if (handle_ == nullptr) return;
    ::app_adc_interrupt_stop(handle_);
    handle_ = nullptr;
  }

 private:
  app_adc_trigger_t trigger_;
  int threshold_;
  app_adc_interrupt_cb_t callback_;
  void *arg_;
  app_adc_interrupt_handle_t handle_ = nullptr;
};

template<int Pin>
struct adc_monitor : AdcPin<Pin> {
  static_assert(AdcPin<Pin>::unit == ADC_UNIT_1,
    "ESP32-S3 ADC monitor uses ADC1; ADC2 continuous DMA is unsupported");
  static_assert(AdcPin<Pin>::channel <= ADC_CHANNEL_7,
    "ESP-IDF 6.0.1 monitor HAL misencodes ADC channels 8/9; use GPIO1..8");

  template<auto Method>
  auto on_above(int threshold) const {
    return AppAdcInterrupt<Pin>(ADC_ABOVE, threshold, [](void *) { Method(); });
  }

  template<auto Method, typename T>
  auto on_above(int threshold, T *context) const {
    return AppAdcInterrupt<Pin>(ADC_ABOVE, threshold,
      [](void *arg) { Method(static_cast<T *>(arg)); },
      const_cast<void *>(static_cast<const volatile void *>(context)));
  }

  template<auto Method>
  auto on_below(int threshold) const {
    return AppAdcInterrupt<Pin>(ADC_BELOW, threshold, [](void *) { Method(); });
  }

  template<auto Method, typename T>
  auto on_below(int threshold, T *context) const {
    return AppAdcInterrupt<Pin>(ADC_BELOW, threshold,
      [](void *arg) { Method(static_cast<T *>(arg)); },
      const_cast<void *>(static_cast<const volatile void *>(context)));
  }
};

#endif // APP_ADC_MONITOR_HPP
