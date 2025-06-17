/*
 * os_mem_drivers.h
 *
 * Created: 27/05/2025 15:13:53
 *  Author: MSI GF72 8RE
 */ 


#ifndef OS_MEM_DRIVERS_H_
#define OS_MEM_DRIVERS_H_

#include <stdint.h>


typedef uint16_t MemAddr;
typedef uint8_t  MemValue;


typedef struct MemDriver {
	void     (*init)(void);
	MemValue (*read)(MemAddr addr);
	void     (*write)(MemAddr addr, MemValue value);
	MemAddr const start;
	uint16_t const size;
} MemDriver;


extern MemDriver intSRAM__;
#define intSRAM (&intSRAM__)

void initSRAM_external(void);

MemValue readSRAM_external(MemAddr addr);

void writeSRAM_external(MemAddr addr, MemValue value);


extern MemDriver extSRAM__;
#define extSRAM (&extSRAM__)



// 23LC1024 befehle
#define SRAM_CMD_READ   0x03
#define SRAM_CMD_WRITE  0x02
#define SRAM_CMD_WRMR   0x01
#define SRAM_MODE_BYTE  0x00

// CS-Pin 
#define EXTSRAM_CS_PORT PORTB
#define EXTSRAM_CS_DDR  DDRB
#define EXTSRAM_CS_PIN  PB4





#endif 