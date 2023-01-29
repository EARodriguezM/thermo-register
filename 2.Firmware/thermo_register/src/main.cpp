/**************************************************************************************/
#pragma region Import Libraries

  #include <Arduino.h>
  #include <U8x8lib.h>

  #include <Wire.h>               // I2C Comunication
  #include <SPI.h>               // Comunnication With SPI Devices

  #include "SdFat.h"             // SD Card Library

  #include <Bounce2.h>           // Debounce Library

#pragma endregion

/**************************************************************************************/
#pragma region Define OLED Parameters

  // Initialize the OLED Display 
  U8X8_SSD1306_128X64_NONAME_SW_I2C display(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display


  #define OLEDWidth                      display.getCols()                                              // OLED Width
  #define ALIGN_CENTER_WIDTH(text)       ((OLEDWidth - display.getUTF8Len(text)) / 2)  // Center Align
  #define ALIGN_RIGHT(text)              (OLEDWidth - display.getUTF8Len(text))         // Right Align
  #define ALIGN_LEFT                     0                                                 // Left Align

  #define OLEDHeight                     display.getRows()                                                // OLED Height
  #define ALIGN_CENTER_HEIGHT(number)     ((OLEDHeight - (number)) / 2)              // Center Align
  #define ALIGN_BOTTOM(number)            (OLEDHeight -  (number))                     // Right Align
  #define ALIGN_TOP                      0                                                 // Left Align


#pragma endregion

/**************************************************************************************/
#pragma region Define MAX6675 Pins and Variables for values

  #define SO 2             // MAX6675 - MISO (Master Input, Slave Output)
  #define SCLK 3           // MAX6675 - SCLK (Serial Clock line) 
  #define CS1 A1           // MAX6675 - CS1 (Chip Select 1)
  #define CS2 A2           // MAX6675 - CS2 (Chip Select 2)
  #define CS3 A3           // MAX6675 - CS3 (Chip Select 3)

  double valueSensor1 = 0;       // Value of sensor 1
  double valueSensor2 = 0;       // Value of sensor 2
  double valueSensor3 = 0;       // Value of sensor 3

#pragma endregion

/**************************************************************************************/
#pragma region Define IR Sensor Pin and variable for value

  #define IR A0            // IR Sensor Pin

  int valueSensorIR = 0;        // Value of IR Sensor

  int maxTemperatureIR = 1300;    // Max temperature of IR Sensor

#pragma endregion

/**************************************************************************************/
#pragma region Define SD Card pins and Parameters

  SdFat SD;                    // Initialize SD Card

  #define sdMOSI 11            // SD Card MOSI Pin
  #define sdMISO 12            // SD Card MISO Pin
  #define sdSCK 13             // SD Card SCK Pin
  #define sdCS 10              // SD Card CS Pin

  File file;                   // File to write

  String fileName;             // Name of the file

#pragma endregion

/**************************************************************************************/
#pragma region Define Button Pin

  Bounce button = Bounce();    // Debounce object

  #define BUTTON_PIN 5         // Button Pin
  #define DEBOUNCE_INTERVAL 20 // Debounce interval

#pragma endregion

/**************************************************************************************/
#pragma region Prototype functions

  void getCursorPos(int *x, int *y);
  void displayInit();
  void msgInit();
  void sdInit();
  void readData();
  int readIR();
  double readThermocouple(int ChipSelect);
  byte SPIRead(void);
  void displayingData();
  String createFile();
  void displayRecordLogo();
  void storeData();

#pragma endregion


unsigned long miliMessure;  // Time of the last measurement
double time;

int arduinoResolution = 1023; // Arduino UNO Resolution

bool recording = false;          // Recording state

char buff[100];             // Buffer to write in the file

void setup() 
{

  // MAX6675 mode of operation statement
  pinMode(SO, INPUT);
  pinMode(SCLK, OUTPUT);
  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);
  pinMode(CS3, OUTPUT);

  // MAX6675 Initial state
  digitalWrite(CS1, HIGH);
  digitalWrite(CS2, HIGH);
  digitalWrite(CS3, HIGH);

  //Display initialization
  displayInit();
  msgInit();
  delay(3000);

  // SD Card initialization
  sdInit();
  delay(2000);

  // Button initialization
  button.attach(BUTTON_PIN, INPUT_PULLUP); // Attach the button to the pin
  button.interval(DEBOUNCE_INTERVAL);      // Set the debounce interval
}
 
 
void loop() {

  button.update(); // Update the button state

  if (button.fell()) { // If the button is pressed
    recording = !recording; // Change the recording state

    if (recording){
      fileName = createFile();
      miliMessure = millis(); //start the timer of the measures
    }

    if (!recording){
      miliMessure = 0; //stop and reset the timer
      file.close();
    }
  }

  readData();

  displayingData();
  if (recording){
    displayRecordLogo();
    storeData();
  }
 
  delay(250);     // Delay 250ms
}

/**************************************************************************************/
// Funtion for display initialization and initial screen
/**************************************************************************************/
void displayInit(){

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin()) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setBusClock(2000);  // Set I2C clock to 2KHz

  // display.setFont(u8g2_font_ncenB10_tr);    f          // Set the font
  display.setFont(u8x8_font_chroma48medium8_r);       // Set the font

}

/**************************************************************************************/
// Funtion for display initialization and initial screen
/**************************************************************************************/
void msgInit(){

  byte centerHeight = ALIGN_CENTER_HEIGHT(3);
  display.setCursor(ALIGN_CENTER_WIDTH("Comb. Reaction"), centerHeight);
  display.print(F("Comb. Reaction"));          // print a sting (PROGMEM) via RAM array.
  centerHeight++;
  display.setCursor(ALIGN_CENTER_WIDTH("Sensor T"), centerHeight);
  display.print(F("Sensor T"));          // print a sting (PROGMEM) via RAM array.
  centerHeight++;
  display.setCursor(ALIGN_CENTER_WIDTH("UNAL"), centerHeight);
  display.print(F("UNAL"));          // print a sting (PROGMEM) via RAM array.

}

