/*
 * os_spi.c
 *
 * Created: 13.06.2025 18:30:56
 *  Author: yousef
 */ 
#include "os_spi.h"
#include "os_scheduler.h"


/* MOSI, SCK, CS als ausgang, MISO als eingang , 
CS auf high,SPE für SPI enable, 
MSTR master mode, CPOL=0 und CPHA=0 für Mode 0, 
DORD=0 für MSB first ,SPI2X=1, SPR1 und SPR0 =0 für fcpu/2  */


void os_spi_init(void) {
	// pins belegung
	SPI_DDR  |=  (1<<SPI_CS) | (1<<SPI_MOSI) | (1<<SPI_SCK); // Ausgang
	SPI_DDR  &= ~(1<<SPI_MISO);                              // Eingang
	SPI_PORT |=  (1<<SPI_MISO);
	SPI_PORT |=  (1<<SPI_CS);
	SPCR = (1<<SPE)                  // SPI Enable
	| (1<<MSTR)                 // Master mode
	| (0<<DORD)                 // MSB first
	| (0<<CPOL) | (0<<CPHA)     // Mode 0
	| (0<<SPR1) | (0<<SPR0);    // fcpu/4
	SPSR = (1<<SPI2X);   // fcpu/2
}

uint8_t os_spi_send(uint8_t data) {
	uint8_t result;
	os_enterCriticalSection();
	SPDR = data;        // start transmission
	while (!(SPSR & (1<<SPIF))); // wait for completion
	result = SPDR;      // read received byte
	os_leaveCriticalSection();
	return result;
}

uint8_t os_spi_receive(void) {
	return os_spi_send(0xFF);
}