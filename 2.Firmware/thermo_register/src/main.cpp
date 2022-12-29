#pragma region Import Libraries

  #include <Adafruit_GFX.h>      // General Adafruit Display Library
  #include <Adafruit_SSD1306.h>  // SSD13066 Monochrome OLED Library

  #include<Wire.h>               // I2C Comunication
  #include <SPI.h>               // Comunnication With SPI Devices

#pragma endregion

#pragma region Define OLED Parameters

  #define SCREEN_WIDTH 128      // Screen Width
  #define SCREEN_HEIGHT 64      // Screen Height
  #define OLED_RESET     4      // Pin Arduino - SDA

  // Initialize the OLED Display 
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#pragma endregion

#pragma region Define MAX6675 Pins and Variables for values

  const int SO = 2;             // MAX6675 - MISO (Master Input, Slave Output)
  const int SCLK = 3;           // MAX6675 - SCLK (Serial Clock line) 
  const int CS1 = A1;           // MAX6675 - CS1 (Chip Select 1)
  const int CS2 = A2;           // MAX6675 - CS2 (Chip Select 2)
  const int CS3 = A3;           // MAX6675 - CS3 (Chip Select 3)
  const int CS4 = A4;           // MAX6675 - CS4 (Chip Select 4)

  double valueSensor1 = 0;       // Value of sensor 1
  double valueSensor2 = 0;       // Value of sensor 2
  double valueSensor3 = 0;       // Value of sensor 3
  double valueSensor4 = 0;       // Value of sensor 4

#pragma endregion

#pragma region Define IR Sensor Pin and variable for value

  const int IR = A0;            // IR Sensor Pin

  int valueSensorIR = 0;        // Value of IR Sensor

#pragma endregion

// how many MAX6675's are connected
const int chipSelect = 10;

// GENERATES DEGREE SYMBOL 
// uint8_t degree[8]  = {140, 146, 146, 140, 128, 128, 128, 128};

#pragma region Prototype functions

  void displayConfig();
  int readIR();
  double readThermocouple(int ChipSelect);
  byte SPIRead(void);
  void displayingData();
  void storeData();

#pragma endregion


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
  
  //Display configuration
  displayConfig();

  delay(1000);
}
 
 
void loop() {

  valueSensorIR = readIR();

  valueSensor1 = readThermocouple(CS2);
  valueSensor2 = readThermocouple(CS1);
  valueSensor3 = readThermocouple(CS3);

  Serial.print(valueSensor1);
  Serial.print(";");  
  Serial.print(valueSensor2);
  Serial.print(";");  
  Serial.print(valueSensorIR);
  Serial.print(";"); 
  Serial.println(valueSensor3);

  displayingData();
 
  delay(250);     // Aguarda 250 ms para uma nova leitura e armazenamento no cartão
}

/**************************************************************************************/
// Funtion for display configuration and initial screen
/**************************************************************************************/
void displayConfig(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE); 
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1.5);
  display.println("  R. combustión ");
  display.println("Sensor T");
  display.println("LexUnal");
  display.write(2);
  display.display();

  delay(2000);

  display.clearDisplay();
  display.display();
}

/**************************************************************************************/
// Function for reading data from IR witout MAX6675
/**************************************************************************************/
int readIR()
{
  int valueIR = analogRead(IR);

  // Mapping to adjust the 1023 bits of the Arduino UNO to the range of the sensor. 
  valueIR = map(valueIR, 0, 1023, 0, 1300);

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
  display.write(247);
  display.display();  
}

/**************************************************************************************/
// Function for storing the reading data previously made
/**************************************************************************************/
void storeData() {

}
