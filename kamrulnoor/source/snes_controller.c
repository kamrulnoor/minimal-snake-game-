#include "snes_controller.h"
#include "gpio.h"
#include "uart.h"

// GPIO pins for SNES controller
static const int CLK = 11;
static const int LAT = 9;
static const int DAT = 10;

// Array to store the state of each button (1 = pressed, 0 = not pressed)
static int buttons[16] = {0};

// Delay function
static void Wait(int microseconds) {
    unsigned c = *CLO_REG + microseconds;
    while (c > *CLO_REG);
}

// Functions to configure GPIO pins
static void INP_GPIO(int pin) {
    *(BASE + (pin / 10)) &= ~(7 << ((pin % 10) * 3));
}

static void OUT_GPIO(int pin) {
    INP_GPIO(pin); // First reset the pin to input
    *(BASE + (pin / 10)) |= (1 << ((pin % 10) * 3));
}

// Read the value from a specific GPIO pin
static int Read_Data(void) {
    return (*(GPLEV0) >> DAT) & 1;
}

// Write a value to a specific GPIO pin
static void write_gpio(int pin, int value) {
    if (value)
        *(GPSET0) = 1 << pin;
    else
        *(GPCLR0) = 1 << pin;
}

void SNES_Init(void) {
    INP_GPIO(CLK);
    OUT_GPIO(CLK);

    INP_GPIO(LAT);
    OUT_GPIO(LAT);

    INP_GPIO(DAT); // DAT pin is input for reading button states
}

void SNES_ReadButtons(void) {
    write_gpio(LAT, 1);
    Wait(12);
    write_gpio(LAT, 0);

    for (int i = 0; i < 16; i++) {
        buttons[i] = Read_Data();
        write_gpio(CLK, 0);
        Wait(6);
        write_gpio(CLK, 1);
        Wait(6);
    }
}

int SNES_IsButtonPressed(int button) {
    if (button >= 0 && button < 16) {
        if (buttons[button] == 0) {
            return 1;
        }
    }
    return 0; // Invalid button number
}
