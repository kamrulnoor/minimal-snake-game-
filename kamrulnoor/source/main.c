#include "gpio.h"              
#include "uart.h"              
#include "framebuffer.h"       
#include "snes_controller.h"   
#include "logo.h"              
#include "start2_image.h"      
#include "start1_image.h"      
#include "score1_image.h"      
#include "apple_image.h"       
#include "pausemenu.h"         
#include <stdint.h>            
#include <stdbool.h>           
#include <stdlib.h>            
#include <time.h>              
#include "gameover.h"         
#include "image0.h"            
#include "image1.h"            
#include "image2.h"            
#include "image3.h"            
#include "image4.h"            
#include "image5.h"  
#include "image6.h" 
#include "image7.h" 
#include "image8.h"
#include "image9.h"
        

// Structure for position with x and y coordinates
typedef struct {
    int x;
    int y;
} Position;

// Function to print strings via UART
void myPrintf(char *str) {
    uart_puts(str); // Uses UART's put string function to send text
}

// Button definitions for SNES controller
#define BUTTON_B 0
#define BUTTON_Y 1
#define BUTTON_SELECT 2
#define BUTTON_START 3
#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 7
#define BUTTON_A 8
#define BUTTON_X 9
#define BUTTON_L 10
#define BUTTON_R 11

// Menu options definitions
#define MENU_START_GAME 0
#define MENU_EXIT 1
#define PAUSE_RESTART_MENU 0
#define PAUSE_QUIT_GAME 1

// Global variables for game state
int currentMenuSelection = MENU_START_GAME; // Tracks the current menu selection
int movementStepSize = 2; // Step size for object movement, affecting game speed

// Game status variables
int lives = 3;             // Player's remaining lives
int score = 0;             // Player's score
bool gameStateUpdate = false; // Flag for game state updates
bool inPauseMenu = false;      // Flag to check if the game is in pause menu
bool buttonStartPressedPreviously = false; // Track the start button press state

// Wall size constant
#define WALL_SIZE 24
// Wall positions
Position wall1 = {0, 0};
Position wall2 = {0, 0};
Position wall3 = {0, 0};
Position wall4 = {0, 0};

// Function to wait for a specified time in microseconds
static void Wait(int microseconds) {
    unsigned c = *CLO_REG + microseconds; // Read current clock register and add delay
    while (c > *CLO_REG); // Wait until the clock register matches the delay
}

// Seed for random number generation
unsigned int lcg_seed = 0;

// Function to set the random seed
void srand(unsigned int seed) {
    lcg_seed = seed; 
}


int rand(void) {
    const unsigned int a = 1664525; 
    const unsigned int c = 1013904223; 
    lcg_seed = (a * lcg_seed + c); // Update the seed
    return (int)(lcg_seed & 0x7FFFFFFF); // Return the generated pseudo-random number
}

// Function to initialize the random number generator
void init_random() {
    srand((unsigned int)*CLO_REG); // Seed the random number generator with the current clock register value
}

// Function to display the game menu
void displayMenu(int selection) {
    drawRect(56, (selection == 0) ? 600 : 579, 68, (selection == 0) ? 612 : 591, 0xFF000000, 1); // Draw menu selection rectangle
    drawRect(56, (selection == 1) ? 600 : 579, 68, (selection == 1) ? 612 : 591, 0xFFFFFFFF, 1); // Draw menu selection rectangle
}

// Main menu loop
bool menuLoop() {
    displayMenu(currentMenuSelection); // Display the current menu
    drawImage((unsigned char *)logo.pixel_data, logo.width, logo.height, 608, 200); // Draw the logo
    drawImage((unsigned char *)start2_image.pixel_data, start2_image.width, start2_image.height, 578, 301); // Draw the start2 image
    drawImage((unsigned char *)start1_image.pixel_data, start1_image.width, start1_image.height, 81, 574); // Draw the start1 image

    while (true) {
        SNES_ReadButtons(); // Read the state of the SNES controller buttons
        if (SNES_IsButtonPressed(BUTTON_UP) || SNES_IsButtonPressed(BUTTON_DOWN)) {
            currentMenuSelection = !currentMenuSelection; // Toggle the menu selection
            displayMenu(currentMenuSelection); // Update the display
            Wait(200000); // Delay for button debounce
        }

        if (SNES_IsButtonPressed(BUTTON_A)) {
            return currentMenuSelection; // Return the current selection if button A is pressed
        }
        Wait(60000); // Short delay to manage menu navigation speed
    }
}

// Function to display the pause menu
void displayPauseMenu(int selection) {
    drawImage((unsigned char *)pausemenu_image.pixel_data, pausemenu_image.width, pausemenu_image.height, 554, 308); // Draw pause menu image
    int yPosRestart = 340; // Y position for the restart menu option
    int yPosQuit = 370; // Y position for the quit menu option
    drawRect(540, (selection == PAUSE_RESTART_MENU) ? yPosRestart : yPosQuit, 560, (selection == PAUSE_RESTART_MENU) ? yPosRestart + 20 : yPosQuit + 20, 0xFFFFFFFF, 1); // Highlight the selected menu option
}