/**************************************************************************************/
// Funtion for SD Card initialization
/**************************************************************************************/
void sdInit(){

  // Get the cursor position
  // int x,y;
  // getCursorPos(&x,&y);

  // SD Card initialization
  display.clearDisplay();
  byte centerHeight = ALIGN_CENTER_HEIGHT(3);

  display.setCursor(ALIGN_CENTER_WIDTH("Init SDcard..."), centerHeight);
  display.println(F("Init SDcard..."));
  centerHeight=+2;

  if (!SD.begin(sdCS, SPI_HALF_SPEED)) {
    delay(1000);
    display.setCursor(ALIGN_CENTER_WIDTH("SD init failed!"), centerHeight);
    display.println(F("SD init failed!"));
    return;
  }

  delay(1000);
  display.setCursor(ALIGN_CENTER_WIDTH("SD init done"), centerHeight);
  display.println(F("SD init done"));
  delay(3000);

}

/**************************************************************************************/
// Funtion for read Data from sensors
/**************************************************************************************/
void readData(){
  valueSensorIR = readIR();

  valueSensor1 = readThermocouple(CS2);
  valueSensor2 = readThermocouple(CS1);
  valueSensor3 = readThermocouple(CS3);
}

/**************************************************************************************/
// Function for reading data from IR witout MAX6675
/**************************************************************************************/
int readIR()
{
  int valueIR = analogRead(IR);

  // Mapping to adjust the 1023 bits of the Arduino UNO to the range of the sensor. 
  valueIR = map(valueIR, 0, arduinoResolution, 0, maxTemperatureIR);

  return valueIR;
}

/**************************************************************************************/
// Function for reading data from MAX6675
/**************************************************************************************/
double readThermocouple(int ChipSelect) {
  uint16_t v;          // Declaring a 16-bit variable - MAX optimal size 
 
  digitalWrite(ChipSelect, LOW);
  _delay_ms(1);
  v = SPIRead();       // Upper part readout - First 8 bits of data
  v <<= 8;             // Shifts 8 positions to the left
  v |= SPIRead();      // Reading the low end and the first 8 bits through an OR
  digitalWrite(ChipSelect, HIGH);
 
  // Checks bit 2 for thermocouple attached 
  if (v & 0x4) {
    return NAN;
  }
  v >>= 3;          // Discards the first 3 bits, where only bit 2 faces the status bit 3 to 14
  return v * 0.25;  // Multiplies to MAX resolution 0.25ºC
}
 
/**************************************************************************************/
// Function that implements an "SPI" via Software to read from the MAX6675
/**************************************************************************************/
byte SPIRead(void) {
  int i;
  byte d = 0;
 
  // FOR loop to catch 8 bits at a time
  for (i = 7; i >= 0; i--)    
  {
    digitalWrite(SCLK, LOW);  // Clock Falling Edge
    _delay_ms(1);             // Wait 1ms
    if (digitalRead(SO)) {    // Reads the MAX output data pin
      d |= (1 << i);          // Stores the read bits, 0 or 1, in the variable and shifts accordingly to fill the entire byte of data
    }                         
    digitalWrite(SCLK, HIGH); // Clock Rising Edge
    _delay_ms(1);             // Wait 1ms
  }

  return d;                   // Remove the read byte from the data read from the MAX
}

/**************************************************************************************/
// Function for displaying the reading data previously made
/**************************************************************************************/
void displayingData() {
  byte centerHeight = ALIGN_CENTER_HEIGHT(8);

  display.setCursor(0,centerHeight);
  display.print("TEMPERATURES");



  centerHeight=+2;
  display.clearLine(centerHeight);
  display.setCursor(0,centerHeight);
  display.print("IR: ");
  display.print(valueSensorIR);
  centerHeight++;
  display.clearLine(centerHeight);
  display.setCursor(0,centerHeight);
  display.print("T1: ");
  display.print(valueSensor1);
  centerHeight++;
  display.clearLine(centerHeight);
  display.setCursor(0,centerHeight);
  display.print("T2: ");
  display.print(valueSensor2);
  centerHeight++;
  display.clearLine(centerHeight);
  display.setCursor(0,centerHeight);
  display.print("T3: ");
  display.print(valueSensor3);
  centerHeight++;
  display.write(248);
}

/**************************************************************************************/
// Function for select the file name and create the file
/**************************************************************************************/
String createFile() {

  for (int i = 0; i < 100000; i++) {
    if (!SD.exists(String(i)+".csv")) {
      // file.open(String(i)+".csv", O_RDWR | O_CREAT);
      file = SD.open(String(i)+".csv", FILE_WRITE);     // Open the file for writing

      file.println(sprintf(buff, "TIME;SENSOR IR;SENSOR 1;SENSOR 2;SENSOR 3")); // Write the header of the file

      return String(i)+".csv";
    }
  } 
}

/**************************************************************************************/
// Function for display the record notificaction
/**************************************************************************************/
void displayRecordLogo() {

  display.setCursor(0,48);
  display.write(254);
  display.print("REC");
  display.display();

}

/**************************************************************************************/
// Function for storing the reading data previously made
/**************************************************************************************/
void storeData() {
  sprintf(buff, "%lu;%i%f;%f;%f", miliMessure, valueSensorIR, valueSensor1, valueSensor2, valueSensor3);
  file.println(buff);
  file.flush();
}
