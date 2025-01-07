#include <driver/i2c.h>
#include <driver/adc.h>
#include <driver/gpio.h>
#include <rom/ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "HD44780.h"

#define rpmPotGPIO ADC1_CHANNEL_5
#define CKP_GPIO GPIO_NUM_25
#define CMP_GPIO GPIO_NUM_26

//Custom Settings
int minRPM = 600;
int maxRPM = 10000;
int delayToUpdateRPM = 300;
int totalTeeth = 60;
int totalMissingTeeth = 2;
int cmpTeeth[] = {14, 19};
int cmpCount = sizeof(cmpTeeth) / sizeof(cmpTeeth[0]);

int rpm = 600;
char displayMessage[16];
int currentTooth = 0;

long map(long x, long in_min, long in_max, long out_min, long out_max);

void updateRPM (void* pvParameter);
void displayRPM (void* pvParameter);
void generateSignal (void* pvParameter);

void app_main() {
    //Init GPIO of RPM potentiometer
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(rpmPotGPIO,ADC_ATTEN_DB_12);

    //Init GPIO of output signal
    gpio_set_direction(CKP_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(CMP_GPIO, GPIO_MODE_OUTPUT);
    
    //Init LCD display
    LCD_init(0x27, 21, 22, 16, 2);
    LCD_clearScreen();

    //Start RPM updater
    xTaskCreatePinnedToCore (
        updateRPM,     // Function to implement the task
        "updateRPM",   // Name of the task
        1024,      // Stack size in bytes
        NULL,      // Task input parameter
        5,         // Priority of the task
        NULL,      // Task handle.
        1          // Core where the task should run
    );

    //Start Display
    xTaskCreatePinnedToCore (
        displayRPM,     // Function to implement the task
        "displayRPM",   // Name of the task
        2048,      // Stack size in bytes
        NULL,      // Task input parameter
        5,         // Priority of the task
        NULL,      // Task handle.
        1          // Core where the task should run
    );
    //Start Signal
    xTaskCreatePinnedToCore (
        generateSignal,     // Function to implement the task
        "generateSignal",   // Name of the task
        2048,      // Stack size in bytes
        NULL,      // Task input parameter
        5,         // Priority of the task
        NULL,      // Task handle.
        0          // Core where the task should run
    );    

}

void updateRPM (void* pvParameter) {
    while (true) {
        int rpmPotValue = adc1_get_raw(rpmPotGPIO);
        rpm = map(rpmPotValue, 0, 4095, minRPM, maxRPM);
        vTaskDelay(pdMS_TO_TICKS(delayToUpdateRPM));
    }
}

void displayRPM (void* pvParameter) {
    while (true) {
        LCD_home();
        snprintf(displayMessage, sizeof(displayMessage), "RPM: %d       ", rpm);
        LCD_writeStr(displayMessage);
        vTaskDelay(pdMS_TO_TICKS(delayToUpdateRPM));
    }
}

void generateSignal (void* pvParameter) {
    while (true) {
        int period = 60000000 / (rpm * 60);           // Calculates the period of one tooth in µs
        int realTeeth = totalTeeth - totalMissingTeeth;
        int cmpState = 0;

        for (int ckpTooth = 0; ckpTooth < totalTeeth; ckpTooth++) {
            currentTooth++;

            // Checks if the currentTooth is the cmp and change its state
            for (int i = 0; i < cmpCount; i++) {
                if (currentTooth == cmpTeeth[i]) {
                    cmpState = !cmpState;
                    gpio_set_level(CMP_GPIO, cmpState);
                }
            }

            // Generate 1 tooth of the CKP signal
            if (ckpTooth < realTeeth) {
                gpio_set_level(CKP_GPIO, 1);
                ets_delay_us(period / 2);
                gpio_set_level(CKP_GPIO, 0);
                ets_delay_us(period / 2);
            }
            else {
                ets_delay_us(period);
            }
            
            // Reset the counter of the currentTooth every 2 cycle of the CKP signal
            if (currentTooth >= totalTeeth * 2){
                currentTooth = 0;
            }

        }
    }
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {

      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

}