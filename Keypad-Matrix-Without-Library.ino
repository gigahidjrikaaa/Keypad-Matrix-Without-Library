/************************************************************************/
/************************************************************************/
/******                                                            ******/
/******            Code made by Giga Hidjrika Aura Adkhy           ******/
/******                      21/479228/TK/52833                    ******/
/******            Keypad Matrix reader (without library)          ******/
/******              Print the Keys to the OLED & Serial           ******/
/****** Treat the keypad Matrix as if it was an old school keypad. ******/
/******                                                            ******/
/************************************************************************/
/************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // Obtained from I2C Scanner. Look for the program in Github.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED class object declaration

// Function to make the OLED displays content centered with X and Y offset
void displayCenter(String text, int X, int Y) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.clearDisplay();
  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  display.setCursor((SCREEN_WIDTH - width) / 2 + X, (SCREEN_HEIGHT - height) / 2 + Y);
  display.println(text);
  display.display();
}

// OLED Splash Screen
void splashScreen()
{
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(3);
  displayCenter("KEYPAD MATRIX", 0, 0);
  delay(2000);
  display.clearDisplay();
}

const int numRows = 4; // Number of rows in the keypad
const int numCols = 4; // Number of columns in the keypad

// Define the pins for the keypad
int colPins[numCols] = {27, 14, 12, 13};
int rowPins[numRows] = {32, 33, 25, 26};
// Declare the last pressed and last non-null key
char lastPressed;
char lastNonNull;
String word;

// Define the characters for each key
char keyMap[numRows][numCols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Define the DEPTH of the keys and what characters are assigned to each key. Null is assigned as the last member of each array to be a landmark.
#define DEPTH 6
char oldSchoolKeys[numRows][numCols][DEPTH] = 
{
  {{'.', ',', '?', '!', '1', '\0'}, {'A', 'B', 'C', '2', '\0'}, {'D', 'E', 'F', '3', '\0'}, {'^', '\0'}},
  {{'G', 'H', 'I', '4', '\0'}, {'J', 'K', 'L', '5', '\0'}, {'M', 'N', 'O', '6', '\0'}, {'\b', '\0'}},
  {{'P', 'Q', 'R', 'S', '6', '\0'}, {'T', 'U', 'V', '8', '\0'}, {'W', 'X', 'Y', 'Z', '9', '\0'}, {'\t', '\0'}},
  {{'*', '+', '(', '$', '%', '\0'}, {'0', ' ', '\0'}, {'#', '-', '/', ')', '@', '\0'}, {'\n', '\0'}}
};

void setup() {
  Serial.begin(921600); // Open serial communication
  for (int i = 0; i < numRows; i++) {
    pinMode(rowPins[i], OUTPUT); // Set rows as output pins
    digitalWrite(rowPins[i], HIGH); // Set rows high initially
  }
  for (int i = 0; i < numCols; i++) {
    pinMode(colPins[i], INPUT_PULLUP); // Set columns as input pins with pull-up resistors
  }
  // Checks if the OLED is connected or not.
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  splashScreen();
}

// Declare the counter for each keys.
int counter[numRows][numCols];
// Declare a global variable for the current row and current column read.
int currRow, currCol;
// Declare the reset time for the key press.
int resetTime;

void loop() {
  char keyPressed = readKeypad(); // Read the keypad and get the pressed key
  char keyValue = oldSchoolKeys[currRow][currCol][counter[currRow][currCol]]; // Get the value of the key based on how many times pressed
  if (keyPressed != '\0' && lastPressed != keyPressed) { // If key is pressed and only trigger once
    if (lastNonNull != keyPressed)  // If the last key pressed is different than the current one, reset the counter to 0.
      counter[currRow][currCol] = 0;
    if (keyValue == 0){  // If the key value is NULL ('\0') then reset to the first key value.
      counter[currRow][currCol] = 0;
      // Serial.printf("Null found! Resetting. Counter: %d\n", counter[currRow][currCol]); // For debugging only.
    }
    keyValue = oldSchoolKeys[currRow][currCol][counter[currRow][currCol]];  // Refreshes the key value
    if (keyValue == '^')  // Capslock character (assigned)
      capsLock();
    else if (keyValue == '\b')  // Backspace character
      displayCenter("BACK", 0, 0);
    else if (keyValue == '\t')  // Tab character
      displayCenter("TAB", 0, 0);
    else if (keyValue == '\n')  // Newline/Enter character
      displayCenter("ENTER", 0, 0);
    else if (keyValue == ' ')   // Space character
      displayCenter("SPACE", 0, 0);
    else
      displayCenter(String(keyValue), 0, 0);
    
    // Serial.printf("%c | %d | %d\n", keyValue, keyValue, counter[currRow][currCol]);  // Print for debugging
    Serial.printf("Pressed Key: %c\n", keyValue); // Print the pressed key to the serial monitor
    
    counter[currRow][currCol]++;  // Add the key counter
    lastNonNull = keyPressed; // Asssign the pressed key (non-null) to the last non-null key pressed
    resetTime = millis(); // Make the reset time the same as millis to restart counterReset()
  }
  else if (keyPressed == '\0')  // If no key is pressed
  {
    counterReset(); // Reset the counter if it is > 2500ms
  }
  lastPressed = keyPressed; // Assign the pressed key to the last pressed key.
}

void counterReset()
{
  if(millis() - resetTime > 2500) // If button is not pressed more than 2500 miliseconds
  {
    for(int i = 0; i < numRows; i++)
    {
      for(int j = 0; j < numCols; j++)
      {
        counter[i][j] = 0;  // Reset all counters to 0;
      }
    }
    displayCenter(" ",0,0); // Clear the screen
  }
}

char readKeypad() {
  for (int i = 0; i < numRows; i++) {
    digitalWrite(rowPins[i], LOW); // Set the current row low
    for (int j = 0; j < numCols; j++) {
      if (digitalRead(colPins[j]) == LOW) {
        delay(10); // Debounce the button press
        digitalWrite(rowPins[i], HIGH); // Set the row back to high
        currRow = i;
        currCol = j;
        return keyMap[i][j]; // Return the pressed key
      }
    }
    digitalWrite(rowPins[i], HIGH); // Set the row back to high
  }
  return '\0'; // Return null if no key is pressed
}

// Capslock function
void capsLock()
{
  bool caps = 0;  // Variable to know if capslock is on or off
  for(int i = 0; i < numRows; i++)
  {
    for(int j = 0; j < numCols; j++)
    {
      for(int k = 0; k < DEPTH; k++)
      {
        if(oldSchoolKeys[i][j][k] >= 'a' && oldSchoolKeys[i][j][k] <= 'z')  // If capslock is off (letters are small), turn on
        {
          oldSchoolKeys[i][j][k] = oldSchoolKeys[i][j][k] - 32;
          caps = true;
        }
        else if(oldSchoolKeys[i][j][k] >= 'A' && oldSchoolKeys[i][j][k] <= 'Z') // If capslock is on (letters are CAPITALIZED), turn off
        {
          oldSchoolKeys[i][j][k] = oldSchoolKeys[i][j][k] + 32;
          caps = false;
        }
      }
    }
  }
  // Show 'Caps' depending on the state.
  if(caps)
    displayCenter("CAPS", 0, 0);
  if(!caps)
    displayCenter("caps", 0, 0);
}
