#define DDR_SPI DDRB
#define PORT_SPI PORTB
#define DD_MOSI 3
#define DD_SCK 5
#define DD_SS 2

#define DDR_TLC5940 DDRD
#define PORT_TLC5940 PORTD
#define DD_VPRG 5
#define DD_XLAT 6
#define DD_BLANK 7
#define DDR_GSCLK DDRB
#define DD_GSCLK 1

void TLC5940_Init(uint8_t count, uint8_t dotCorrection[], uint16_t grayScale[]);
