#include "menu.h"

void setupButtons() {
    gpio_set_direction(BUTTON_UP, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_DOWN, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_BACK, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_CONFIRM, GPIO_MODE_INPUT);
}

int readButton(gpio_num_t button) {
    return gpio_get_level(button);
}

int getSyncCount() {
    int count = 0;
    while (syncTable[count].totalTeeth != 0 || strcmp(syncTable[count].syncName, "") != 0) {
        count++;
    }
    return count;
}

void LCD_writeStrWithPadding(char* str) {
    char paddedStr[16];
    snprintf(paddedStr, sizeof(paddedStr), "%-15s", str);
    LCD_writeStr(paddedStr);
}

void syncSelectMenu() {
    int selected = 0;
    int syncCount = getSyncCount();
    bool menuActive = true;
    bool buttonConfirm = false;
    setupButtons();

    LCD_clearScreen();
    LCD_setCursor(0, 0);
    LCD_writeStr("Select:");
    LCD_setCursor(0, 1);
    LCD_writeChar('>'); 
    

    while (menuActive) {
        LCD_setCursor(1, 1);
        // Needs to have padding to not use LCD_clearScreen avoiding screen flickering
        LCD_writeStrWithPadding(syncTable[selected].syncName);

        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (readButton(BUTTON_UP)) {
            printf("Up button pressed\n");
            selected = (selected - 1 + syncCount) % syncCount;
            vTaskDelay(250 / portTICK_PERIOD_MS);
        }

        if (readButton(BUTTON_DOWN)) {
            printf("Down button pressed\n");
            selected = (selected + 1) % syncCount;
            vTaskDelay(250 / portTICK_PERIOD_MS);
        }

        if (readButton(BUTTON_CONFIRM)) {
            printf("Confirm button pressed\n");
            selectedSync = selected;
            buttonConfirm = true;
            menuActive = false;
            vTaskDelay(250 / portTICK_PERIOD_MS);
        }

        if (readButton(BUTTON_BACK)) {
            printf("Back button pressed\n");
            menuActive = false;
            vTaskDelay(250 / portTICK_PERIOD_MS);
        }
    }

    LCD_clearScreen();
    LCD_setCursor(0, 0);
    if (buttonConfirm) {
        LCD_setCursor(0, 1);
        LCD_writeStr(syncTable[selected].syncName);
        LCD_setCursor(0, 0);
        LCD_writeStr("Change made");

        for (int i = 0; i < 3; i++) {
            vTaskDelay(500 / portTICK_PERIOD_MS);
            LCD_writeChar('.');
        }
    }
    else {
        LCD_writeStr("Canceling");

        for (int i = 0; i < 3; i++) {
            vTaskDelay(500 / portTICK_PERIOD_MS);
            LCD_writeChar('.');
        }
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    LCD_clearScreen();
}