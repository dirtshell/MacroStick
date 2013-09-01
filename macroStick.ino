//-------------------------------------------------------------
//  Macro Arcade Stick Controller v0.8
//  By Jacob Kenin
// 
//  This version lacks the ability to interact with 
//  an SD card. I am replacing the SD card with using PROGMEM
//
//  Work  needs to be done so that the system can adress all
//  96000 adresses in b_combos. A Mega can only adress 35000 of
//	them. A Due though doesnt have this issue
//  
//  Last updated on April 10, 2013
//--------------------------------------------------------------
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <BitBool.h>

SoftwareSerial mySerial(2, 3); // RX, TX

// Size of the LCD
const int numRows = 4;
const int numCols = 20;

LiquidCrystal lcd(7, 6, 5, 4, 3, 2); // Initialize the library with the numbers of the interface pins

// Cursor empty
byte emptyCursor[8] = {
  0b11111,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b11111
};

// Cursor selected
byte fullCursor[8] = {
  0b11111,
  0b10001,
  0b10101,
  0b10101,
  0b10101,
  0b10101,
  0b10001,
  0b11111
};

// Cursor pressed
byte pressedCursor[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

// ----------------------------------------------------
//
//  Current Combos
//    Combo 1 (Assigned Button Goes Here)
//    Combo 2 (Assigned Button Goes Here)
//      Edit
//      Rename
//      Execute
//      View (Actual commands)
//      Info
//        Character: Character_Goes_Here (Default to HUZZHAH)
//        Damage: Record_Damage_Here (Default to 0)
//        Meter: Meter_Used (Default to 0)
//        Length: Buffers_Times_Delay_Divided_By_1000
//        Advantage: Advantage_Goes_Here (Default to 0)
//      Reassign
//      Delete
//   Add New
//     From File (Request the button to be assigned to)
//     Record (Request the button to be assigned to)
//
// ----------------------------------------------------
// Menu array
//  0: Main Menu (Temp name, not seen by user)
//    0: Current Combos
//    1: New Combo
//  1: Current Combos (Temp name, not seen by user)
//    0: Combo 1
//    1: Combo 2
//    2: Combo 3
//    3: Combo 4
//    4: Combo 5
//    5: Combo 6
//    6: Combo 7
//    7: Combo 8
//  2: Combo Menu (Temp name, not seen by user)
//    0: Edit
//    1: Rename
//    2: Execute
//    3: View
//    4: Info
//    5: Reassign
//    6: Delete
//  3: Info (Temp name, not seen by user)
//    0: Character
//    1: Damage
//    2: Meter
//    3: Length
//    4: Advantage
//  4: New Combo (Temp name, not seen by user)
//    0: Record
//    1: From File 

// Variables needed to move around the menu
int lcdRow = 0; // Create a var for storing the cursors current row in the menu (starts at 0)
int menuPos = 0; // Menu position in menu array
int subPos = 0; // Sub Menu position (ie sub combos etc)
boolean menuFlag = true; // Menu flag (HIGH means in menu[], as opposed to menu[][])
int menuSize[5] = {2,8,7,5,2}; // Stores the number of elements in each menu (Main Menu, Combo List, Combo Menu, Info, New Combo) 
// Array containing the names of the combos, in order
String comboNames[8] = {
  " Combo 1",
  " Combo 2",
  " Combo 3",
  " Combo 4",
  " Combo 5",
  " Combo 6",
  " Combo 7",
  " Combo 8" 
};

String menu[5][8]= {
  // Main menu
  {
    " Current Combos",
    " New Combos"
  },
  // Current Combos
  {
    comboNames[0],
    comboNames[1],
    comboNames[2],
    comboNames[3],
    comboNames[4],
    comboNames[5],
    comboNames[6],
    comboNames[7]
  },
  // Combo Menu
  {
    " Edit",
    " Rename",
    " Execute",
    " View",
    " Info",
    " Reassign",
    " Delete"
  },
  // Info
  {
    " Character",
    " Damage",
    " Meter",
    " Length",
    " Advantage"
  },
  // New Combo
  {
    " Record",
    " From File"
  }
};

// Outputs
int xOut = 22;
int circleOut = 23;
int right2Out = 24;
int left2Out = 25;
int squareOut = 26;
int triangleOut = 27;
int right1Out = 28;
int left1Out = 29;
int downOut = 30;
int upOut = 31;
int leftOut = 32; // Could be pin 33
int rightOut = 33; // Could be pin 32
int selectOut = 34;
int startOut = 35;

// Inputs
int xIn = 38;
int circleIn = 39;
int right2In = 40;
int left2In = 41;
int squareIn = 42;
int triangleIn = 43;
int right1In = 44;
int left1In = 45;
int downIn = 46;
int upIn = 47;
int leftIn = 48; // Could be pin 33
int rightIn = 49; // Could be pin 32
int selectIn = 50;
int startIn = 51;
int record = 52;
int shift = 53;

// currMode flag (1 = console mode is activated)
byte currMode = 0;

// Buffer
// 0  = X
// 1  = Circle
// 2  = R2
// 3  = L2
// 4  = Square
// 5  = Triangle
// 6  = R1
// 7  = L1
// 8  = Down
// 9  = Up
// 10 = Left
// 11 = Right
// 12 = Select
// 13 = Start
// 14 = Record
// 15 = Shift
byte buffer[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // One slot for each button (1 or 0)

int maxLength = 1000; // Will be updated later

// NEEDS TO BE FIXED TO DETECT ON STARTUP
// Used to store the length of a given combo
// Write a script that calculates these at startup
int comboLength[8] = { maxLength,maxLength,maxLength,maxLength,maxLength,maxLength,maxLength,maxLength };

// Temp int used for storing the button for the current recording
int macroButton = 0;

// Recording flag (false by default)
boolean recFlag = false;

// Keeps track of the length of a combo being recorded
int recIteration = 0;

// Saved combos
// Maximum of 8 saved combos
// Maximum length of combos is 30 seconds (1000 buffers)
// Each buffer has 12 booleans (buttons) does not include start and select
// the central number should be equal to maxLength
// This is a simple one. 8*12*100=9600 elements in the bitBool object for 8 combos 
// Eventually I will tweak this up to 1000 buffers

// Each combo consists of 12*maxLength bits, so the first combo would end at b_combos[12*maxLength-1]
// Each buffer is 12 bits, so the first buffer ends at b_combos[11]
BitBool<96000> b_combos; // Later, this will be tweaked to 24000 once I am sure it works

byte tempBuffer[12]; // This is a temporary buffer to be used by the bitBool conversion function

void setup()  
{
  // Setting up inputs
  pinMode(circleIn, INPUT_PULLUP);
  pinMode(xIn, INPUT_PULLUP); // Configure the x input pin
  pinMode(triangleIn, INPUT_PULLUP);
  pinMode(squareIn, INPUT_PULLUP);
  pinMode(record, INPUT_PULLUP);
  pinMode(right2In, INPUT_PULLUP);
  pinMode(left2In, INPUT_PULLUP);
  pinMode(right1In, INPUT_PULLUP);
  pinMode(left1In, INPUT_PULLUP);
  pinMode(leftIn, INPUT_PULLUP);
  pinMode(rightIn, INPUT_PULLUP);
  pinMode(upIn, INPUT_PULLUP);
  pinMode(downIn, INPUT_PULLUP);
  pinMode(selectIn, INPUT_PULLUP);
  pinMode(startIn, INPUT_PULLUP);
  pinMode(shift, INPUT_PULLUP);
  
  // Setting up outputs
  pinMode(circleOut, OUTPUT);
  pinMode(xOut, OUTPUT);
  pinMode(triangleOut, OUTPUT);
  pinMode(squareOut, OUTPUT);
  pinMode(right2Out, OUTPUT);
  pinMode(left2Out, OUTPUT);
  pinMode(right1Out, OUTPUT);
  pinMode(left1Out, OUTPUT);
  pinMode(leftOut, OUTPUT);
  pinMode(rightOut, OUTPUT);
  pinMode(upOut, OUTPUT);
  pinMode(downOut, OUTPUT);
  pinMode(selectOut, OUTPUT);
  pinMode(startOut, OUTPUT);
  
  digitalWrite(circleOut, HIGH); // Defaults to HIGH (not pressed)
  digitalWrite(xOut, HIGH); 
  digitalWrite(triangleOut, HIGH);
  digitalWrite(squareOut, HIGH);
  digitalWrite(right2Out, HIGH);
  digitalWrite(left2Out, HIGH);
  digitalWrite(left1Out, HIGH);
  digitalWrite(right1Out, HIGH);
  digitalWrite(upOut, HIGH);
  digitalWrite(downOut, HIGH);
  digitalWrite(leftOut, HIGH);
  digitalWrite(rightOut, HIGH);
  digitalWrite(startOut, HIGH);
  digitalWrite(selectOut, HIGH);
  
  // Creates the empty, full, and pressed cursors
  lcd.createChar(4, emptyCursor);
  lcd.createChar(5, fullCursor);
  lcd.createChar(6, pressedCursor);
  
  // Set up the LCD's number of columns and rows: 
  lcd.begin(numCols,numRows);
  
  // Create the default layout
  lcd.write(5);
  lcd.print(" Current Combos");
  lcd.setCursor(0,1);
  lcd.write(4);
  lcd.print(" New Combo");
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
   while (!Serial) {
    ;
  }
}  

/***********************************************/
/*************** PS3 FUNCTIONS *****************/
/***********************************************/

// Print the buffer
// Useful for debugging
void printBuffer()
{
  for (int i = 0; i <= 15; i++) {
      if (buffer[i]) { Serial.print("1 "); }
      else Serial.print("0 ");
    }
  Serial.println(); // Signals the end of a buffer cycle
  delay(30); // Delay for readability
}

// Print the tempBuffer
// Useful for debugging
void printTempBuffer()
{
  for (int i = 0; i <= 15; i++) {
      if (tempBuffer[i]) { Serial.print("1 "); }
      else Serial.print("0 ");
    }
  Serial.println(); // Signals the end of a buffer cycle
  delay(30); // Delay for readability
}

// Send the buffer to the PS3
// Currently contains the same functionality as printBuffer, but also changes pins
void sendBuffer()
{
  for (int i = 0; i <= 15; i++) {
      if (buffer[i]) {
        Serial.print("1 ");
        digitalWrite(i+22, LOW);
      }
      else Serial.print("0 ");
    }
  Serial.println(); // Signals the end of a buffer cycle
  delay(30); // Delay for PS3 detection
}

// Send a specified buffer to the PS3
// Allows you to send a specific buffer to the PS3
void sendBuffer(byte specificBuffer[12])
{
  for (int i = 0; i <= 11; i++) {
      if (specificBuffer[i]) {
        Serial.print("1 ");
        digitalWrite(i+22, LOW);
      }
      else Serial.print("0 ");
    }
  Serial.println(); // Signals the end of a buffer cycle for serial monitor
  delay(30); // Delay for PS3 detection
}

// Reset the buffer
// Used to reset the buffer and its corresponding pins
void resetBuffer()
{
  for (int i = 0; i <= 15; i++) {
    buffer[i] = false;
    digitalWrite(i+22, HIGH);
  } 
}

// Creates the buffer
// Used to create the buffer for the current cycle
void createBuffer()
{
  // If the button was pressed set the appropriate buffer frame high
  for (int frame = 0; frame <= 15; frame++) {
    if (!digitalRead(frame+38)) { buffer[frame] = true; } // Checks the appropriate pin and sends the corresponding buffer index appropriately
  }
}


/***********************************/
/********* MENU FUNCTIONS **********/
/***********************************/

// Check if it should be in console mode (shift + record) and change the consoleFlag accordingly
// The debouncing delay doesn't matter because you will not switch mode at a critical time (theoretically)
void consoleCheck() 
{
  if (buffer[14] && buffer[15]) { 
    if (currMode) { currMode = 0; }// Deactivate
    else { currMode = 1; } // Activate
    delay(500); // Debounce
  }
}

// Moves up one in the menu
// Used to easily navigate through the menu
void menuUp()
{
  lcdRow--; // Decrease lcdRow
  subPos--; // Increases the position in menu[menuPos][] array
  // If the menuPos is at a new screen
  if (lcdRow == -1) {
    lcdRow++; // Increase the lcdRow to a realistic value
    lcd.clear(); // Clear the screen
    lcd.write(5);
    lcd.print(menu[menuPos][subPos]); // Print the current menu item
    lcd.setCursor(0,lcdRow+1);
    lcd.write(4);
    lcd.print(menu[menuPos][subPos+1]); // Print the menu item one below
    lcd.setCursor(0,lcdRow+2);
    lcd.write(4);
    lcd.print(menu[menuPos][subPos+2]); // Print the menu item two below
    lcd.setCursor(0,lcdRow+3);
    lcd.write(4);
    lcd.print(menu[menuPos][subPos+3]); // Print the menu item three below
  }
  else {
    lcd.setCursor(0,lcdRow+1);
    lcd.write(4); // Write an empty cursor at the previous position
    lcd.setCursor(0,lcdRow);
    lcd.write(5); // Write a full cursor one above the prvious position
  }
  delay(200); // To prevent producing a ton of up commands when only one is wanted
}

// Move down in the menu
// Easily move down in the menu
void menuDown()
{
  lcdRow++; // Increase lcdRow
  subPos++; // Decreases position in menu array 
  // If the menuPos is at a new screen
  if (lcdRow == numRows) {
    lcdRow--; // Decrease the lcdRow to a realistic value
    lcd.clear(); // Clear the screen
    lcd.write(4);
    lcd.print(menu[menuPos][subPos-3]); // Print the menu item three above
    lcd.setCursor(0,lcdRow-2);
    lcd.write(4);
    lcd.print(menu[menuPos][subPos-2]); // Print the menu item two above
    lcd.setCursor(0,lcdRow-1);
    lcd.write(4);
    lcd.print(menu[menuPos][subPos-1]); // Print the menu item one above
    lcd.setCursor(0,lcdRow);
    lcd.write(5);
    lcd.print(menu[menuPos][subPos]); // Print the current menu item
  }
  else {
    lcd.setCursor(0,lcdRow-1);
    lcd.write(4); // Write an empty cursor at the previous position
    lcd.setCursor(0,lcdRow);
    lcd.write(5); // Write a full cursor one below the previous position
  }
  delay(200); // To prevent producing a ton of up commands when only one is wanted
}

// Move back in the menu
// Simple code that sends you to the menu above
void menuBack()
{
  if (menuPos == 1) menuPos = 0; // Reset to main menu
  if (menuPos == 2) menuPos = 1; // Reset to the combo list
  if (menuPos == 3) menuPos = 2; // Reset to the combo menu
  if (menuPos == 4) menuPos = 0; // Reset to main menu
  if (menuPos != 0) { // Prevents pressing circle on the main menu from resetting subPos and lcdRow
    subPos = 0; // Resets the subPos
    lcdRow = 0; // Resets the lcdRow
  }
  renderMenu(); // Display the menu
  delay(200); // To prevent producing extra back inputs
}

// Renders the current menu[menuPos]
// Useful for displaying whatever menu your on
void renderMenu()
{
  lcd.clear();
  lcd.write(5);
  lcd.print(menu[menuPos][0]);
  for (int x = 1; x < menuSize[menuPos] && x < numRows; x++) {
    lcd.setCursor(0,x);
    lcd.write(4);
    lcd.print(menu[menuPos][x]);
  } 
}

// NEEDS TO ADD THE ABLILITY TO EXECUTE SPECIFIC THINGS IN THE COMBO MENU through executeMacro(x)
// Highlight the current selection
// Blinks for 250 milliseconds and then renders the new menu
void menuSelect()
{
  lcd.setCursor(0,lcdRow);
  lcd.write(6); // Write a blink for a tenth of a second
  delay(250);
  switch (menuPos) {
    case 0:
      if (subPos == 0) { menuPos = 1; }
      else if (subPos == 1) { menuPos = 4; }
      subPos = 0; // Resets the position to the top of the new menu
      lcdRow = 0; // Resets the lcdRow because it is back at the top of the menu
      renderMenu();
      break;
    case 1:
      menuPos =  2;
      subPos = 0; // Resets the position to the top of the new menu
      lcdRow = 0; // Resets the lcdRow because it is back at the top of the menu
      renderMenu();
      break;
    case 2:
      if (subPos == 4) {
        menuPos = 3;
        subPos = 0; // Resets the position to the top of the new menu
        lcdRow = 0; // Resets the lcdRow because it is back at the top of the menu
        renderMenu();
      }
      break;
    case 3:
      menuPos = 4;
      subPos = 0; // Resets the position to the top of the new menu
      lcdRow = 0; // Resets the lcdRow because it is back at the top of the menu
      renderMenu();
      break;
  }
}

/*************************************/
/********* MACRO FUNCTIONS ***********/
/*************************************/
/* When it starts recording it       */
/* should have the max combo length  */
/* possible. The length can be edited*/
/* in the menu.                      */
/*************************************/

// Converts a buffer in b_combos to a buffer for sendBuffer
void bitBool_to_ByteArray (int buffer, int comboNumber) {
  byte currBuffer[12]; // The new buffer output by this function
  int offset = (comboNumber * 12 * maxLength) + (buffer * 12); // This gives me the index of the first element of the buffer I am looking for in b_Data
  for (int counter = 0; counter < 12; counter++) {   // Take the next 12 indexes and add them to currBuffer
    tempBuffer[counter] = b_combos[offset+counter];  // This increments and copies the buffer in b_Data to tempBuffer
  }
}

// Prints out the contents of the combo in b_combos
void testPrint()
{
  unsigned int anotherStartIndex;
  unsigned int startIndexNewOne;
  for (int x = 0; x <= 7; x++) { // Look through the first 8 elements and take the macro corresponding to the first HIGH element
    if (buffer[x]) { 
      for (int y = 0; y < 1200; y++)
       if (y%8 == 0) Serial.println(b_combos[y]);
       else Serial.print(b_combos[y]);
    }
  }
}

// Checks if the shift buttons was pressed
// Contains code to execute the macro
// UPDATED FOR BITBOOLBUT DOES NOT WORK!
void executeMacro()
{
  for (int x = 0; x <= 7; x++) { // Look through the first 8 elements and take the macro corresponding to the first HIGH element
     if (buffer[x]) { 
       for (int bufferIndex = 0; bufferIndex < comboLength[x]; bufferIndex++) { // Actual execution occurs here (this is the buffer loop)
         for (int i = 0; i < 12; i++) { // This constructs the buffer from b_combos
           buffer[i] = b_combos[(comboLength[x]*x)+(12*bufferIndex)+i];
         }
         sendBuffer(); // Send the buffer
         resetBuffer();
       }
       break;
     }
     else if (x == 7) { sendBuffer(); } // Prevents dual button macro saves
  }
}

// Begins the recording process
// This organization is weird, and should be worked on for later
void initRecord() 
{
  delay(1000);
  for (int x = 0; x <= 7; x++) {
    if (!digitalRead(52) && buffer[x]) { // If after 1 second the rec button is still pressed & a macroButton was assigned
      macroButton = x; // Set macroButton to x
      recFlag = HIGH; // Set the recording flag high to activate the recordBuffer() function in main
      comboLength[x] = maxLength; // Sets the comboLength var (the size of the combo) to the max length
      Serial.println("Recording in 5");
      delay(1100);
      Serial.println("Recording in 4");
      delay(1100);
      Serial.println("Recording in 3");
      delay(1100);
      Serial.println("Recording in 2");
      delay(1100);
      Serial.println("Recording in 1");
      delay(1100);
      Serial.println("Recording...");
      
      // Prevents the button the macro is being assigned to from being activated
      resetBuffer();
        
      // Break out of the for loop
      x = 8;
    }
  } 
}

// Records the current buffer to the specified button in combos[]
// Runs at the beginning of a buffer cycle
// UPDATED FOR BITBOOL AND WORKS!!!!!
void recordBuffer()
{
 // Save the buffer in the combos array
 if (recIteration < comboLength[macroButton]) { // As long as the iteration is less than the max length (update so it checks for each combo)
   for (int i = 0; i < 12; i++) {
     b_combos[(comboLength[macroButton]*macroButton)+(12*recIteration)+i] = buffer[i];
   }
   recIteration++;
 }
 else { // When done reset the values for next recording
   Serial.println("DONE RECORDING"); // PROTO
   delay(1000); // PROTO So that it can be seen on console
   recFlag = LOW;
   recIteration = 0;
 }
}

void loop()
{
  createBuffer(); // Creates the buffer for this iteration of loop()
  
  consoleCheck(); // Checks the mode of the stick, setting currMode: HIGH for console mode, or LOW for PS3 mode
  if (currMode) { // If the console is activated (currMode is HIGH), send all data to the console
    Serial.print("C "); 
    printBuffer();
    if (buffer[9] && lcdRow > -1 && subPos > 0) menuUp(); // Up
    else if (buffer[8] && lcdRow <= numRows && subPos < menuSize[menuPos] - 1) menuDown(); // Down (menuPos < sizeOfMenu-1)
    else if (buffer[0]) menuSelect(); // Select (X button)
    else if (buffer[1]) menuBack(); // Back (Circle button)
    resetBuffer(); // This may be optional
  }
  else { // If not in console mode (currMode is LOW), send all data to the PS3
    if (recFlag) { recordBuffer(); }  // If it is recording
    else if (buffer[14]) { initRecord(); }  // If the rec button was pressed
    if (buffer[15]) { // If the shift button was pressed
      executeMacro(); // Checks if a valid button was pressed and executes the appropriate macro
      //testPrint();
    }
    else sendBuffer(); // If nothing happened, just send it out
  }
  resetBuffer();
}
