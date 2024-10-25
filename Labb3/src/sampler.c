#include "sampler.h"
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <soc/adc_channel.h>
#include <stdio.h>
#include <esp_timer.h>
#include <rom/ets_sys.h>
#include <driver/timer.h> // Include the header file that defines gptimer_handle_t
#include "driver/gptimer.h"
#include <esp_task_wdt.h>

#define ADC_RESOLUTION 4096  // 12-bit ADC
#define VREF 3300  // 3.3V reference voltage in mV
#define DC_OFFSET 1250  // DC offset in mV
  gptimer_handle_t gptimer = NULL;

adc_oneshot_unit_handle_t adc1_handle = NULL;
int count = 0;
int crossings;
int lastValue;
volatile int val;
volatile int timeNow;
int threshold = 1250;
//bool buttonPressed = false;


void initADC(){




     // ADC1 supports GPIOs 32 - 39, let's choose 32.
    // GPIOs - ADC channels mapping is done with macros ADC1_GPION_CHANNEL, so in our case ADC1_GPIO32_CHANNEL

    // the handle is used to keep a reference to the ADC
    

    // configuration of the ADC
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,            // select the unit
        .ulp_mode = ADC_ULP_MODE_DISABLE, // do not use the ULP
    };
    // we use the "one shot" mode
    esp_err_t result = adc_oneshot_new_unit(&init_config, &adc1_handle);
    ESP_ERROR_CHECK(result);

    // configure the channel, we need to choose the resolution and the attenuation
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12, // use full 12 bits width
        .atten = ADC_ATTEN_DB_11,    // set attenuation to support full scale voltage
    };
    result = adc_oneshot_config_channel(adc1_handle, ADC1_GPIO32_CHANNEL, &config);
    ESP_ERROR_CHECK(result);

    // let's setup a pull-up on that pin
    // this must be called after initialisation of the ADC or it will be ignored
    result = gpio_set_pull_mode(GPIO_NUM_32, GPIO_PULLUP_ONLY);
    ESP_ERROR_CHECK(result);


    threshold = (DC_OFFSET * ADC_RESOLUTION) / VREF;

}

static bool IRAM_ATTR timerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{

 
    // this is called by another function, the actual ISR
    // so most of the typical ISR parts are already done for you
    // you can focus on your logic here
    //printf("Timer ISR\n");
         timeNow = esp_timer_get_time();
        adc_oneshot_read(adc1_handle, ADC1_GPIO32_CHANNEL, &val);
        //esp_rom_printf(">Frequency:%d\n", val);
        count++;
        if (val > threshold && lastValue <= threshold)
        {
            crossings++;
        }else if (val < threshold && lastValue >= threshold)
        {
            crossings++;
        }
        

        lastValue = val;
    return 0;
}

void startSampling(int frequency){

 // confuguration of the timer
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // default clock, 80MHz
        .direction = GPTIMER_COUNT_UP,      // count up at each tick
        .resolution_hz = frequency,          // 2KHz
    };
    // this will contain the reference to the timer instance
  
    // configure the timer
    esp_err_t result = gptimer_new_timer(&timer_config, &gptimer);
    ESP_ERROR_CHECK(result);

    // configuration of the alarm
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 1,//1 // this is like the compare register. If set to 2000 -> 2kHz / 2000 = 1Hz
        .flags = {1},        // sets the first bit, auto-reload, to true
        .reload_count = 0,   // restart from 0 each time the alarm fires
    };
    result = gptimer_set_alarm_action(gptimer, &alarm_config);
    ESP_ERROR_CHECK(result);

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timerISR,
    };
    result = gptimer_register_event_callbacks(gptimer, &cbs, NULL);
    ESP_ERROR_CHECK(result);

    // enable the timer
    result = gptimer_enable(gptimer);
    ESP_ERROR_CHECK(result);

    // start the timer
    result = gptimer_start(gptimer);
    ESP_ERROR_CHECK(result);

}

void stopSampling(){
    // stop the timer
    gptimer_stop(gptimer);
    // disable the timer
    gptimer_disable(gptimer);
    // delete the timer
    gptimer_del_timer(gptimer);
    // delete the handle
    gptimer = NULL;
    // delete the ADC handle
    adc_oneshot_del_unit(adc1_handle);
    adc1_handle = NULL;
    // reset the crossings
    crossings = 0;
    lastValue = 0;
    val = 0;
    timeNow = 0;
    count = 0;
    threshold = 1250;

}

float getFrequency(int frequency){
float endTime = (float)count*1.0/(float)frequency;
printf("Crossing: %d\n", crossings);

float val = (float)crossings/(2.0* endTime);
crossings = 0;
return val;
}

void resetSampling(){
    crossings = 0;
    count = 0;
}
