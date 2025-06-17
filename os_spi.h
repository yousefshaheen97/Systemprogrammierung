/*
 * os_spi.h
 *
 * Created: 13.06.2025 18:31:16
 *  Author: yousef
 */ 
#include "util.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>


#ifndef OS_SPI_H_
#define OS_SPI_H_


void os_spi_init(void);


uint8_t os_spi_send(uint8_t data);


uint8_t os_spi_receive();


#define SPI_DDR   DDRB
#define SPI_PORT  PORTB
#define SPI_CS    PB4    
#define SPI_MOSI  PB5
#define SPI_MISO  PB6
#define SPI_SCK   PB7

#endif /* OS_SPI_H_ */