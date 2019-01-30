#include "soc/i2s_reg.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

class AudioOutput;
void IRAM_ATTR timerInterrupt(AudioOutput *audioOutput);

class AudioOutput
{
  public:
  AudioSystem *audioSystem;
  
  void init(AudioSystem &audioSystem)
  {
    this->audioSystem = &audioSystem;
    timer_config_t config;
    config.alarm_en = 1;
    config.auto_reload = 1;
    config.counter_dir = TIMER_COUNT_UP;
    config.divider = 16;
    config.intr_type = TIMER_INTR_LEVEL;
    config.counter_en = TIMER_PAUSE;
    timer_init((timer_group_t)TIMER_GROUP_0, (timer_idx_t)TIMER_0, &config);
    timer_pause((timer_group_t)TIMER_GROUP_0, (timer_idx_t)TIMER_0);
    timer_set_counter_value((timer_group_t)TIMER_GROUP_0, (timer_idx_t)TIMER_0, 0x00000000ULL);
    timer_set_alarm_value((timer_group_t)TIMER_GROUP_0, (timer_idx_t)TIMER_0, 1.0/audioSystem.samplingRate * TIMER_BASE_CLK / config.divider);
    timer_enable_intr((timer_group_t)TIMER_GROUP_0, (timer_idx_t)TIMER_0);
    timer_isr_register((timer_group_t)TIMER_GROUP_0, (timer_idx_t)TIMER_0, (void (*)(void*))timerInterrupt, (void*) this, ESP_INTR_FLAG_IRAM, NULL);
    timer_start((timer_group_t)TIMER_GROUP_0, (timer_idx_t)TIMER_0);
  }
};

void IRAM_ATTR timerInterrupt(AudioOutput *audioOutput)
{
  uint32_t intStatus = TIMERG0.int_st_timers.val;
  if(intStatus & BIT(TIMER_0)) 
  {
      TIMERG0.hw_timer[TIMER_0].update = 1;
      TIMERG0.int_clr_timers.t0 = 1;
      TIMERG0.hw_timer[TIMER_0].config.alarm_en = 1;
      
      WRITE_PERI_REG(I2S_CONF_SIGLE_DATA_REG(0), audioOutput->audioSystem->nextSample() << 24);
  }
}  

