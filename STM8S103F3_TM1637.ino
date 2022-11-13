//STM8S103F3 board and TM1637 led without external lib
//C only. no C++ required.
//remember to add 10k pullup resistor for CLK and DIO if not already present on the board

#include <Arduino.h>
/*
    tm1637.c - A Propeller C libarary for writing to TM1637 displays
    Copyright (c) 2019 Jon McPhalen

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    This permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/


#include "tm1637.h"
void tm1637_dio_high()
{
  pinMode(tmdio, INPUT);
}

void tm1637_dio_low()
{
  digitalWrite(tmdio, LOW);
  pinMode(tmdio, OUTPUT);
}
void tm1637_clk_high()
{
  pinMode(tmclk, INPUT);
}

void tm1637_clk_low()
{
  digitalWrite(tmclk, LOW);
  pinMode(tmclk, OUTPUT);
}

uint8_t tm1637_setup(uint8_t cpin, uint8_t dpin, uint8_t ncols)
{

  //  cmask = 1 << cpin;                                            // create CLK pin mask
  //  dmask = 1 << dpin;                                            // create DIO pin mask



  if ((ncols >= 2) && (ncols <= 6))                             // validate # of columns
{
  tmsize = ncols;
}
  else
  {
    tmsize = 4;
  }
  
  tmclk = cpin;  
  tmdio = dpin;

  //DIRA &= ~cmask;                                               // set CLK as input
  //OUTA &= ~cmask;                                               // write 0 to CLK output bit
  //DIRA &= ~dmask;                                               // set DIO as input
  //OUTA &= ~dmask;                                               // write 0 to DIO output bit

//dio/clk supposed to be opencollector with a 10k pullup. on INPUT the line goes to high with the pullup
//on output the line goes low
  tm1637_clk_high();
  tm1637_dio_high();

  return tm1637_send_command(0);                                // check for device connected
}


void tm1637_clear()
{
  tm1637_fill(0);
}


void tm1637_fill(uint8_t segments)
{
  for (uint8_t i = 0; i < 6; i++)                               // copy segments to tmbuffer
    tmbuffer[i] = segments;

  tm1637_start();
  tm1637_write(WR_SEGS);                                        // write to segments, starting at column 0
  for (uint8_t i = 0; i < 6; i++)                               // write segments to all columns
    tm1637_write(segments);
  tm1637_stop();
}


void tm1637_display_on(uint8_t level)
{
  if (level > 8)                                                // validate level
    return;

  if (level == 0)                                               // if 0
    tm1637_send_command(DSP_OFF);                               //  off
  else                                                          // else
    tm1637_send_command(DSP_ON | (level - 1));                  //  on with brightness
}


void tm1637_set_segments(uint8_t col, uint8_t segments)
{
  if (col >= tmsize)                                              // validate column
    return;

  tmbuffer[col] = segments;                                       // save segments to tmbuffer

  tm1637_start();
  tm1637_write(WR_SEGS | col);                                  // write to specified column
  tm1637_write(segments);
  tm1637_stop();
}


void tm1637_set_digit(uint8_t col, uint8_t value, uint8_t dp)
{
  if (col >= tmsize)
    return;

  if (value >= 16)                                              // validate digit (hex or smaller)
    return;

  value = hexTable[value];                                      // convert to segments

  if (dp)
    value |= SEG_DP;                                            // add dp if specified

  tm1637_set_segments(col, value);                              // write to col
}


void tm1637_set_char(uint8_t col, char ch, uint8_t dp)
{
  if (col >= tmsize)
    return;

  if ((ch < 32) || (ch > 127))                                  // validate character
    return;

  ch = chrTable[ch - 32];                                       // convert to segments

  if (dp)
    ch |= SEG_DP;

  tm1637_set_segments(col, ch);
}


void tm1637_str(uint8_t col, char *str, uint8_t len)
{
  char    ch, la;
  uint8_t segs;

  if (col >= tmsize)
    return;

  if (len == 0) {                                               // auto-tmsize string
    len = strlen(str);
  }

  tm1637_send_command(WR_DATA);                                 // auto increment column
  tm1637_start();
  tm1637_write(WR_SEGS | col);                                  // set starting column

  while (len) {
    ch = *str;                                                  // get a character
    str++;                                                      // advance to next
    if (ch == '\0')                                             // if at end
      break;                                                    //  abort
    else {
      segs = chrTable[ch - 32];                                 // convert to segments
      if (ch != '.') {                                          // if not dp
        la = *str;                                              //  look ahead to next
        if (la == '.') {                                        // if next is dp
          segs |= SEG_DP;                                       //  add dp to character
          str++;                                                //  advance past trailing dp
        }
      }
      tm1637_write(segs);                                       // write to display
      if (++col == tmsize)                                        // check for end of display
        break;
      --len;
    }
  }

  tm1637_stop();
}


void tm1637_wr_buf(uint8_t col, uint8_t *xbuf, uint8_t len)
{
  if (col >= tmsize)
    return;

  if (len > (tmsize - col))                                       // validate length
    len = tmsize - col;

  tm1637_send_command(WR_DATA);

  tm1637_start();
  tm1637_write(WR_SEGS | col);
  for (uint8_t i = 0; i < len; i++) {
    tmbuffer[col] = *xbuf;                                        // copy to tmbuffer
    tm1637_write(tmbuffer[col++]);                                // write to display
    xbuf++;
  }
  tm1637_stop();
}


void tm1637_set_dpoint(uint8_t col, uint8_t state)
{
  if (col >= tmsize)
    return;

  uint8_t segs = tmbuffer[col];                                   // get segments for col

  if (state) {                                                  // enable dp
    if ((segs & ~SEG_DP) == 0)                                  //  off now?
      tm1637_set_segments(col, segs | SEG_DP);                  //  dp on
  }
  else {                                                        // disable dp
    if (segs & SEG_DP)                                          //  on now?
      tm1637_set_segments(col, segs & ~SEG_DP);                 //  dp off
  }
}


uint8_t tm1637_get_segs(uint8_t col)
{
  if (col > tmsize)
    return 0;

  return tmbuffer[col];
}


uint8_t tm1637_rd_button()
{
  uint8_t btn;

  tm1637_start();
  tm1637_write(RD_BTN);                                         // issue read command
  btn = tm1637_read();                                          // read button code
  tm1637_stop();

  if (btn == 0xFF)
    return 0;                                                   // no press
  else
    return (~btn & 0x1F) - 7;                                   // decode button
}


uint8_t tm1637_send_command(uint8_t cmd)
{
  tm1637_start();
  uint8_t ackbit = tm1637_write(cmd);
  tm1637_stop();

  return ackbit;
}


void tm1637_start()
{
  // DIRA &= ~cmask;                                               // float CLK pin
  // DIRA &= ~dmask;                                               // float DIO pin
  tm1637_clk_high();
  tm1637_dio_high();

  tm1637_bit_delay();
  //DIRA |= dmask;                                                // DIO low (0)
  tm1637_dio_low();

  tm1637_bit_delay();
  //  DIRA |= cmask;                                                // CLK low
  tm1637_clk_low();
  tm1637_bit_delay();
}




uint8_t tm1637_write(uint8_t b)
{
  uint8_t ackbit;

  for (uint8_t i = 0; i < 8; i++) {                             // write 8 bits
    if (b & 0x01 == 1)                                          // get lsb, if 1
    {
      //      DIRA &= ~dmask;                                           //  float DIO
      tm1637_dio_high();
    }
    else                                                        // else
    {
      //    DIRA |= dmask;                                            //  DIO low (0)
      tm1637_dio_low();
    }
    b >>= 1;
    //    DIRA &= ~cmask;//clk low
    tm1637_clk_high();
    tm1637_bit_delay();
    //   DIRA |= cmask;
    tm1637_clk_low();
    tm1637_bit_delay();
  }

  //DIRA &= ~dmask;                                               // DIO to input
  //DIRA &= ~cmask;                                               // CLK high
  tm1637_dio_high();
  tm1637_clk_high();

  tm1637_bit_delay();
  //  if (INA & dmask)                                              // get ACK bit
  if (digitalRead(tmdio) == HIGH)    {
    ackbit = 1;
  }
  else
  {
    ackbit = 0;
  }
  //DIRA |= cmask;                                                // CLK low
  tm1637_clk_low();
  tm1637_bit_delay();

  return ackbit;
}


uint8_t tm1637_read()
{
  uint8_t b = 0;
  uint8_t mask = 0x01;

  //DIRA &= ~dmask;                                               // DIO to input
  tm1637_dio_high();

  for (uint8_t i = 0; i < 8; i++) {                             // read 8 bits
    //DIRA &= ~cmask;                                             // CLK high
    tm1637_clk_high();
    tm1637_bit_delay();
    // if (INA & dmask)                                            // get bit (lsbfirst)
    if (digitalRead(tmdio) == HIGH)  b |= mask;
    mask <<= 1;                                                 // prep for next bit
    // DIRA |= cmask;                                              // CLK low
    tm1637_clk_low();
    tm1637_bit_delay();
  }

  //DIRA |= dmask;                                                // write ACK
  tm1637_dio_low();
  tm1637_bit_delay();
  //DIRA &= ~cmask;                                               // CLK high
  tm1637_clk_high();
  tm1637_bit_delay();
  //  DIRA |= cmask;
  tm1637_clk_low();
  tm1637_bit_delay();                                           // CLK low

  return b;
}


void tm1637_stop()
{

  //DIRA |= dmask;                                                // DIO low
  //DIRA |= cmask;                                                // CLK low
  tm1637_dio_low();
  tm1637_clk_low();
  tm1637_bit_delay();
  //DIRA &= ~cmask;                                               // CLK high
  tm1637_clk_high();
  tm1637_bit_delay();
  //DIRA &= ~dmask;                                               // DIO high
  tm1637_dio_high();
}


void tm1637_bit_delay()
{
  // placeholder only
  // -- no delay is required for CMM or LMM @ 80MHz
}






#define  NO        0
#define  YES       1

#define  tmCLKpin       12
#define  tmDIOpin       11

#define  tmWIDTH     4
#define  tmDPOINTS   YES   
#define  tmHAS_BTNS  NO
#define  tmCOLON     NO  


char message[] = "      HELLO      ";
char nstr[11];

uint8_t animate[] = { 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x63, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C };  


void scroll_text(char *str)
{
  for (;;) {
    if (*str == '\0') 
      return;
    else {
      tm1637_str(0, str, tmWIDTH);
      delay(125);
      str++; 
    }        
  } 
}  


void buttons_demo()
{
  uint8_t btn;
  
  if (tmHAS_BTNS != YES)
    return;
    
  tm1637_clear();
  
  if (tmWIDTH == 6)
    tm1637_str(0, "btn", 3);
    
  for (uint8_t i = 0; i < 100; i++) {
    btn = tm1637_rd_button();
    if (btn > 9)
      tm1637_set_digit(4, btn/10, 0);
    tm1637_set_digit(5, btn%10, 0);
    delay(50);
  } 
  
  tm1637_clear(); 
}


void buffer_demo(uint8_t *buf)
{
  for (uint8_t i = 0; i <= tmWIDTH; i++) {
    tm1637_wr_buf(0, buf, tmWIDTH);
    delay(125);
    buf++;       
  } 
}  


void setup() {
  //Initialize serial and wait for port to open:
  Serial_begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(LED_BUILTIN, OUTPUT);

  if (tm1637_setup(tmCLKpin, tmDIOpin, tmWIDTH) == 0) {
    tm1637_clear();
    tm1637_display_on(3);
    while (1) {
      scroll_text(message); 
      buttons_demo();
      buffer_demo(animate); 
    }        
  }    

}


void loop() {
}