// Function to get a random position within the play area
Position get_random_position(int playAreaWidth, int playAreaHeight) {
    Position pos;
    pos.x = rand() % playAreaWidth; // Random x position
    pos.y = rand() % playAreaHeight; // Random y position
    return pos; // Return the generated position
}

// Function to draw an apple on the screen at the specified position
void drawApple(Position applePosition) {
    drawImage((unsigned char *)apple_image.pixel_data, apple_image.width, apple_image.height, applePosition.x, applePosition.y); // Draw the apple image
}

// Initialize apple objects
int appleSize = 24; // Size of apple images
Position apple1 = {0,0}; 
Position apple2 = {0,0}; 
Position apple3 = {0,0}; 
Position apple4 = {0,0}; 
Position apple5 = {0,0}; 

// Function to initialize the apple positions
void init_apples(int playAreaWidth, int playAreaHeight) {
    // Set apple positions to random locations within the play area minus the apple size
    apple1 = get_random_position(playAreaWidth - appleSize, playAreaHeight - appleSize);
    apple2 = get_random_position(playAreaWidth - appleSize, playAreaHeight - appleSize);
    apple3 = get_random_position(playAreaWidth - appleSize, playAreaHeight - appleSize);
    apple4 = get_random_position(playAreaWidth - appleSize, playAreaHeight - appleSize);
    apple5 = get_random_position(playAreaWidth - appleSize, playAreaHeight - appleSize);
}

// Function to check for collisions between two objects
bool check_collision(Position pos1, int size1, Position pos2, int size2) {
    // Check if the areas of the two objects overlap
    return (pos1.x < pos2.x + size2 && pos1.x + size1 > pos2.x) &&
           (pos1.y < pos2.y + size2 && pos1.y + size1 > pos2.y);
}

// Function to check if the player has made contact with an apple
bool has_made_contact_with_apple(Position currentBoxPosition) {
    Position apples[] = {apple1, apple2, apple3, apple4, apple5}; 
    int numberOfApples = sizeof(apples) / sizeof(apples[0]); 

    for (int i = 0; i < numberOfApples; i++) {
        if (check_collision(currentBoxPosition, 10, apples[i], appleSize)) {
            drawRect(apples[i].x, apples[i].y, apples[i].x + 24, apples[i].y + 24, 0xFF000000, 1); // Clear the apple from the screen
            score++; // Increment the score
            gameStateUpdate = true; // Indicate that the game state needs to be updated
            return true; // Collision detected
        }
    }
    return false; // No collision detected
}

// Function to initialize wall positions
void init_walls(int playAreaWidth, int playAreaHeight) {
    // Set wall positions around the play area
    wall1.x = 50; wall1.y = 50;
    wall2.x = playAreaWidth - WALL_SIZE - 50; wall2.y = 50;
    wall3.x = 50; wall3.y = playAreaHeight - WALL_SIZE - 50;
    wall4.x = playAreaWidth - WALL_SIZE - 50; wall4.y = playAreaHeight - WALL_SIZE - 50;
}

// Function to draw a wall block at the specified position
void drawWall(Position wallPosition) {
    drawRect(wallPosition.x, wallPosition.y, wallPosition.x + WALL_SIZE, wallPosition.y + WALL_SIZE, 0xFFFFFFFF, 1); // Draw wall in white color
}

// Function to check if the player has hit a wall
bool has_hit_wall(Position currentBoxPosition) {
    Position walls[] = {wall1, wall2, wall3, wall4}; // Array of wall positions
    int numberOfWalls = sizeof(walls) / sizeof(walls[0]); // Calculate the number of walls

    for (int i = 0; i < numberOfWalls; i++) {
        if (check_collision(currentBoxPosition, 10, walls[i], WALL_SIZE)) {
            return true; // Collision detected
        }
    }
    return false; // No collision detected
}

