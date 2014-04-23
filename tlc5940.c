#include <avr/io.h>
#include <avr/interrupt.h>

#include "tlc5940.h"

uint8_t TLC5940_count;
uint16_t *TLC5940_grayScale;

void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output */
	DDR_SPI |= (1<<DD_MOSI)|(1<<DD_SCK)|(1<<DD_SS);
	/* Enable SPI, Master, set clock rate fck/2 */
	SPCR = (1<<SPE)|(1<<MSTR);
	SPSR |= (1<<SPI2X);
}

void SPI_MasterTerminate(void)
{
	/* Disable SPI */
	SPCR = 0;
	SPSR &= ~(1<<SPI2X);
}

void SPI_MasterTransmit(char cData)
{
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF))) ;
}

void TLC5940_TransmitDC(uint8_t dotCorrection[])
{
	/* Send the 6-bit dot correction values as a continuous stream of bytes */
	SPI_MasterInit();
	for (uint8_t i = 0; i < 12 * TLC5940_count; i++)
		SPI_MasterTransmit((dotCorrection[16*TLC5940_count-1-(4*(i/3)+(i%3))]<<(2*((i%3)+1)))|(dotCorrection[16*TLC5940_count-1-(4*(i/3)+(i%3)+1)]>>(2*(2-(i%3)))));
	SPI_MasterTerminate();
}

void TLC5940_TransmitGS()
{
	/* Send the 12-bit grayscale values as a continuous stream of bytes */
	SPI_MasterInit();
	for (uint8_t i = 0; i < 24 * TLC5940_count; i++)
	switch (i%3)
	{
		case 0:
		SPI_MasterTransmit(TLC5940_grayScale[16*TLC5940_count-1-2*(i/3)]>>4);
		break;
		case 1:
		SPI_MasterTransmit((TLC5940_grayScale[16*TLC5940_count-1-2*(i/3)]<<4)|(TLC5940_grayScale[16*TLC5940_count-1-(2*(i/3)+1)]>>8));
		break;
		case 2:
		SPI_MasterTransmit(TLC5940_grayScale[16*TLC5940_count-1-(2*(i/3)+1)]);
		break;
	}
	SPI_MasterTerminate();
}

ISR(TIMER0_COMPA_vect)
{
	/* Pulse SCK */
	PORT_SPI |= (1<<DD_SCK);
	PORT_SPI &= ~(1<<DD_SCK);
	/* Transmit the grayscale data */
	TLC5940_TransmitGS();
	/* Set BLANK to high */
	PORT_TLC5940 |= (1<<DD_BLANK);
	/* Pulse XLAT */
	PORT_TLC5940 |= (1<<DD_XLAT);
	PORT_TLC5940 &= ~(1<<DD_XLAT);
	/* Set BLANK to low */
	PORT_TLC5940 &= ~(1<<DD_BLANK);
}

void TLC5940_Init(uint8_t count, uint8_t dotCorrection[], uint16_t grayScale[])
{
	TLC5940_count = count;
	TLC5940_grayScale = grayScale;

	/* Set the used pins as outputs */
	DDR_TLC5940 |= (1<<DD_VPRG)|(1<<DD_XLAT)|(1<<DD_BLANK);
	DDR_GSCLK |= (1<<DD_GSCLK);
	/* Set VPRG to high */
	PORT_TLC5940 |= (1<<DD_VPRG);
	/* Send the dot correction data */
	TLC5940_TransmitDC(dotCorrection);
	/* Pulse XLAT */
	PORT_TLC5940 |= (1<<DD_XLAT);
	PORT_TLC5940 &= ~(1<<DD_XLAT);
	/* Set VPRG to low */
	PORT_TLC5940 &= ~(1<<DD_VPRG);
	
	/* First GS data */
	/* Set BLANK to high */
	PORT_TLC5940 |= (1<<DD_BLANK);
	/* Send the grayscale data */
	TLC5940_TransmitGS();
	/* Pulse XLAT */
	PORT_TLC5940 |= (1<<DD_XLAT);
	PORT_TLC5940 &= ~(1<<DD_XLAT);
	/* Set BLANK to low */
	PORT_TLC5940 &= ~(1<<DD_BLANK);
	
	/* Disable interrupts until the timers are set up */
	cli();
	/* Use Timer/Counter1 to drive GSCLK at fck/2 */
	/* Toggle OC1A on compare match */
	TCCR1A = (1<<COM1A0);
	/* Clear timer on compare match, no prescaling */
	TCCR1B = (1<<WGM12)|(1<<CS10);
	/* Use Timer/Counter0 to generate an interrupt every 4096 GSCLK cycles */
	/* Clear timer on compare match */
	TCCR0A = (1<<WGM01);
	/* fck/256 prescaling */
	TCCR0B = (1<<CS02);
	/* Count to 31 */
	OCR0A = 31;
	/* Enable output compare match interrupt */
	TIMSK0 = (1<<OCIE0A);
	/* Enable interrupts */
	sei();
}
