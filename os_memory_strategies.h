/*
 * os_memory_strategies.h
 *
 * Created: 27/05/2025 16:15:35
 *  Author: MSI GF72 8RE
 */ 


#ifndef OS_MEMORY_STRATEGIES_H_
#define OS_MEMORY_STRATEGIES_H_


#include <stdint.h>
#include "os_memheap_drivers.h"
#include "os_memory.h"  


MemAddr os_Memory_FirstFit(Heap *heap, size_t size);
MemAddr os_Memory_NextFit(Heap *heap, size_t size);
MemAddr os_Memory_WorstFit(Heap *heap, size_t size);
MemAddr os_Memory_BestFit(Heap *heap, size_t size);


#endif 