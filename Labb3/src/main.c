#include <stdio.h>
#include "pins.h"
#include "sampler.h"
#include <esp_task_wdt.h>
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include <esp_adc/adc_oneshot.h>
#include <soc/adc_channel.h>
#include <driver/gpio.h>
#include "notes.h"
#include "soundsgen.h"  
#define BUTTON_PIN GPIO_NUM_18
#define TARGET_TOLERANCE 3.0  // Tolerance in Hz for frequency comparison
#define SAMPLE_TIME 1000      // Sampling time in milliseconds
#define PUSH_TIME_US 200000 // 200 ms
static volatile uint64_t lastPush = -PUSH_TIME_US;

// Frequencies of open strings in standard tuning (E2, A2, D3, G3, B3, E4)
float guitarFrequencies[] = {329.63, 246.94, 196.00, 146.83, 110.00, 82.41}; // Starting from high E (thinnest string)

// Global variables
int currentString = 0;  // Track the current string (0 = high E)
bool buttonPressed = false;

void flashStart(){
    setLEDPin(1);
    setLEDPin2(1);
    vTaskDelay(pdMS_TO_TICKS(100));
    setLEDPin(0);
    setLEDPin2(0);
    vTaskDelay(pdMS_TO_TICKS(100));

    setLEDPin(1);
    setLEDPin2(1);
    vTaskDelay(pdMS_TO_TICKS(100));

    setLEDPin(0);
    setLEDPin2(0);
}

// Button interrupt handler

static void buttonPress_handler(void *arg)
{
    // 1: disable interrupt to avoid race conditions
    gpio_intr_disable(BUTTON_PIN);

    // 2: identify the cause of the interrupt
    // this is pointless here because this callback is only called for our pin

    // 3: deal with the cause
    uint64_t now = esp_timer_get_time();
        if ((now - lastPush) > PUSH_TIME_US)
        {
            lastPush = now;
            esp_rom_printf("ISR called!\n");
            buttonPressed = 1;
        }


    // 4: enable interrupts
    gpio_intr_enable(BUTTON_PIN);

    // 5: do anything left do that is safe
    // print a message on the serial. This may not work, it's an internal and unstable API!
    

    // otherwise use:
    // #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
    // #include "esp_log.h"
    // ESP_DRAM_LOGD(DRAM_STR("TEST"), DRAM_STR("ISR called!"));

}

// Function to handle tuning logic for a single string
void tuneString(float targetFreq) {
    //startSampling(6000);  // Start sampling at 6kHz
    printf("Sampling...\n");
    resetSampling();  // Reset frequency detection
    vTaskDelay(pdMS_TO_TICKS(SAMPLE_TIME));  // Sample for 1 second
    float detectedFreq = getFrequency(6000);  // Get detected frequency
    

    char note[5];
    freq2note(detectedFreq, note);
    printf("Detected Frequency: %.2f Hz, Note: %s\n", detectedFreq, note);

    // Check how close the detected frequency is to the target frequency
    float diff = detectedFreq - targetFreq;

    if (diff > TARGET_TOLERANCE) {
        // Detected frequency is higher than the target
        setLEDPin(0);  // Turn off LED A
        setLEDPin2(1); // Turn on LED B (indicating frequency is too high)
    } else if (diff < -TARGET_TOLERANCE) {
        // Detected frequency is lower than the target
        setLEDPin(1);  // Turn on LED A (indicating frequency is too low)
        setLEDPin2(0); // Turn off LED B
    } else {
        // Frequency is within ±TARGET_TOLERANCE of the target
        setLEDPin(1);  // Turn on both LEDs (indicating correct tuning)
        setLEDPin2(1);
    }
}

void app_main() {
    initADC();
    initPins();
    flashStart();  // Flash LEDs to signal start

    // Configure button interrupt
    //gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, buttonPress_handler, NULL);
    startSampling(6000);  // Start sampling at 6kHz
    //vTaskDelay(pdMS_TO_TICKS(SAMPLE_TIME));  // Sample for 1 second

    // Main tuning loop


    while (1) {
       //printf("Before tuning string %d\n", currentString);
        flashStart();
        //tuneString(guitarFrequencies[currentString]);
        //printf("After tuning string %d\n", currentString);
        

            if (buttonPressed == 1)
            {
                resetSampling();
                //startSampling(6000);  // Start sampling at 6kHz
                //vTaskDelay(pdMS_TO_TICKS(SAMPLE_TIME));  // Sample for 1 second
                printf("target freq: %f\n", guitarFrequencies[currentString]);
                tuneString(guitarFrequencies[currentString]);
                buttonPressed = 0;
                currentString++;
                if(currentString >= 6){
                    currentString = 0;
                }
              //  startSampling(6000);  // Start sampling at 6kHz
            
            }
                
            vTaskDelay(pdMS_TO_TICKS(SAMPLE_TIME)); 
         
        
        // Wait for the button press to move to the next string
        //while (!buttonPressed) {
          //  vTaskDelay(pdMS_TO_TICKS(100));  // Poll every 100 ms
        //}
       // buttonPressed = false;  // Reset button flag

        // Move to the next string or stop if all strings are tuned
        //currentString++;
        //if (currentString >= 6) {
          //  printf("All strings tuned. Restarting...\n");
          //  currentString = 0;  // Start over or break the loop to stop
        //}
       // printf("Current string: %d\n", currentString);
    }
    stopSampling();
}
