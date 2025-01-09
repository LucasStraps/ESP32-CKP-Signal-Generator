#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <string.h> 
#include <driver/i2c.h> 
#include <driver/gpio.h>
#include <stdio.h>
#include "HD44780.h"

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

#endif // MENU_H