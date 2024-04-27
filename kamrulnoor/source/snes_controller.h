#ifndef SNES_CONTROLLER_H
#define SNES_CONTROLLER_H

// Initialize GPIO pins for SNES controller
void SNES_Init(void);

// Read the current state of all buttons on the SNES controller
void SNES_ReadButtons(void);

// Check if a specific button is pressed. Returns 1 if pressed, 0 otherwise.
// Button numbers: 0-15 corresponding to the SNES controller buttons
int SNES_IsButtonPressed(int button);

#endif // SNES_CONTROLLER_H
