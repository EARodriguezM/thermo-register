/**************************************************************************************/
#pragma region Import Libraries

  #include <Adafruit_GFX.h>      // General Adafruit Display Library
  #include <Adafruit_SSD1306.h>  // SSD13066 Monochrome OLED Library

  #include <Wire.h>               // I2C Comunication
  #include <SPI.h>               // Comunnication With SPI Devices

  #include "SdFat.h"             // SD Card Library

  #include <Bounce2.h>           // Debounce Library

#pragma endregion

/**************************************************************************************/
#pragma region Define OLED Parameters

  #define SCREEN_WIDTH 128      // Screen Width
  #define SCREEN_HEIGHT 64      // Screen Height
  #define OLED_RESET     4      // Pin Arduino - SDA

  // Initialize the OLED Display 
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#pragma endregion

/**************************************************************************************/
#pragma region Define MAX6675 Pins and Variables for values

  #define SO 2             // MAX6675 - MISO (Master Input, Slave Output)
  #define SCLK 3           // MAX6675 - SCLK (Serial Clock line) 
  #define CS1 A1           // MAX6675 - CS1 (Chip Select 1)
  #define CS2 A2           // MAX6675 - CS2 (Chip Select 2)
  #define CS3 A3           // MAX6675 - CS3 (Chip Select 3)
  #define CS4 A4           // MAX6675 - CS4 (Chip Select 4)

  double valueSensor1 = 0;       // Value of sensor 1
  double valueSensor2 = 0;       // Value of sensor 2
  double valueSensor3 = 0;       // Value of sensor 3
  double valueSensor4 = 0;       // Value of sensor 4

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

/**************************************************************************************/

unsigned long miliMessure;  // Time of the last measurement

int arduinoUNOresolution = 1023; // Arduino UNO Resolution

bool recording = false;          // Recording state

char buff[100] = "";             // Buffer to write in the file

void setup() 
{

  Serial.begin(9600);

  // MAX6675 mode of operation statement
  pinMode(SO, INPUT);
  pinMode(SCLK, OUTPUT);
  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);
  pinMode(CS3, OUTPUT);
  pinMode(CS4, OUTPUT);
  
  // MAX6675 Initial state
  digitalWrite(CS1, HIGH);
  digitalWrite(CS2, HIGH);
  digitalWrite(CS3, HIGH);
  digitalWrite(CS4, HIGH);
  
  //Display initialization
  displayInit();
  delay(2000);
  
  // SD Card initialization
  sdInit();
  delay(1000);

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

  Serial.print(valueSensor1);
  Serial.print(";");  
  Serial.print(valueSensor2);
  Serial.print(";");  
  Serial.print(valueSensorIR);
  Serial.print(";"); 
  Serial.println(valueSensor3);

  displayingData();
  if (recording){
    displayRecordLogo();
    storeData();
  }
 
  delay(250);     // Aguarda 250 ms para uma nova leitura e armazenamento no cartão
}

/**************************************************************************************/
// Funtion for return x,y cursor position
/**************************************************************************************/
void getCursorPos(int *x, int *y){
  *x = display.getCursorX();
  *y = display.getCursorY();
}

/**************************************************************************************/
// Funtion for display initialization and initial screen
/**************************************************************************************/
void displayInit(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize the OLED Display
  
  display.setTextColor(SSD1306_WHITE);        // Set the color of the text
  display.clearDisplay();                     // Clear the display

  display.setCursor(0,0);                     // Set the cursor to the top left corner
  display.setTextSize(1.5);                   // Set the size of the text

  display.println(" Combustion reaction ");
  display.println("Sensor T");
  display.println("LexUnal");

  display.write(248);                         // Write the degree symbol (or 176)
  display.display();
}

/**************************************************************************************/
// Funtion for SD Card initialization
/**************************************************************************************/
void sdInit(){

  // Get the cursor position
  int x,y;
  getCursorPos(&x,&y);

  // SD Card initialization
  display.println("Initializing SD card...");
  display.display();

  display.setCursor(x,y);

  if (!SD.begin(sdCS, SPI_HALF_SPEED)) {
    display.println("SD Card initialization failed!");
    return;
  }
  display.println("SD Card initialization done.");
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
  valueIR = map(valueIR, 0, arduinoUNOresolution, 0, maxTemperatureIR);

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
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setCursor(10,5);
  display.print("T1: ");
  display.print(valueSensor1);
  display.setCursor(10,25);
  display.print("T2: ");
  display.print(valueSensor2);
  display.setCursor(10,45);
  display.print("IR: ");
  display.print(valueSensorIR);
  display.setCursor(10,55);
  display.print("T3: ");
  display.print(valueSensor3);
  display.write(248);
  display.display();  
}

/**************************************************************************************/
// Function for select the file name and create the file
/**************************************************************************************/
String createFile() {

  for (int i = 0; i < 100000; i++) {
    if (!SD.exists(String(i)+".csv")) {
      // file.open(String(i)+".csv", O_RDWR | O_CREAT);
      file = SD.open(String(i)+".csv", FILE_WRITE);     // Open the file for writing

      file.println(sprintf(buff, "TIME;SENSOR 1;SENSOR 2; SENSOR IR; SENSOR 3")); // Write the header of the file

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
  char buff[32];
  file.println(sprintf(buff, "%lu,%f;%f;%i;%f", miliMessure,valueSensor1, valueSensor2, valueSensorIR, valueSensor3));
  file.flush();
}
