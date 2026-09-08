#include "app_adc.h"

#include <stdlib.h>
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_ll.h"
#include "soc/soc_caps.h"

enum {
  ADC_BUFFER_BYTES = 1024,
  ADC_FRAME_BYTES = 256
};

static adc_continuous_handle_t sampler;
static adc_t *sources[SOC_ADC_MAX_CHANNEL_NUM];
static app_adc_interrupt_t *listeners[CHIP_ADC_MONITOR_COUNT];
static unsigned listener_count;
static adc_digi_pattern_config_t sample_pattern;
static uint32_t sample_rate;

static void check(esp_err_t error) {
  if (error == ESP_OK)
    return;
  ESP_ERROR_CHECK(error);
  abort();
}

static bool samples_ready(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *event, void *arg) {
  (void)handle;
  (void)arg;
  for (unsigned i = 0; i + sizeof(adc_digi_output_data_t) <= event->size; i += SOC_ADC_DIGI_RESULT_BYTES) {
    const adc_digi_output_data_t *sample = (const adc_digi_output_data_t *)(event->conv_frame_buffer + i);
    if (sample->type2.unit != sample_pattern.unit || sample->type2.channel >= SOC_ADC_MAX_CHANNEL_NUM)
      continue;
    adc_t *source = sources[sample->type2.channel];
    if (source != NULL)
      __atomic_store_n(&source->sample, sample->type2.data + 1u, __ATOMIC_RELAXED);
  }
  return false;
}

static bool threshold_reached(adc_monitor_handle_t handle, const adc_monitor_evt_data_t *event, void *arg) {
  (void)handle;
  (void)event;
  app_adc_interrupt_t *interrupt = arg;
  interrupt->callback(interrupt->arg);
  return false;
}

static void attach_monitor(app_adc_interrupt_t *interrupt) {
  adc_monitor_config_t config = {
    .adc_unit = sample_pattern.unit,
    .channel = interrupt->channel,
    .h_threshold = -1,
    .l_threshold = -1
  };
  adc_monitor_evt_cbs_t callbacks = {0};
  if (interrupt->trigger == ADC_BELOW) {
    config.l_threshold = interrupt->threshold;
    callbacks.on_below_low_thresh = threshold_reached;
  } else {
    config.h_threshold = interrupt->threshold;
    callbacks.on_over_high_thresh = threshold_reached;
  }
  check(adc_new_continuous_monitor(sampler, &config, &interrupt->handle));
  check(adc_continuous_monitor_register_event_callbacks(interrupt->handle, &callbacks, interrupt));
  check(adc_continuous_monitor_enable(interrupt->handle));
}

static void detach_monitor(app_adc_interrupt_t *interrupt) {
  check(adc_continuous_monitor_disable(interrupt->handle));
  check(adc_del_continuous_monitor(interrupt->handle));
  interrupt->handle = NULL;
}

static void pause_sampler(void) {
  check(adc_continuous_stop(sampler));
  adc_ll_digi_monitor_clear_intr();
}

static void shutdown_sampler(void) {
  pause_sampler();
  for (unsigned i = 0; i < listener_count; ++i)
    detach_monitor(listeners[i]);
  check(adc_continuous_deinit(sampler));
  sampler = NULL;
  // Old DMA callbacks are quiescent before invalidating this sampling generation.
  for (unsigned channel = 0; channel < SOC_ADC_MAX_CHANNEL_NUM; ++channel) {
    if (sources[channel] != NULL)
      __atomic_store_n(&sources[channel]->sample, 0, __ATOMIC_RELAXED);
  }
}

