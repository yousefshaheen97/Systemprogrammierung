/*
 * os_mem_drivers.c
 *
 * Created: 27/05/2025 15:13:14
 *  Author: MSI GF72 8RE
 */ 


#include "os_mem_drivers.h"
#include "defines.h" 
#include "os_spi.h"
#include <avr/io.h>
#include "os_scheduler.h"

void intSRAM_init(void) {
	
}


MemValue intSRAM_read(MemAddr addr) {
	volatile uint8_t *ptr = (volatile uint8_t *)addr;
	return *ptr;
}


void intSRAM_write(MemAddr addr, MemValue value) {
	volatile uint8_t *ptr = (volatile uint8_t *)addr;
	*ptr = value;
}


MemDriver intSRAM__ = {
	.init  = intSRAM_init,
	.read  = intSRAM_read,
	.write = intSRAM_write,
	.start = AVR_SRAM_START,
	.size = AVR_MEMORY_SRAM
};




MemDriver extSRAM__ = {
	.init  = initSRAM_external,
	.read  = readSRAM_external,
	.write = writeSRAM_external,
	.start = EXTHEAP_MAP_START,
	.size  = EXTHEAP_TOTAL_SIZE
};



void selectChip(){
	EXTSRAM_CS_PORT &= ~(1 << EXTSRAM_CS_PIN);
}

void deselectChip(){
	EXTSRAM_CS_PORT |= (1 << EXTSRAM_CS_PIN);
}


void initSRAM_external(void) {
	os_spi_init();
	selectChip();
	os_spi_send(SRAM_CMD_WRMR);
	os_spi_send(SRAM_MODE_BYTE);
	deselectChip();
}

MemValue readSRAM_external(MemAddr addr) {
	os_enterCriticalSection();
	MemValue value;
	selectChip();
	// lese befehl senden
	os_spi_send(SRAM_CMD_READ);
	// send 16 bit address
	os_spi_send(0x00);
	os_spi_send((addr >> 8) & 0xFF);    // High byte
	os_spi_send(addr & 0xFF);   // low byte
	// bekomme die daten
	value = os_spi_receive();
	deselectChip();
	os_leaveCriticalSection();
	return value;
}

void writeSRAM_external(MemAddr addr, MemValue value) {
	os_enterCriticalSection();
	selectChip();
	// lese befehl senden
	os_spi_send(SRAM_CMD_WRITE);
	// send 16 bit address
	os_spi_send(0x00);
	os_spi_send((MemValue)(addr >> 8));     // high byte
	os_spi_send((MemValue)addr);     // low byte
	// Sendende daten
	os_spi_send(value);
	deselectChip();
	os_leaveCriticalSection();
}
