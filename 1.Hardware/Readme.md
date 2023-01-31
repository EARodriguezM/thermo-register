# Hardware

<!-- <i>In this folder you will find the design files (Schematics, PCB files) and production files (Gerber folder) for the fabrication of a PCB that allows the assembly of the different components used in the system.

On the other hand, below is the list of the components used in the system, as well as the connection between the pins of the components/microcontroller divided into three sectors.</i>

> **Note:** The BOM.csv file contains the complete description of the components used to manufacture the PCB. The BOM.csv file contains the complete description of the components used to manufacture the PCB, where you will see that the bill of materials is different from the one below, this is because the modules are assembled on the PCB by means of headers connectors. For more details, check the schematic file. -->

In this folder you will find the design files (Schematics, Pictorial) and the connection between the pins of the components/microcontroller divided into three sectors.

### Components

The components listed here are for a BreadBoard assembly as shown in the [image](./1.%20Pictorial%20diagramcircuit.png)1. Pictorial diagram circuit.png) on the repository home page.

- Arduino UNO
- 3 MAX6675 module
- HW-685
- SSD1306 OLED 128sx64
- microSD module
- Push button
- 10K Ohm Resistor
- IR sensor HL600-400A

### Pin Connections

The connections described here are for a BreadBoard assembly.

#### K-type termocouples

This system area have two parts for the pin connections.

> The K-type termocouples are connected to the MAX6675 module how is indicated in the table:

| Termocouple | MAX6675 |
| :----:  | :----:  |
| Red crimp spade | + pin |
| Blue crimp spade | - pin |

> The tabble below indicated the shared connection between the Arduino UNO pins and the MAX6675 modules. Additionally, the connection of the CS pin of each module to its respective pin of the microcontroller.

| MAX6675 | ARDUINO UNO ||
| :----: | :----: | :----: |
| GND | GND | *Shared* |
| VCC | 3V3 | *Shared* |
| SCK | 3 | *Shared* |
| SO | 2 | *Shared* |
| CS thermocouple 1 | A1 | *One to One* |
| CS thermocouple 2 | A2 | *One to One* |
| CS thermocouple 3 | A3 | *One to One* |

#### Infrared temperature sensor

> The IR sensor has connections to multiple devices, be careful with the device column.

| IR SENSOR | PIN | DEVICE |
| :----: | :----: | :----: |
| Black wire | -terminal | Supply power |
| Red wire | + terminal | Supply power |
| Blue wire | I+ | HW-685 module|
| Orange wire | I- | HW-685 module |


> The HW-685 module has connections to multiple devices, be careful with the device column.

| HW-685 | PIN | DEVICE |
| :----: | :----: | :----: |
| GND | -terminal | Supply power |
| VCC | + terminal | Supply power |
| VOUT | A0 | Arduino UNO|

#### Real data display and micro-SD data record

| SSD1306 OLED 128sx64 | Arduino UNO |
| :----:  | :----:  |
| GND | GND |
| VCC | 3V3 |
| SCL | A5 |
| SDA | A4 |

| micro-SD module | Arduino UNO |
| :----:  | :----:  |
| GND | GND |
| VCC | 5V5 |
| MISO | 12 |
| MOSI | 11 |
| SCK | 13 |
| CS | 10 |

> The push button features a 10k Ohm resistor on the power line.

| Push Button | Arduino UNO |
| :----:  | :----:  |
| 1 | GND |
| 2 | 5 |
| 3 | VCC |
| 4 |  |