static void start_sampler(void) {
  adc_digi_pattern_config_t patterns[SOC_ADC_MAX_CHANNEL_NUM];
  unsigned count = 0;
  for (unsigned channel = 0; channel < SOC_ADC_MAX_CHANNEL_NUM; ++channel) {
    if (sources[channel] == NULL)
      continue;
    patterns[count] = sample_pattern;
    patterns[count++].channel = channel;
  }
  if (count == 0)
    return;

  const adc_continuous_handle_cfg_t handle_config = {
    .max_store_buf_size = ADC_BUFFER_BYTES,
    .conv_frame_size = ADC_FRAME_BYTES,
    .flags.flush_pool = true
  };
  const adc_continuous_evt_cbs_t callbacks = {
    .on_conv_done = samples_ready
  };
  const adc_continuous_config_t config = {
    .pattern_num = count,
    .adc_pattern = patterns,
    .sample_freq_hz = sample_rate,
    .conv_mode = sample_pattern.unit == ADC_UNIT_1 ? ADC_CONV_SINGLE_UNIT_1 : ADC_CONV_SINGLE_UNIT_2
  };
  check(adc_continuous_new_handle(&handle_config, &sampler));
  check(adc_continuous_register_event_callbacks(sampler, &callbacks, NULL));
  for (unsigned i = 0; i < listener_count; ++i)
    attach_monitor(listeners[i]);
  check(adc_continuous_config(sampler, &config));
  check(adc_continuous_start(sampler));
}

adc_t adc_init(void) {
  return (adc_t) {0};
}

bool app_adc_latest(const adc_t *source, int *raw) {
  const unsigned sample = __atomic_load_n(&source->sample, __ATOMIC_RELAXED);
  if (sample == 0)
    return false;
  *raw = (int)sample - 1;
  return true;
}

int app_adc_oneshot(adc_t *source, adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bitwidth_t bitwidth) {
  adc_oneshot_unit_handle_t handle;
  const adc_oneshot_unit_init_cfg_t unit_config = {
    .unit_id = unit
  };
  const adc_oneshot_chan_cfg_t channel_config = {
    .atten = atten,
    .bitwidth = bitwidth
  };
  int raw;
  check(adc_oneshot_new_unit(&unit_config, &handle));
  check(adc_oneshot_config_channel(handle, channel, &channel_config));
  check(adc_oneshot_read(handle, channel, &raw));
  check(adc_oneshot_del_unit(handle));
  source->raw = raw;
  return raw;
}

void app_adc_start_sampling(adc_t *source, adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bitwidth_t bitwidth, uint32_t rate_hz) {
  if (sampler != NULL)
    shutdown_sampler();
  sources[channel] = source;
  __atomic_store_n(&source->sample, 0, __ATOMIC_RELAXED);
  sample_pattern = (adc_digi_pattern_config_t) {
    .atten = atten,
    .unit = unit,
    .bit_width = bitwidth
  };
  sample_rate = rate_hz;
  start_sampler();
}

void app_adc_stop_sampling(adc_channel_t channel) {
  shutdown_sampler();
  sources[channel] = NULL;
  start_sampler();
}

void app_adc_interrupt_create(app_adc_interrupt_t *interrupt, adc_channel_t channel, app_adc_trigger_t trigger, int threshold, app_adc_interrupt_cb_t callback, void *arg) {
  *interrupt = (app_adc_interrupt_t) {
    .channel = channel,
    .callback = callback,
    .arg = arg,
    .trigger = trigger,
    .threshold = (int16_t)threshold
  };
}

void app_adc_interrupt_run(app_adc_interrupt_t *interrupt) {
  pause_sampler();
  attach_monitor(interrupt);
  interrupt->slot = listener_count;
  listeners[listener_count++] = interrupt;
  check(adc_continuous_start(sampler));
}

void app_adc_interrupt_change(app_adc_interrupt_t *interrupt, app_adc_trigger_t trigger, int threshold) {
  interrupt->trigger = trigger;
  interrupt->threshold = (int16_t)threshold;
  if (interrupt->handle != NULL) {
    pause_sampler();
    detach_monitor(interrupt);
    attach_monitor(interrupt);
    check(adc_continuous_start(sampler));
  }
}

void app_adc_interrupt_stop(app_adc_interrupt_t *interrupt) {
  pause_sampler();
  detach_monitor(interrupt);
  listeners[interrupt->slot] = listeners[--listener_count];
  listeners[interrupt->slot]->slot = interrupt->slot;
  check(adc_continuous_start(sampler));
}
