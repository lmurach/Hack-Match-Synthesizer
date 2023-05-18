//---------------------------------------------------------
/*
 
 test.ino
 
 Program for writing to Newhaven Display's 160x128 Graphic Color OLED with SEPS525 controller.
 
 Pick one up today in the Newhaven Display shop!
 ------> http://www.newhavendisplay.com/nhd169160128asc3-p-9288.html
 
 This code is written for the Arduino Uno.

 Copyright (c) 2015 - Newhaven Display International, LLC.
 
 Newhaven Display invests time and resources providing this open source code,
 please support Newhaven Display by purchasing products from Newhaven Display!
 
  */
//---------------------------------------------------------

// The 8 bit data bus is connected to PORTD[7..0]

#include <SPI.h>

#define   SDI_PIN   11    // SDI (serial mode) signal connected to pin 11
#define   SCL_PIN   13    // SCL (serial mdoe) signal connected to pin 13
#define    RS_PIN    4    // RS signal connected to pin 4
#define    RW_PIN    9    // R/W (6800 mode) signal connected to pin 9
#define    WR_PIN    9    // /WR (8080 mode) signal connected to pin 9
#define     E_PIN   5    // E (6800 mode) signal connected to pin 10
#define    RD_PIN   5    // /RD (8080 mode) signal connected to pin 10
#define   RES_PIN   6    // /RES signal connected to pin 6
#define    CS_PIN   10    // /CS signal connected to pin 5
#define    PS_PIN   A0    // PS signal connected to pin A0
#define   CPU_PIN   A1    // CPU signal connected to pin A1
#define   LVL_DIR   A2    // DIR (direction control) signal of level shifter IC connected to pin A2
#define   LVL_OEN   A3    // /OE (output enable) signal of level shifter IC connected to pin A3

#define    BLUE  0x001F
#define  GREEN  0x07E0
#define   RED  0xF800
#define  WHITE  0xFFFF
#define  BLACK  0x0000

#define MAIN_RED 0xE800
#define SHINE 0xF555
#define SHADE 0x8014

/*********************************/
/****** INTERFACE SELECTION ******/
/*********************************/

const unsigned char interface = 2;    // 0 = 8-bit parallel (6800 mode) interface; 1 = 8-bit parallel (8080 mode) interface; 2 = 4-wire SPI interface

unsigned long boxcolors[2][4] = {
  // edge, main part, inside, decal
  {0x3F975, 0x3D36A, 0xD0CA, 0x3ED7C},
  {0xb488e2, 0x243b84, 0x08054e, 0xc8c6f8}
};

/*===============================*/
/*===============================*/
/*===============================*/


/*********************************/
/****** LOW LEVEL FUNCTIONS ******/
/************* START *************/
/*********************************/

void OLED_Command_160128RGB(unsigned char c)        // send command to OLED
{
    unsigned char i;
    unsigned char mask = 0x80;
    
    switch(interface)
    {
        case 0:   digitalWrite(CS_PIN, LOW);
            digitalWrite(RS_PIN, LOW);
            PORTD = c;
            digitalWrite(RW_PIN, LOW);
            digitalWrite(E_PIN, HIGH);
            digitalWrite(E_PIN, LOW);
            digitalWrite(CS_PIN, HIGH);
            break;
        case 1:   digitalWrite(CS_PIN, LOW);
            digitalWrite(RS_PIN, LOW);
            digitalWrite(WR_PIN, HIGH);
            PORTD = c;
            digitalWrite(WR_PIN, LOW);
            digitalWrite(WR_PIN, HIGH);
            digitalWrite(CS_PIN, HIGH);
            break;
        case 2:   digitalWrite(CS_PIN, LOW);
            digitalWrite(RS_PIN, LOW);
            for(i=0;i<8;i++)
            {
                digitalWrite(SCL_PIN, LOW);
                if((c & mask) >> 7 == 1)
                {
                    digitalWrite(SDI_PIN, HIGH);
                }
                else
                {
                    digitalWrite(SDI_PIN, LOW);
                }
                digitalWrite(SCL_PIN, HIGH);
                c = c << 1;
            }
            digitalWrite(CS_PIN, HIGH);
            break;
        default:  break;
    }
}

