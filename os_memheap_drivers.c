/*
 * os_memheap_drivers.c
 *
 * Created: 27/05/2025 16:02:31
 *  Author: MSI GF72 8RE
 */ 


#include "os_memheap_drivers.h"
#include "defines.h"      
#include "led_draw.h"

Heap intHeap__ = {
	.driver     = intSRAM,
	.mapStart   = HEAP_MAP_START,
	.mapSize    = HEAP_MAP_SIZE,
	.useStart   = HEAP_USE_START,
	.useSize    = HEAP_USE_SIZE,
	.allocStrat = OS_MEM_FIRST,
	.name       = "intHeap"
};

void os_initHeaps(void) {
	draw_letter('u', 5, 19, COLOR_YELLOW, false, false);
	 // Interner Heap
	intHeap__.driver->init();
	for (size_t i = 0; i < intHeap__.mapSize; ++i) {
		intHeap__.driver->write(intHeap__.mapStart + i, (MemValue)0x00);
	}
	
	// Externer Heap:
	extHeap__.driver->init();
	for (size_t i = 0; i < extHeap__.mapSize; ++i) {
		extHeap__.driver->write(extHeap__.mapStart + i, (MemValue)0x00);
	}
	
}

uint8_t os_getHeapListLength(void) {
	return 1;  
}

Heap* os_lookupHeap(uint8_t index) {
	if (index == 0) {
		return &intHeap__;
		} else if (index == 1) {
		return &extHeap__;
	}
	return 0;
}



Heap extHeap__ = {
	.driver     = extSRAM,
	.mapStart   = EXTHEAP_MAP_START,
	.mapSize    = EXTHEAP_MAP_SIZE,
	.useStart   = EXTHEAP_USE_START,
	.useSize    = EXTHEAP_USE_SIZE,
	.allocStrat = OS_MEM_FIRST,
	.name       = "extHeap"
};
