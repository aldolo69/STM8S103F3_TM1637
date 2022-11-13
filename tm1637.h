#ifndef TM1637_H
#define TM1637_H

#if defined(__cplusplus)
extern "C" {
#endif

//#include <propeller.h>
//#include <stdint.h>
//#include <string.h>


/*
 *  TM1637 commands
 */
#define  WR_DATA  0x40
#define  RD_BTN   0x42
#define  WR_FIXD  0x44
#define  DSP_OFF  0x80
#define  DSP_ON   0x88
#define  WR_SEGS  0xC0


/*
 *  Individual segment maps
 */
#define  SEG_A    0b00000001
#define  SEG_B    0b00000010
#define  SEG_C    0b00000100
#define  SEG_D    0b00001000
#define  SEG_E    0b00010000
#define  SEG_F    0b00100000
#define  SEG_G    0b01000000
#define  SEG_DP   0b10000000


/*
 *  Segment maps for digits up to base 16 (hex)
 */
const static uint8_t hexTable[] = {
  0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111,   // 0..7
  0b01111111, 0b01101111, 0b01110111, 0b01111100, 0b00111001, 0b01011110, 0b01111001, 0b01110001    // 8, 9, A, b, C, d, E, F
};


/*
 * Segment maps for ASCII characters
 * -- not all characters can be mapped to 7-segment display
 */

const static uint8_t chrTable[] = {
  0b00000000, 0b00000000, 0b00100010, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,   //  32.. 39
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01000000, 0b10000000, 0b00000000,   //  40.. 47
  0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111,   //  48.. 55
  0b01111111, 0b01101111, 0b00000000, 0b00000000, 0b00000000, 0b01001000, 0b00000000, 0b01010011,   //  56.. 63
  0b01111011, 0b01110111, 0b00000000, 0b00111001, 0b00000000, 0b01111001, 0b01110001, 0b01111101,   //  64.. 71
  0b01110110, 0b00000110, 0b00011110, 0b00000000, 0b00111000, 0b00000000, 0b00000000, 0b00111111,   //  72.. 79
  0b01110011, 0b00000000, 0b00000000, 0b01101101, 0b00000000, 0b00111110, 0b00000000, 0b00000000,   //  80.. 87
  0b00000000, 0b01101110, 0b01011011, 0b00111001, 0b00000000, 0b00001111, 0b00000000, 0b00001000,   //  88.. 95
  0b00000000, 0b00000000, 0b01111100, 0b01011000, 0b01011110, 0b00000000, 0b00000000, 0b00000000,   //  96..103
  0b01110100, 0b00000100, 0b00000000, 0b00000000, 0b00000110, 0b00000000, 0b01010100, 0b01011100,   // 104..111
  0b00000000, 0b00000000, 0b01010000, 0b00000000, 0b01111000, 0b00011100, 0b00000000, 0b00000000,   // 112..119
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000    // 120..127
};


//static uint32_t cmask;                                          // CLK pin mask
//static uint32_t dmask;                                          // DIO pin mask
static uint8_t  tmsize;                                           // size of display (characters)
static uint8_t  tmbuffer[6];                                      // display buffer
static uint8_t  tmclk;
static uint8_t  tmdio;


/*
 *  Connect to TM1637
 *  -- cpin is CLK pin
 *  -- dpin is DIO pin (use 4.7K series resistor if display connected to 5v)
 *  -- ncols is the number of display colums (1 to 6)
 *  -- returns 0 if TM1637 detected
 */
uint8_t tm1637_setup(uint8_t cpin, uint8_t dpin, uint8_t ncols);


/*
 *  Clear (all segments off) display buffer and display
 */
void tm1637_clear();


/*  Fill display buffer and display with segments
 *  -- segments arrangement is DP G F E D C B A
 */
void tm1637_fill(uint8_t segments);


/*  Enable/dispable display and set brightness
 *  -- level 0 turns off display
 *  -- level 1..8 enableds display and sets brightness
 */
void tm1637_display_on(uint8_t level);


/*
 *  Write segments to col in display
 *  -- col is position to write (0..size-1)
 *  -- segments is segment map to write
 */
void tm1637_set_segments(uint8_t col, uint8_t segments);


/*
 *  Write digit with optional decimal point to col in display
 *  -- col is position to write (0..size-1)
 *  -- value is digit to write, 0..9, 0xA..0xF
 *  -- dp enables decimal point
 */
void tm1637_set_digit(uint8_t col, uint8_t value, uint8_t dp);


/*
 *  Write ASCII character with optional decimal point to col in display
 *  -- col is position to write (0..size-1)
 *  -- ch is ASCII character to write (32..127)
 *  -- dp enables decimal point
 */
void tm1637_set_char(uint8_t col, char ch, uint8_t dp);


/*
 *  Write string to display
 *  -- col is the starting position (0..ncols-1)
 *  -- str is pointer to string
 *  -- len is the number of characters to write
 */
void tm1637_str(uint8_t col, char *str, uint8_t len);


/*
 *  Write segments buffer to display
 *  -- col is the starting position (0..ncols-1)
 *  -- xbuf is pointer to external buffer
 *  -- len is number of columns to write
 */
void tm1637_wr_buf(uint8_t col, uint8_t *xbuf, uint8_t len);


/*
 *  Set or clear decimal point
 *  -- col is the position (0..ncols-1)
 *  -- state is 1 (non-zero) to enable decimal point, 0 to clear it
 */
void tm1637_set_dpoint(uint8_t col, uint8_t state);


/*
 *  Return segments
 *  -- col is the position to read (0..ncols-1)
 */
uint8_t tm1637_get_segs(uint8_t col);


/*
 *  Return button input
 *  -- 0 indicates no button pressed
 *  -- 1..16 identifies button (only one may be pressed)
 */
uint8_t tm1637_rd_button();


/*
 *  Send command to TM1637
 *  -- returns 0 if successful
 */
uint8_t tm1637_send_command(uint8_t cmd);


/*
 *  Create TM1637 start sequence
 */
void     tm1637_start();


/*
 *  Write byte b to TM1637
 */
uint8_t tm1637_write(uint8_t b) ;


/*
 *  Read byte from TM1637
 */
uint8_t tm1637_read();


/*
 *  Create TM1637 stop sequence
 */
void tm1637_stop();


/*
 *  Create short delay for bit timing
 *  -- placeholder
 */
void tm1637_bit_delay();


#if defined(__cplusplus)
}
#endif
/* __cplusplus */

#endif
/* TM1637_H */
