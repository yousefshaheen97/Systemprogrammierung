/*
 * os_memory.h
 *
 * Created: 27/05/2025 16:18:56
 *  Author: MSI GF72 8RE
 */ 


#ifndef OS_MEMORY_H_
#define OS_MEMORY_H_


#include <stdint.h>
#include <stddef.h>
#include "os_memheap_drivers.h"
#include "os_process.h"
#include "os_mem_drivers.h"


MemAddr os_malloc(Heap* heap, uint16_t size);

void os_free(Heap* heap, MemAddr addr);


uint8_t os_getMapEntry(Heap const* heap, MemAddr useAddr);
void os_setMapEntry(Heap const* heap, MemAddr useAddr, uint8_t value);


size_t os_getMapSize(Heap const* heap);
MemAddr os_getMapStart(Heap const* heap);
size_t os_getUseSize(Heap const* heap);
MemAddr os_getUseStart(Heap const* heap);


uint16_t os_getChunkSize( Heap const* heap, MemAddr addr);


void os_setAllocationStrategy(Heap* heap, AllocStrategy strat);
AllocStrategy os_getAllocationStrategy(Heap const* heap);

void os_freeProcessMemory(Heap* heap, ProcessID pid);
MemAddr os_realloc(Heap* heap, MemAddr addr, uint16_t size);





#endif 