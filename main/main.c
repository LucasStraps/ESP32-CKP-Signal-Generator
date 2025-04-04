#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <rom/ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "HD44780.h"
#include "menu.h"

#define RPM_GPIO ADC_CHANNEL_4
#define CKP_GPIO GPIO_NUM_25
#define CMP_GPIO GPIO_NUM_26
// Custom Settings
int minRPM = 600;
int maxRPM = 10000;
int delayToUpdateRPM = 300;

synchronism syncTable[] = {
    {"VW 60-2", 60, 2, {14, 19, 27, 49, 57, 79, 104, 110}, 8},
    {"Fire 60-2", 60, 2, {8, 30, 38, 59, 68, 99}, 6},
    {"Sync Test", 100, 4, {50, 51}, 2},
    {"", 0, 0, {0}, 0} // end indicator
};

int selectedSync = 0;
int rpm = 600;
char displayMessage[16];
bool generatingSignal;

long map(long x, long in_min, long in_max, long out_min, long out_max);

void updateRPM (void* pvParameter);
void displayRPM (void* pvParameter);
void generateSignal (void* pvParameter);
void startUpdateRPM();
void startDisplayRPM();
void startGenerateSignal();
void startCheckForRestart();

extern void app_main() {
    // Init GPIO of output signal
    gpio_set_direction(CKP_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(CMP_GPIO, GPIO_MODE_OUTPUT);
    
    //Init LCD display
    LCD_init(0x27, 21, 22, 16, 2);
    LCD_clearScreen();

    // Display menu and start tasks
    syncSelectMenu();
    generatingSignal = true;
    startUpdateRPM();
    startDisplayRPM();
    startGenerateSignal();
    startCheckForRestart();
}

void updateRPM (void* pvParameter) {
    printf("UpdateRPM started\n");
    int rpmPotValue = 0;
    adc_oneshot_unit_handle_t rpmPotHandle;
    adc_oneshot_unit_init_cfg_t rpmPotInitConfig = {.unit_id = ADC_UNIT_2};
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&rpmPotInitConfig, &rpmPotHandle));

    adc_oneshot_chan_cfg_t rpmPotChanConfig = {.bitwidth = ADC_BITWIDTH_12, .atten = ADC_ATTEN_DB_12};
    ESP_ERROR_CHECK(adc_oneshot_config_channel(rpmPotHandle, RPM_GPIO , &rpmPotChanConfig));

    while (generatingSignal) {
        ESP_ERROR_CHECK(adc_oneshot_read(rpmPotHandle, RPM_GPIO, &rpmPotValue));
        rpm = map(rpmPotValue, 0, 4095, minRPM, maxRPM);
        vTaskDelay(pdMS_TO_TICKS(delayToUpdateRPM));
    }
    printf("updateRPM task ending\n");
    vTaskDelete(NULL);
}

void displayRPM (void* pvParameter) {
    printf("display RPM Started\n");
    while (generatingSignal) {
        LCD_home();
        snprintf(displayMessage, sizeof(displayMessage), "RPM: %d       ", rpm);
        LCD_writeStr(displayMessage);
        vTaskDelay(pdMS_TO_TICKS(delayToUpdateRPM));
    }
    printf("displayRPM task ending\n");
    vTaskDelete(NULL);
}

void generateSignal (void* pvParameter) {
    printf("Started signal\n");
    synchronism sync = syncTable[selectedSync];
    int currentTooth = 0;
    int cmpState = 0;

    while (generatingSignal) {
        int oneMinuteinUs = 60000000;
        int period = oneMinuteinUs / (rpm * sync.totalTeeth);  // Calculates the period of one tooth in µs
        int realTeeth = sync.totalTeeth - sync.totalMissingTeeth;

        for (int ckpTooth = 0; ckpTooth < sync.totalTeeth; ckpTooth++) {
            currentTooth++;

            // Checks if the currentTooth is the cmp and change its state
            for (int i = 0; i < sync.cmpCount; i++) {
                if (currentTooth == sync.cmpTeeth[i]) {
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
            if (currentTooth >= sync.totalTeeth * 2){
                currentTooth = 0;
            }
        }
    }
    printf("generateSignal task ending\n");
    vTaskDelete(NULL);
}
void checkForRestart(void* pvParameter) {
    printf("Restart check task started\n");
    while (generatingSignal) {
        if (readButton(BUTTON_CONFIRM) || readButton(BUTTON_UP) || readButton(BUTTON_DOWN)) {
            printf("Button pressed, restarting device...\n");
            generatingSignal = false;
            vTaskDelay(pdMS_TO_TICKS(250)); 
            printf("Restarting device...\n");
            esp_restart(); 
        }
        vTaskDelay(pdMS_TO_TICKS(250)); 
    }
}
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void startUpdateRPM () {
    xTaskCreatePinnedToCore (
        updateRPM,     // Function to implement the task
        "updateRPM",   // Name of the task
        1024,      // Stack size in bytes
        NULL,      // Task input parameter
        5,         // Priority of the task
        NULL,      // Task handle.
        1          // Core where the task should run
    );
}

void startDisplayRPM () {
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

void startGenerateSignal () {
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
void startCheckForRestart() {
    xTaskCreatePinnedToCore (
        checkForRestart,     // Function to implement the task
        "checkForRestart",   // Name of the task
        2048,      // Stack size in bytes
        NULL,      // Task input parameter
        5,         // Priority of the task
        NULL,      // Task handle.
        1          // Core where the task should run
    );
}