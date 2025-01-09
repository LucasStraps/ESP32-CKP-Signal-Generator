#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <string.h> 
#include <driver/i2c.h> 
#include <driver/gpio.h>
#include <stdio.h>
#include "HD44780.h"
#define BUTTON_UP     GPIO_NUM_34
#define BUTTON_DOWN   GPIO_NUM_32
#define BUTTON_BACK   GPIO_NUM_33
#define BUTTON_CONFIRM GPIO_NUM_35

typedef struct {
    char syncName[15];
    int totalTeeth;
    int totalMissingTeeth;
    int cmpTeeth[10];
    int cmpCount;
} synchronism;

extern int selectedSync;
extern synchronism syncTable[];

void setupButtons(void);

void syncSelectMenu(void);

int readButton(gpio_num_t button);

#endif // MENU_H