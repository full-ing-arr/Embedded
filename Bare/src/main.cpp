#include <stdint.h>

extern "C" {
  int ets_printf(const char *format, ...);
  void ets_install_uart_printf(void);
  void ets_delay_us(uint32_t us);
  void uart_tx_switch(uint8_t uart);
  void disable_default_watchdog(void);
  uint32_t ets_get_cpu_frequency(void);

  extern uint32_t _bss_start;
  extern uint32_t _bss_end;
}

static uint32_t cycle_count() {
  uint32_t cycles;
  asm volatile("rsr.ccount %0" : "=a"(cycles));
  return cycles;
}

extern "C" __attribute__((noreturn)) void app_main(void) {
  ets_delay_us(1);

  for (volatile uint32_t *word = &_bss_start; word != &_bss_end; ++word)
    *word = 0;

  disable_default_watchdog();
  uart_tx_switch(0);
  ets_install_uart_printf();

  const uint32_t ticks_per_us = ets_get_cpu_frequency();
  ets_printf("\r\nbare: CPU %u MHz\r\n", ticks_per_us);

  uint32_t previous = cycle_count();
  while (true) {
    const uint32_t now = cycle_count();
    const uint32_t elapsed = now - previous;
    previous = now;
    ets_printf("dt_us=%u\r\n", elapsed / ticks_per_us);
  }
}
