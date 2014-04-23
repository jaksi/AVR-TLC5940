#include <avr/io.h>

#include "tlc5940.h"

#define COUNT 1

int main(void)
{
	uint8_t DC[12*COUNT] = {0};
	uint16_t GS[12*COUNT] = {0};
	TLC5940_Init(COUNT, DC, GS);
	while(1)
	{
		// Do something with GS
	}
}