void OLED_Data_160128RGB(unsigned char d)        // send data to OLED
{
    unsigned char i;
    unsigned char mask = 0x80;
    
    switch(interface)
    {
        case 0:   digitalWrite(CS_PIN, LOW);
            digitalWrite(RS_PIN, HIGH);
            PORTD = d;
            digitalWrite(RW_PIN, LOW);
            digitalWrite(E_PIN, HIGH);
            digitalWrite(E_PIN, LOW);
            digitalWrite(CS_PIN, HIGH);
            break;
        case 1:   digitalWrite(CS_PIN, LOW);
            digitalWrite(RS_PIN, HIGH);
            digitalWrite(WR_PIN, HIGH);
            PORTD = d;
            digitalWrite(WR_PIN, LOW);
            digitalWrite(WR_PIN, HIGH);
            digitalWrite(CS_PIN, HIGH);
            break;
            break;
        case 2:   digitalWrite(CS_PIN, LOW);
            digitalWrite(RS_PIN, HIGH);
            for(i=0;i<8;i++)
            {
                digitalWrite(SCL_PIN, LOW);
                if((d & mask) >> 7 == 1)
                {
                    digitalWrite(SDI_PIN, HIGH);
                }
                else
                {
                    digitalWrite(SDI_PIN, LOW);
                }
                digitalWrite(SCL_PIN, HIGH);
                d = d << 1;
            }
            digitalWrite(CS_PIN, HIGH);
            break;
        default:  break;
    }
}

void OLED_SerialPixelData_160128RGB(unsigned char d)    // serial write for pixel data
{
    unsigned char i;
    unsigned char mask = 0x80;
    for(i=0;i<6;i++)
    {
        digitalWrite(SCL_PIN, LOW);
        if((d & mask) >> 7 == 1)
        {
            digitalWrite(SDI_PIN, HIGH);
        }
        else
        {
            digitalWrite(SDI_PIN, LOW);
        }
        digitalWrite(SCL_PIN, HIGH);
        d = d << 1;
    }
}

void OLED_RealSerialTime(unsigned long color) {
  digitalWrite(CS_PIN, LOW);
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, LSBFIRST, SPI_MODE0));
  SPI.transfer16((color)& 0xFFFF);
  SPI.endTransaction();
  SPI.end();
  //bitBang(color >> 16);
  digitalWrite(CS_PIN, HIGH);
}

void bitBang(byte lastFew) {
  digitalWrite(SCL_PIN, HIGH);
  digitalWrite(SCL_PIN, LOW);
  digitalWrite(SDI_PIN, lastFew & 1);
  
  digitalWrite(SCL_PIN, HIGH);
  digitalWrite(SCL_PIN, LOW);
  digitalWrite(SDI_PIN, lastFew & 2);
}

void OLED_Pixel_160128RGB(unsigned long color)    // write one pixel of a given color
{
    switch(interface)
    {
        case 0:
            OLED_Data_160128RGB((color>>16));
            OLED_Data_160128RGB((color>>8));
            OLED_Data_160128RGB(color);
            break;
        case 1:
            OLED_Data_160128RGB((color>>16));
            OLED_Data_160128RGB((color>>8));
            OLED_Data_160128RGB(color);
            break;
        case 2:
            digitalWrite(RS_PIN, HIGH);
            digitalWrite(CS_PIN, LOW);
            OLED_SerialPixelData_160128RGB((color>>16));
            OLED_SerialPixelData_160128RGB((color>>8));
            OLED_SerialPixelData_160128RGB(color);
            digitalWrite(CS_PIN, HIGH);
            break;
        default:
            break;
    }
}

void OLED_SetPosition_160128RGB(unsigned char x_pos, unsigned char y_pos)    // set x,y address
{
    OLED_Command_160128RGB(0x20);
    OLED_Data_160128RGB(x_pos);
    OLED_Command_160128RGB(0x21);
    OLED_Data_160128RGB(y_pos);
}

void OLED_FillScreen_160128RGB(unsigned long color)    // fill screen with a given color
{
    unsigned int i;
    OLED_SetPosition_160128RGB(0,0);
    OLED_WriteMemoryStart_160128RGB();
    digitalWrite(RS_PIN, HIGH);
    for(i=0;i<((160) * (128));i++)
    {
        //OLED_Pixel_160128RGB(color);
        OLED_RealSerialTime(color);
    }
}

void OLED_Rectangle(unsigned long color, int width, int height, int x, int y)    // fill screen with a given color
{    
  for(int i = 0; i < height; i++) {
    OLED_SetPosition_160128RGB(x,y + i);
    OLED_WriteMemoryStart_160128RGB();
    digitalWrite(RS_PIN, HIGH);
    for(int k = 0; k < width; k++) {
      //OLED_Pixel_160128RGB(color);
      OLED_RealSerialTime(color);
    }
  }   
}