// Main game function
int main() {
    uart_init(); 
    SNES_Init(); 
    init_framebuffer(); 
    init_random(); 
    
    fillScreen(0xFF000000); // Clear the screen

    int pauseSelection = PAUSE_RESTART_MENU; // Initialize pause menu selection

    bool startGame = menuLoop(); // Start the main menu loop
    if (startGame == MENU_EXIT) {
        myPrintf("Exiting...\n"); // Print exit message
        return 0; // Exit the game
    }

    // Define the main gameplay area dimensions
    int mainBoxWidth = 1279;
    int mainBoxHeight = 719;
    int boxWidth = 10; 
    int boxHeight = 10; 
    int boxX = 630; 
    int boxY = 350; 
    Position currentBoxPosition = {boxX, boxY}; 
    uint32_t boxColor = 0xFF00FF00; // Color of the player's box (green)
    int playAreaWidth = mainBoxWidth; 
    int playAreaHeight = mainBoxHeight - score1_image.height - 6; 

    fillScreen(0xFF000000); // Clear the screen again for the game start
    int prevBoxX = boxX; 
    int prevBoxY = boxY; 

    drawImage((unsigned char *)score1_image.pixel_data, score1_image.width, score1_image.height, 0, playAreaHeight + 1); // Draw the score image

    init_apples(playAreaWidth, playAreaHeight); // Initialize apple positions
    drawApple(apple1); 
    drawApple(apple2); 
    drawApple(apple3); 
    drawApple(apple4); 
    drawApple(apple5); 

    init_walls(playAreaWidth, playAreaHeight); // Initialize wall positions
    drawWall(wall1); 
    drawWall(wall2); 
    drawWall(wall3);
    drawWall(wall4); 

    bool isGameStateValid = true; // Flag to check if the game state is still valid
    while (isGameStateValid) { // Main game loop
        SNES_ReadButtons(); // Read the state of the SNES controller buttons

        if (!inPauseMenu) { // If not in pause menu, proceed with game logic
            drawRect(prevBoxX, prevBoxY, prevBoxX + boxWidth, prevBoxY + boxHeight, 0xFF000000, 1); // Clear the old box position

            if (SNES_IsButtonPressed(BUTTON_UP)) boxY -= movementStepSize; // Move up
            if (SNES_IsButtonPressed(BUTTON_DOWN)) boxY += movementStepSize; // Move down
            if (SNES_IsButtonPressed(BUTTON_LEFT)) boxX -= movementStepSize; // Move left
            if (SNES_IsButtonPressed(BUTTON_RIGHT)) boxX += movementStepSize; // Move right

            // Update the current position of the player's box
            currentBoxPosition.x = boxX;
            currentBoxPosition.y = boxY;

            if (has_made_contact_with_apple(currentBoxPosition)) {
                // Handle apple contact logic
            }

            if (has_hit_wall(currentBoxPosition)) {
                myPrintf("Hit wall!\n"); // Print collision message
                isGameStateValid = false; // End game loop on collision with wall
            }

            // Ensure the player's box stays within the play area boundaries
            boxX = (boxX < 0) ? 0 : (boxX + boxWidth > playAreaWidth) ? playAreaWidth - boxWidth : boxX;
            boxY = (boxY < 0) ? 0 : (boxY + boxHeight > playAreaHeight) ? playAreaHeight - boxHeight : boxY;

            // Draw the player's box at the new position
            drawRect(boxX, boxY, boxX + boxWidth, boxY + boxHeight, boxColor, 1);

            // Update previous position for next loop iteration
            prevBoxX = boxX;
            prevBoxY = boxY;
        } else { // Pause menu logic
            if (SNES_IsButtonPressed(BUTTON_A)) {
                // Confirm selection in the pause menu
                if (pauseSelection == PAUSE_QUIT_GAME) {
                    myPrintf("Quitting game...\n"); // Print quitting message
                    return 0; // Exit the game
                } else {
                    myPrintf("Restarting game...\n"); // Print restarting message
                    inPauseMenu = false; // Exit pause menu
                    fillScreen(0xFF000000); // Refresh screen to clear the pause menu
                    drawImage((unsigned char *)score1_image.pixel_data, score1_image.width, score1_image.height, 0, playAreaHeight + 1); // Redraw the score image
                    
                    init_apples(playAreaWidth, playAreaHeight); // Reinitialize apple positions
                    drawApple(apple1); 
                    drawApple(apple2); 
                    drawApple(apple3); 
                    drawApple(apple4); 
                    drawApple(apple5); 


                    init_walls(playAreaWidth, playAreaHeight); // Reinitialize wall positions
                    drawWall(wall1); 
                    drawWall(wall2); 
                    drawWall(wall3); 
                    drawWall(wall4); 
                }
            }

            if (SNES_IsButtonPressed(BUTTON_UP) || SNES_IsButtonPressed(BUTTON_DOWN)) {
                pauseSelection = !pauseSelection; // Toggle pause menu selection
                displayPauseMenu(pauseSelection); // Update pause menu display
                Wait(200000); 
            }
        }

        if (SNES_IsButtonPressed(BUTTON_START) && !buttonStartPressedPreviously) {
            if (!inPauseMenu) {
                myPrintf("Game paused\n"); // Print pause message
                inPauseMenu = true; // Enter pause menu
                displayPauseMenu(pauseSelection); // Show pause menu
            } else {
                buttonStartPressedPreviously = true; // Update button state
            }
        } else {
            buttonStartPressedPreviously = SNES_IsButtonPressed(BUTTON_START); // Update button state for next loop iteration
        }

        Wait(30000); // Delay for frame timing
    }

    return 0; 
}
