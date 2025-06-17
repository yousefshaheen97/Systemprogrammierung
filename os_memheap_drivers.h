/*
 * os_memheap_drivers.h
 *
 * Created: 27/05/2025 16:02:06
 *  Author: MSI GF72 8RE
 */ 


#ifndef OS_MEMHEAP_DRIVERS_H_
#define OS_MEMHEAP_DRIVERS_H_

#include <stdint.h>
#include <stddef.h>
#include "os_mem_drivers.h"   

#define MAX_NUMBER_OF_PROCESSES 8


typedef enum AllocStrategy {
	OS_MEM_FIRST,  
	OS_MEM_NEXT,   
	OS_MEM_BEST,   
	OS_MEM_WORST   
} AllocStrategy;


typedef struct Heap {
	const MemDriver* driver;      
	MemAddr mapStart;    
	size_t mapSize;     
	MemAddr useStart;   
	size_t useSize;     
	AllocStrategy allocStrat;
	MemAddr allocFrameStart[MAX_NUMBER_OF_PROCESSES];
	MemAddr allocFrameEnd  [MAX_NUMBER_OF_PROCESSES];  
	const char* name;        
} Heap;


extern Heap intHeap__;
#define intHeap (&intHeap__)


void os_initHeaps(void);

uint8_t os_getHeapListLength(void);

Heap* os_lookupHeap(uint8_t index);



extern Heap extHeap__;
#define extHeap (&extHeap__)








#endif 