void OLED_Parallelogram(unsigned long color, int width, int height, int x, int y, int slant)    // fill screen with a given color
{   
  int offset = 0; 
  for(int i = 0; i < height; i++) {
    if (i % slant == 0) {
      offset += 1;
    }
    OLED_SetPosition_160128RGB(x + offset, y + i);
    OLED_WriteMemoryStart_160128RGB();
    digitalWrite(RS_PIN, HIGH);
    for(int k = 0; k < width; k++) {
      //OLED_Pixel_160128RGB(color);
      OLED_RealSerialTime(color);
    }
  }   
}

void OLED_SetColumnAddress_160128RGB(unsigned char x_start, unsigned char x_end)    // set column address start + end
{
    OLED_Command_160128RGB(0x17);
    OLED_Data_160128RGB(x_start);
    OLED_Command_160128RGB(0x18);
    OLED_Data_160128RGB(x_end);
}

void OLED_SetRowAddress_160128RGB(unsigned char y_start, unsigned char y_end)    // set row address start + end
{
    OLED_Command_160128RGB(0x19);
    OLED_Data_160128RGB(y_start);
    OLED_Command_160128RGB(0x1A);
    OLED_Data_160128RGB(y_end);
}

void OLED_WriteMemoryStart_160128RGB(void)    // write to RAM command
{
    OLED_Command_160128RGB(0x22);
}

/*===============================*/
/*===== LOW LEVEL FUNCTIONS =====*/
/*============= END =============*/
/*===============================*/


/*********************************/
/****** HIGH LEVEL FUNCTIONS *****/
/************* START *************/
/*********************************/

//void drawPlayBG() {
//  // light gray borders
//  OLED_Rectangle(LIGHT_GRAY, 160, 3, 0, 125);
//  OLED_Rectangle(LIGHT_GRAY, 79, 24, 81, 101);
//  OLED_Rectangle(LIGHT_GRAY, 3, 122, 0, 3);
//  OLED_Rectangle(LIGHT_GRAY, 5, 93, 81, 8);
//  OLED_Rectangle(LIGHT_GRAY, 3, 93, 157, 8);
//  OLED_Rectangle(LIGHT_GRAY, 79, 15, 81, 3);
//  OLED_Rectangle(LIGHT_GRAY, 160, 3, 0, 0);
//  // inside play area
//  OLED_Rectangle(DARK_CYAN, 78, 122, 3, 3);
//  // inside score area
//  OLED_Rectangle(BLACK, 71, 83, 86, 18);
//}
//
//void drawBox(int index, int x_offset, int y_offset) {
//  OLED_Rectangle(boxcolors[index][1], 1, 13, 0 + x_offset, 0 + y_offset); // side bar shine
//  OLED_Rectangle(boxcolors[index][1], 7, 1, 3 + x_offset, 2 + y_offset); // bottom shine
//  OLED_Rectangle(boxcolors[index][1], 8, 1, 10 + x_offset, 2 + y_offset); //right shine
//  
//  OLED_Rectangle(boxcolors[index][2], 12, 3, 1 + x_offset, 10 + y_offset); // top main part
//  OLED_Rectangle(boxcolors[index][2], 2, 10, 1 + x_offset, 0 + y_offset); // left main part
//  OLED_Rectangle(boxcolors[index][2], 10, 2, 3 + x_offset, 0 + y_offset); // bottom main part
//  OLED_Rectangle(boxcolors[index][2], 2, 8, 11 + x_offset, 2 + y_offset); // side main part
//
//  OLED_Rectangle(boxcolors[index][3], 7, 7, 3 + x_offset, 3 + y_offset);
//}

//void drawRow() {
//  int x_offset = 3;
//  int y_offset = 112;
//  for (int i = 0; i < 6; i++) {
//    drawBox(0, x_offset, y_offset);
//    x_offset += 13;
//  }
//}

void drawLogo() {
  // H
  drawLogoH(1, 76);

  // A
  drawLogoA(25, 76);

  // C
  drawLogoC(47, 76);

  // K
  drawLogoK(65, 76);

  // M
  drawLogoM(38, 45);

  // A
  drawLogoA(69, 45);

  //T
  drawLogoT(92, 45);

  // C
  drawLogoC(106, 45);

  // H
  drawLogoH(124, 45);

  OLED_Rectangle(MAIN_RED, 109, 2, 36, 40); // bar
  OLED_Rectangle(MAIN_RED, 109, 2, 34, 36); // bar2
}

void drawLogoH(int x_offset, int y_offset) {
  OLED_Parallelogram(MAIN_RED, 8, 28, 0 + x_offset, 0 + y_offset, 2); // side
  OLED_Parallelogram(MAIN_RED, 9, 4, 12 + x_offset, 8 + y_offset, 2); // middle
  OLED_Parallelogram(MAIN_RED, 8, 28, 14 + x_offset, 0 + y_offset, 2); // side
}

