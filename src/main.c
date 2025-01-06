#include <driver/i2c.h>
#include <driver/adc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "HD44780.h"

#define rpmPotGPIO ADC1_CHANNEL_5

//Custom Settings
int minRPM = 600;
int maxRPM = 10000;
int delayToUpdateRPM = 300;

int rpm = 600;
char displayMessage[16];

long map(long x, long in_min, long in_max, long out_min, long out_max);

void updateRPM (void* pvParameter);
void displayRPM (void* pvParameter);
void generateCKP (void* pvParameter);

void app_main() {
    //Settings for GPIO of RPM potentiometer
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(rpmPotGPIO,ADC_ATTEN_DB_12);

    //Settings for LCD display
    LCD_init(0x27, 21, 22, 16, 2);
    LCD_clearScreen();

    //Start update RPM loop
    xTaskCreatePinnedToCore (
        updateRPM,     // Function to implement the task
        "updateRPM",   // Name of the task
        2048,      // Stack size in bytes
        NULL,      // Task input parameter
        5,         // Priority of the task
        NULL,      // Task handle.
        1          // Core where the task should run
    );

    //Start update Display loop
    xTaskCreatePinnedToCore (
        displayRPM,     // Function to implement the task
        "displayRPM",   // Name of the task
        2048,      // Stack size in bytes
        NULL,      // Task input parameter
        5,         // Priority of the task
        NULL,      // Task handle.
        1          // Core where the task should run
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

long map(long x, long in_min, long in_max, long out_min, long out_max) {

      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

    }