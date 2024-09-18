
/* 
 *  WarmCat_6x14Backpack.h - Library for the
 *  6 digit, 14 segment display packpack.
 *  Created April 2019. v0.9.2
 *  Wayne K Jones - WarmCat Solutions Ltd.
 *  https://github.com/WarmCatUK/WarmCat_6x14Backpack
 *  Released under Creative Commons Attribution-ShareAlike 3.0 Licence
 *  Any reproduction must include above text.
 */


#ifndef WarmCat_6x14Backpack_h
#define WarmCat_6x14Backpack_h

#include <Wire.h>

static const uint8_t DisplayNo[8] = {
  0x70,
  0x71,
  0x72,
  0x73,
  0x74,
  0x75,
  0x76,
  0x77,
};

static const uint16_t AllOn =   0b0111111111111111;
static const uint16_t Swirly[14] = {
  0b0000000000000001, // A
  0b0000000000000010, // B
  0b0000000000000100, // C
  0b0000000000001000, // D
  0b0000000000010000, // E
  0b0000000000100000, // F
  0b0000000100000000, // H
  0b0000001000000000, // J
  0b0000010000000000, // K
  0b0000000010000000, // G2
  0b0000100000000000, // N
  0b0001000000000000, // M
  0b0010000000000000, // L
  0b0000000001000000, // G1
};
static const uint16_t FourteenSegmentASCII[96] = {
  0b000000000000000, /* (space) */
  0b100000000000110, /* ! */
  0b000001000000010, /* " */
  0b001001011001110, /* # */
  0b001001011101101, /* $ */
  0b011111111100100, /* % */
  0b000101101011001, /* & */
  0b000001000000000, /* ' */
  0b000110000000000, /* ( */
  0b010000100000000, /* ) */
  0b011111111000000, /* * */
  0b001001011000000, /* + */
  0b010000000000000, /* , */
  0b000000011000000, /* - */
  0b100000000000000, /* . */
  0b010010000000000, /* / */
  0b010010000111111, /* 0 */
  0b000010000000110, /* 1 */
  0b000000011011011, /* 2 */
  0b000000010001111, /* 3 */
  0b000000011100110, /* 4 */
  0b000100001101001, /* 5 */
  0b000000011111101, /* 6 */
  0b000000000000111, /* 7 */
  0b000000011111111, /* 8 */
  0b000000011101111, /* 9 */
  0b001001000000000, /* : */
  0b010001000000000, /* ; */
  0b000110001000000, /* < */
  0b000000011001000, /* = */
  0b010000110000000, /* > */
  0b101000010000011, /* ? */
  0b000001010111011, /* @ */
  0b000000011110111, /* A */
  0b001001010001111, /* B */
  0b000000000111001, /* C */
  0b001001000001111, /* D */
  0b000000001111001, /* E */
  0b000000001110001, /* F */
  0b000000010111101, /* G */
  0b000000011110110, /* H */
  0b001001000001001, /* I */
  0b000000000011110, /* J */
  0b000110001110000, /* K */
  0b000000000111000, /* L */
  0b000010100110110, /* M */
  0b000100100110110, /* N */
  0b000000000111111, /* O */
  0b000000011110011, /* P */
  0b000100000111111, /* Q */
  0b000100011110011, /* R */
  0b000000011101101, /* S */
  0b001001000000001, /* T */
  0b000000000111110, /* U */
  0b010010000110000, /* V */
  0b010100000110110, /* W */
  0b010110100000000, /* X */
  0b000000011101110, /* Y */
  0b010010000001001, /* Z */
  0b000000000111001, /* [ */
  0b000100100000000, /* \ */
  0b000000000001111, /* ] */
  0b010100000000000, /* ^ */
  0b000000000001000, /* _ */
  0b000000100000000, /* ` */
  0b000000011110111, /* A */
  0b001001010001111, /* B */
  0b000000000111001, /* C */
  0b001001000001111, /* D */
  0b000000001111001, /* E */
  0b000000001110001, /* F */
  0b000000010111101, /* G */
  0b000000011110110, /* H */
  0b001001000001001, /* I */
  0b000000000011110, /* J */
  0b000110001110000, /* K */
  0b000000000111000, /* L */
  0b000010100110110, /* M */
  0b000100100110110, /* N */
  0b000000000111111, /* O */
  0b000000011110011, /* P */
  0b000100000111111, /* Q */
  0b000100011110011, /* R */
  0b000000011101101, /* S */
  0b001001000000001, /* T */
  0b000000000111110, /* U */
  0b010010000110000, /* V */
  0b010100000110110, /* W */
  0b010110100000000, /* X */
  0b000000011101110, /* Y */
  0b010010000001001, /* Z */
  0b010000101001001, /* { */
  0b001001000000000, /* | */
  0b000110010001001, /* } */
  0b010010011000000, /* ~ */
  0b000000000000000, /* (del) */
};

class WarmCat6x14
{
  public:
    WarmCat6x14(uint8_t displayCount);
    void begin();
    void clear();
    void blink(uint8_t bl);
    void setBrightness(uint8_t brightness);
    void showOnDisp(uint8_t disp);
    void showScroll(void);
    void emptyScrollBuffer(void);

    void dots();
    void disp6Char(char text[], uint8_t disp);
    void scrollText(char text[], int scrollrate = 120);
    void dispChar(uint8_t disp, uint8_t digit, byte ascii, bool dp = false);
    void swirlyAll(int swirlrate = 20);
    void swirly(int swirlrate = 20);

    void scrollSerialText(char c, int scrollrate);
    
    uint16_t displayBuffer[8];
    int scrollBuffer[8][8];
    uint8_t displayCount;
    
  private:
    static uint8_t _displayCount;
};




#endif