void drawLogoA(int x_offset, int y_offset) {
  OLED_Parallelogram(MAIN_RED, 8, 28, 0 + x_offset, 0 + y_offset, 2); // side
  OLED_Parallelogram(MAIN_RED, 8, 28, 12 + x_offset, 0 + y_offset, 2); // side
  OLED_Parallelogram(MAIN_RED, 9, 4, 11 + x_offset, 8 + y_offset, 2); // middle
  OLED_Parallelogram(MAIN_RED, 9, 4, 18 + x_offset, 24 + y_offset, 2); // top
  OLED_Rectangle(SHADE, 10, 29, 26 + x_offset, 0 + y_offset); // erase
}

void drawLogoC(int x_offset, int y_offset) {
  OLED_Parallelogram(MAIN_RED, 8, 28, 0 + x_offset, 0 + y_offset, 2); //side
  OLED_Parallelogram(MAIN_RED, 8, 4, 17 + x_offset, 24 + y_offset, 2); // bottom
  OLED_Parallelogram(MAIN_RED, 8, 4, 8 + x_offset, 0 + y_offset, 2); // top
}

void drawLogoK(int x_offset, int y_offset) {
  OLED_Parallelogram(MAIN_RED, 8, 28, 0 + x_offset, 0 + y_offset, 2); //side
  OLED_Parallelogram(MAIN_RED, 8, 28, 11 + x_offset, 0 + y_offset, 2); //side
  OLED_Parallelogram(MAIN_RED, 9, 4, 12 + x_offset, 8 + y_offset, 2); // middle
  OLED_Rectangle(SHADE, 8, 8, 84, 0 + y_offset); // erase
  OLED_Parallelogram(SHADE, 8, 8, 19 + x_offset, 8 + y_offset, 1); //side
}

void drawLogoM(int x_offset, int y_offset) {
  OLED_Parallelogram(MAIN_RED, 8, 28, 0 + x_offset, 0 + y_offset, 2); //side
  OLED_Parallelogram(MAIN_RED, 8, 28, 10 + x_offset, 0 + y_offset, 2); //side
  OLED_Parallelogram(MAIN_RED, 8, 28, 20 + x_offset, 0 + y_offset, 2); //side
  OLED_Parallelogram(MAIN_RED, 23, 7, 12 + x_offset, 21 + y_offset, 2); // top
  OLED_Rectangle(MAIN_RED, 4, 12, 30 + x_offset, 60); // right extra side
  OLED_Rectangle(SHADE, 10, 19, 33 + x_offset, 10 + y_offset); // erase
}

void drawLogoT(int x_offset, int y_offset) {
  OLED_Parallelogram(MAIN_RED, 8, 28, 0 + x_offset, 0 + y_offset, 2); //side
  OLED_Parallelogram(MAIN_RED, 20, 4, 4 + x_offset, 24 + y_offset, 2); //side
}

/*===============================*/
/*==== HIGH LEVEL FUNCTIONS =====*/
/*============= END =============*/
/*===============================*/

/*********************************/
/******** INITIALIZATION *********/
/************ START **************/
/*********************************/

