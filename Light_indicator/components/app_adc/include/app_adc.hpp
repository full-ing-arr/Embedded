#ifndef APP_ADC_HPP
#define APP_ADC_HPP

extern "C" {
  #include "app_adc.h"
}

template<int Pin>
struct AdcPin;

#define ADC_PIN(Pin, Unit, Channel) \
  template<> struct AdcPin<Pin> { \
    static constexpr adc_unit_t unit = Unit; \
    static constexpr adc_channel_t channel = Channel; \
  };

#include "adc.pins"
#undef ADC_PIN

template<int Pin>
struct adc : adc_t, AdcPin<Pin> {
  adc() : adc_t(::adc_init(AdcPin<Pin>::unit, AdcPin<Pin>::channel)) {}
};

inline constexpr auto adc_read = []<int Pin>(adc<Pin> *adc) {
  ::adc_read_channel(adc, AdcPin<Pin>::channel);
};

#endif // APP_ADC_HPP
