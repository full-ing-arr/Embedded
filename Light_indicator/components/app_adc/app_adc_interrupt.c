#include "app_adc_interrupt.h"
#include "app_adc_internal.h"

#include <stdlib.h>

#include "esp_err.h"
#include "esp_adc/adc_monitor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/adc_ll.h"
#include "soc/soc_caps.h"

#define ADC_SAMPLE_HZ 1000

struct app_adc_interrupt {
  adc_monitor_handle_t handle;
  adc_channel_t channel;
  app_adc_interrupt_cb_t callback;
  void *arg;
};

static adc_continuous_handle_t adc;
static struct app_adc_interrupt interrupts[SOC_ADC_DIGI_MONITOR_NUM];
static bool running;

static _Noreturn void fail(esp_err_t error) {
  ESP_ERROR_CHECK(error);
  abort();
}

static void check(esp_err_t error) {
  if (error != ESP_OK) fail(error);
}

static void prepare(void) {
  if (xPortInIsrContext() ||
      xTaskGetSchedulerState() != taskSCHEDULER_RUNNING ||
      xTaskGetCoreID(NULL) != 0) {
    fail(ESP_ERR_INVALID_STATE);
  }
}

static void check_threshold(app_adc_trigger_t trigger, int threshold) {
  if ((trigger != ADC_ABOVE && trigger != ADC_BELOW) ||
      (unsigned)threshold > 4095) {
    fail(ESP_ERR_INVALID_ARG);
  }
}

static void check_interrupt(app_adc_interrupt_handle_t interrupt) {
  for (int i = 0; i < SOC_ADC_DIGI_MONITOR_NUM; ++i) {
    if (interrupt == &interrupts[i] && interrupts[i].handle != NULL) return;
  }
  fail(ESP_ERR_INVALID_ARG);
}

bool app_adc_monitor_active(void) {
  return adc != NULL;
}

static bool invoke(adc_monitor_handle_t handle,
  const adc_monitor_evt_data_t *event, void *arg) {
  (void)handle;
  (void)event;
  struct app_adc_interrupt *interrupt = arg;
  interrupt->callback(interrupt->arg);
  return false;
}

static void pause(void) {
  if (!running) return;
  check(adc_continuous_stop(adc));
  // SDK deletion leaves pending threshold IRQs; clear them before freeing slots.
  adc_ll_digi_monitor_clear_intr();
  running = false;
}

static void resume(void) {
  adc_digi_pattern_config_t patterns[SOC_ADC_DIGI_MONITOR_NUM];
  unsigned count = 0;
  unsigned channels = 0;

  for (int i = 0; i < SOC_ADC_DIGI_MONITOR_NUM; ++i) {
    if (interrupts[i].handle == NULL) continue;
    const adc_channel_t channel = interrupts[i].channel;
    if (channels & (1U << channel)) continue;
    channels |= 1U << channel;
    patterns[count++] = (adc_digi_pattern_config_t) {
      .atten = ADC_ATTEN_DB_6,
      .channel = channel,
      .unit = ADC_UNIT_1,
      .bit_width = ADC_BITWIDTH_12,
    };
  }

  if (count == 0) {
    check(adc_continuous_deinit(adc));
    adc = NULL;
    return;
  }

  const adc_continuous_config_t config = {
    .pattern_num = count,
    .adc_pattern = patterns,
    .sample_freq_hz = ADC_SAMPLE_HZ,
    .conv_mode = ADC_CONV_SINGLE_UNIT_1,
  };
  check(adc_continuous_config(adc, &config));
  check(adc_continuous_start(adc));
  running = true;
}

static void attach(app_adc_interrupt_handle_t interrupt,
  app_adc_trigger_t trigger, int threshold) {
  const adc_monitor_config_t config = {
    .adc_unit = ADC_UNIT_1,
    .channel = interrupt->channel,
    .h_threshold = trigger == ADC_ABOVE ? threshold : -1,
    .l_threshold = trigger == ADC_BELOW ? threshold : -1,
  };
  const adc_monitor_evt_cbs_t callbacks = {
    .on_over_high_thresh = trigger == ADC_ABOVE ? invoke : NULL,
    .on_below_low_thresh = trigger == ADC_BELOW ? invoke : NULL,
  };
  check(adc_new_continuous_monitor(adc, &config, &interrupt->handle));
  check(adc_continuous_monitor_register_event_callbacks(
    interrupt->handle, &callbacks, interrupt));
  check(adc_continuous_monitor_enable(interrupt->handle));
}

static void detach(app_adc_interrupt_handle_t interrupt) {
  check(adc_continuous_monitor_disable(interrupt->handle));
  check(adc_del_continuous_monitor(interrupt->handle));
  interrupt->handle = NULL;
}

app_adc_interrupt_handle_t app_adc_interrupt_run(adc_unit_t unit,
  adc_channel_t channel, app_adc_trigger_t trigger, int threshold,
  app_adc_interrupt_cb_t callback, void *arg) {
  prepare();
  check_threshold(trigger, threshold);
  // ESP-IDF 6.0.1 S3 monitor HAL truncates channels 8/9 to 0/1.
  if (unit != ADC_UNIT_1 || (unsigned)channel > ADC_CHANNEL_7) {
    fail(ESP_ERR_NOT_SUPPORTED);
  }
  if (callback == NULL) fail(ESP_ERR_INVALID_ARG);
  if (app_adc_oneshot_active(unit)) fail(ESP_ERR_INVALID_STATE);

  app_adc_interrupt_handle_t interrupt = NULL;
  for (int i = 0; i < SOC_ADC_DIGI_MONITOR_NUM; ++i) {
    if (interrupts[i].handle == NULL) {
      interrupt = &interrupts[i];
      break;
    }
  }
  if (interrupt == NULL) fail(ESP_ERR_NOT_FOUND);

  if (adc == NULL) {
    const adc_continuous_handle_cfg_t config = {
      .max_store_buf_size = 1024,
      .conv_frame_size = 256,
      .flags.flush_pool = true,
    };
    check(adc_continuous_new_handle(&config, &adc));
  }

  pause();
  interrupt->channel = channel;
  interrupt->callback = callback;
  interrupt->arg = arg;
  attach(interrupt, trigger, threshold);
  resume();
  return interrupt;
}

void app_adc_interrupt_change(app_adc_interrupt_handle_t interrupt,
  app_adc_trigger_t trigger, int threshold) {
  prepare();
  check_threshold(trigger, threshold);
  check_interrupt(interrupt);
  pause();
  detach(interrupt);
  attach(interrupt, trigger, threshold);
  resume();
}

void app_adc_interrupt_stop(app_adc_interrupt_handle_t interrupt) {
  if (interrupt == NULL) return;
  prepare();
  check_interrupt(interrupt);
  pause();
  detach(interrupt);
  resume();
}