void OLED_Init_160128RGB(void)      //OLED initialization
{
    digitalWrite(RES_PIN, LOW);
    delay(2);
    digitalWrite(RES_PIN, HIGH);
    delay(2);
    
    // display off, analog reset
    OLED_Command_160128RGB(0x04);
    OLED_Data_160128RGB(0x01);
    delay(1);
    
     // normal mode
    OLED_Command_160128RGB(0x04); 
    OLED_Data_160128RGB(0x00); 
    delay(1);
    
    // display off
    OLED_Command_160128RGB(0x06);
    OLED_Data_160128RGB(0x00);
    delay(1);
    
    // turn on internal oscillator using external resistor
    OLED_Command_160128RGB(0x02);
    OLED_Data_160128RGB(0x01); 
    
    // 90 hz frame rate, divider 0
    OLED_Command_160128RGB(0x03);
    OLED_Data_160128RGB(0x30); 
    
    // duty cycle 127
    OLED_Command_160128RGB(0x28);
    OLED_Data_160128RGB(0x7F);
    
    // start on line 0
    OLED_Command_160128RGB(0x29);
    OLED_Data_160128RGB(0x00); 
    
    // rgb_if
    OLED_Command_160128RGB(0x14);
    OLED_Data_160128RGB(0x31); 
    
    // Set Memory Write Mode
    OLED_Command_160128RGB(0x16);
    OLED_Data_160128RGB(0x66);
    // TRI mode off to have 16 bits
    // red and blue have 5 bits, green has 6
    
    // driving current r g b (uA)
    OLED_Command_160128RGB(0x10);
    OLED_Data_160128RGB(0x45);
    OLED_Command_160128RGB(0x11);
    OLED_Data_160128RGB(0x34);
    OLED_Command_160128RGB(0x12);
    OLED_Data_160128RGB(0x33);
    
    // precharge time r g b
    OLED_Command_160128RGB(0x08);
    OLED_Data_160128RGB(0x04);
    OLED_Command_160128RGB(0x09);
    OLED_Data_160128RGB(0x05);
    OLED_Command_160128RGB(0x0A);
    OLED_Data_160128RGB(0x05);
    
    // precharge current r g b (uA)
    OLED_Command_160128RGB(0x0B);
    OLED_Data_160128RGB(0x9D);
    OLED_Command_160128RGB(0x0C);
    OLED_Data_160128RGB(0x8C);
    OLED_Command_160128RGB(0x0D);
    OLED_Data_160128RGB(0x57);
    
    // Set Reference Voltage Controlled by External Resister
    OLED_Command_160128RGB(0x80);
    OLED_Data_160128RGB(0x00);
    
    // mode set
    OLED_Command_160128RGB(0x13);
    OLED_Data_160128RGB(0xA0);
    
    OLED_SetColumnAddress_160128RGB(0, 159);
    OLED_SetRowAddress_160128RGB(0, 127);

    // Display On
    OLED_Command_160128RGB(0x06);
    OLED_Data_160128RGB(0x01); 
}


/*===============================*/
/*======= INITIALIZATION ========*/
/*============= END =============*/
/*===============================*/

void setup()                                       // for Arduino, runs first at power on
{   
    Serial.begin(9600);
    pinMode(3, OUTPUT);
    pinMode(LVL_OEN, OUTPUT);                       // configure LVL_OEN as output
    digitalWrite(LVL_OEN, LOW);
    pinMode(LVL_DIR, OUTPUT);                       // configure LVL_DIR as output
    digitalWrite(LVL_DIR, HIGH);
    DDRD = 0xFF;                                    // configure PORTD as output
    DDRB = 0b00101100;
    pinMode(RS_PIN, OUTPUT);                        // configure RS_PIN as output
    pinMode(RES_PIN, OUTPUT);                       // configure RES_PIN as output
    pinMode(CS_PIN, OUTPUT);                        // configure CS_PIN as output
    pinMode(PS_PIN, OUTPUT);                        // configure PS_PIN as output
    pinMode(CPU_PIN, OUTPUT);                       // configure CPU_PIN as output
    digitalWrite(LVL_OEN, LOW);
    digitalWrite(CS_PIN, HIGH);                     // set CS_PIN
    switch(interface)
    {
        case 0:
            pinMode(RW_PIN, OUTPUT);                    // configure RW_PIN as output
            pinMode(E_PIN, OUTPUT);                     // configure E_PIN as output
            digitalWrite(PS_PIN, HIGH);                 // set PS_PIN
            digitalWrite(CPU_PIN, HIGH);                // set CPU_PIN
            digitalWrite(RW_PIN, LOW);                  // reset RW_PIN
            digitalWrite(E_PIN, HIGH);                  // set E_PIN
            break;
        case 1:
            pinMode(WR_PIN, OUTPUT);                    // configure WR_PIN as output
            pinMode(RD_PIN, OUTPUT);                    // configure RD_PIN as output
            digitalWrite(PS_PIN, HIGH);                 // set PS_PIN
            digitalWrite(CPU_PIN, LOW);                 // reset CPU_PIN
            digitalWrite(WR_PIN, HIGH);                 // set WR_PIN
            digitalWrite(RD_PIN, HIGH);                 // set RD_PIN
            break;
        case 2:
            pinMode(SDI_PIN, OUTPUT);                   // configure SDI_PIN as output
            pinMode(SCL_PIN, OUTPUT);                   // configure SCL_PIN as output
            PORTD = 0x00;                               // reset SDI_PIN and SCL_PIN, ground DB[5..0] of the display
            digitalWrite(PS_PIN, LOW);                  // reset PS_PIN
            break;
        default:
            break;
    }
    OLED_Init_160128RGB();                           // initialize display
}

void loop()                                         // main loop, runs after "setup()"
{
    OLED_FillScreen_160128RGB(SHADE);
    drawLogo();
    while(1);
}