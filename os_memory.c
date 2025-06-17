/*
 * os_memory.c
 *
 * Created: 27/05/2025 16:19:10
 *  Author: MSI GF72 8RE
 */ 


#include <stdint.h>
#include <stddef.h>
#include "os_memory.h"
#include "os_memory_strategies.h"
#include "os_scheduler.h"  
#include "os_process.h"
#include "os_core.h"




MemAddr mapByteAddr(Heap const* heap, MemAddr addr) { 
	
	uint16_t offset = addr - heap->useStart;
	
	return heap->mapStart + (offset >> 1);
}

bool isHigh (Heap const* heap, MemAddr addr) { 
	
	return ((addr - heap->useStart) & 1) == 0;
}


uint8_t os_getMapEntry(Heap const* heap, MemAddr addr){
	
	if (addr < heap->useStart || addr >= heap->useStart + heap->useSize) {
		return 0xFF;  
	}
	
	MemAddr m = mapByteAddr(heap, addr);
	MemValue b = heap->driver->read(m);
	
	if (isHigh(heap, addr)){
		return (b >> 4) & 0x0F;
	} else {
		return b & 0x0F;
	}
	
}

void os_setMapEntry(Heap const* heap, MemAddr addr, uint8_t value){
	
	if (addr < heap->useStart || addr >= heap->useStart + heap->useSize) {
		return;
	}
	
	MemAddr mapAddr = mapByteAddr(heap, addr);

	MemValue raw = heap->driver->read(mapAddr);

	uint8_t lowNib  = raw & 0x0F;
	uint8_t highNib = (raw >> 4) & 0x0F;

	
	uint8_t newNib = value & 0x0F;

	
	if (isHigh(heap, addr)) {
		
		highNib = newNib;
		} else {
		
		lowNib = newNib;
	}
	
	MemValue combined = (MemValue)((highNib << 4) | lowNib);

	heap->driver->write(mapAddr, combined);
}




void frameExtend(Heap *heap, ProcessID pid, MemAddr start, uint16_t len){
	if (start < heap->allocFrameStart[pid])
	heap->allocFrameStart[pid] = start;
	if (start + len > heap->allocFrameEnd[pid])
	heap->allocFrameEnd[pid] = start + len;
}


void frameShrinkIfNeeded(Heap *heap, ProcessID pid){
	// nach link ruecken, wenn start >= end
	MemAddr end   = heap->allocFrameEnd[pid];
	MemAddr start = heap->allocFrameStart[pid];
	if (start >= end) return;                         

	// schrumpfe den chunk
	while (end > start && os_getMapEntry(heap, end-1) == 0)
	--end;
	heap->allocFrameEnd[pid] = end;

	// Initialwerte zur�cksetzen
	if (heap->allocFrameStart[pid] >= heap->allocFrameEnd[pid]) {
		heap->allocFrameStart[pid] = heap->useStart + heap->useSize;
		heap->allocFrameEnd  [pid] = heap->useStart;
	}
}





MemAddr os_malloc(Heap* heap, uint16_t size){
	if (size == 0) return 0;
	
	if (size > heap->useSize) {
		return 0;
	}

	os_enterCriticalSection();
	
	MemAddr addr = 0;
	switch (heap->allocStrat) {
		case OS_MEM_FIRST: addr = os_Memory_FirstFit (heap, size); break;
		case OS_MEM_NEXT: addr = os_Memory_NextFit (heap, size); break;
		case OS_MEM_BEST: addr = os_Memory_BestFit (heap, size); break;
		case OS_MEM_WORST: addr = os_Memory_WorstFit (heap, size); break;
	}

	if (addr) {
		
		if (addr < heap->useStart || addr >= heap->useStart + heap->useSize) {
			os_leaveCriticalSection();
			return 0;
		}
		
		uint8_t pid = os_getCurrentProc();
		os_setMapEntry(heap, addr, pid);
		
		for (uint16_t i = 1; i < size; ++i) {
			
			if (addr + i < heap->useStart + heap->useSize) {
				os_setMapEntry(heap, addr + i, 0xF);
			}
		}
	}
	ProcessID pid = os_getCurrentProc();
	frameExtend(heap, pid, addr, size);

	os_leaveCriticalSection();
	return addr;
}




void os_free(Heap* heap, MemAddr addr){
	if(!addr) return;
	os_enterCriticalSection();
	// chunk anfang finden
	if(os_getMapEntry(heap, addr) == 0) {
		os_leaveCriticalSection();
		return;
	}
	while(addr > heap->useStart && os_getMapEntry(heap, addr) == 0xF) {
		--addr;
	}
	// erste nibble in map l�schen
	os_setMapEntry(heap, addr, 0);
	heap->driver->write(addr, 0);
	// folgende nibble l�schen
	MemAddr heapEnd = heap->useStart + heap->useSize;
	MemAddr p = addr + 1;
	while(p < heapEnd && os_getMapEntry(heap, p) == 0xF) {
		os_setMapEntry(heap, p, 0);
		heap->driver->write(p, 0);
		++p;
	}
	
	uint16_t size = os_getChunkSize(heap, addr);
	ProcessID pid = os_getCurrentProc();
	if (addr <= heap->allocFrameStart[pid] || addr+size >= heap->allocFrameEnd[pid])
	frameShrinkIfNeeded(heap, pid);
	
	
	os_leaveCriticalSection();
}



size_t os_getMapSize (Heap const* heap){ 
	return heap->mapSize; 
}
MemAddr os_getMapStart (Heap const* heap){ 
	return heap->mapStart; 
}
size_t os_getUseSize (Heap const* heap){ 
	return heap->useSize; 
}
MemAddr os_getUseStart (Heap const* heap){ 
	return heap->useStart; 
}

uint16_t os_getChunkSize (Heap const* heap, MemAddr addr){
	
	if (addr == 0) {
		return 0;
	}
	if (addr < heap->useStart) {
		return 0;
	}
	MemAddr heapEnd = heap->useStart + heap->useSize;
	if (addr >= heapEnd) {
		return 0;
	}

	MemAddr chunkStart = addr;
	while (chunkStart > heap->useStart) {
		uint8_t mark = os_getMapEntry(heap, chunkStart);
		if (mark == 0xF) {
			chunkStart--;
			} else {
			break;
		}
	}

	
	uint8_t firstMark = os_getMapEntry(heap, chunkStart);
	if (firstMark == 0 || firstMark == 0xF) {
		return 0;
	}

	
	uint16_t length = 0;
	MemAddr current = chunkStart;
	while (current < heapEnd) {
		uint8_t mark = os_getMapEntry(heap, current);
		if (mark == 0) {
			break;  
		}
		length++;
		current++;
	}

	return length;
}


void os_setAllocationStrategy(Heap* heap, AllocStrategy allocStrat){ 
	heap->allocStrat = allocStrat; 
}
AllocStrategy os_getAllocationStrategy(Heap const* heap){ 
	return heap->allocStrat; 
}






void os_freeProcessMemory(Heap* heap, ProcessID pid) {
	// begrenze der suche
	MemAddr start = heap->allocFrameStart[pid];
	MemAddr end   = heap->allocFrameEnd  [pid];

	
	for (MemAddr addr = start; addr < end; ) {
		
		if (os_getMapEntry(heap, addr) == (uint8_t)pid) {
			
			os_free(heap, addr);

			
			uint16_t len = os_getChunkSize(heap, addr);
			
			addr += (len > 0 ? len : 1);
		}
		else {
		
			++addr;
		}
	}
	// grenze zur�cksetzen
	heap->allocFrameStart[pid] = heap->useStart + heap->useSize;
	heap->allocFrameEnd  [pid] = heap->useStart;
}




static void move(Heap *heap, MemAddr destination, MemAddr source, uint16_t length) {
	if (destination == source || length == 0) return;

	if (destination < source) {
		for (uint16_t i = 0; i < length; ++i) {
			heap->driver->write(destination + i, heap->driver->read(source + i));
		}
		} else {
		for (uint16_t i = length; i > 0; --i) {
			heap->driver->write(destination + i - 1, heap->driver->read(source + i - 1));
		}
	}
}

MemAddr os_realloc(Heap* heap, MemAddr addr, uint16_t size){
	if (addr == 0) {
		return os_malloc(heap, size);
	}

	if (size == 0) {
		os_free(heap, addr);
		return 0;
	}

	os_enterCriticalSection();

	ProcessID pid = os_getCurrentProc();
	if (os_getMapEntry(heap, addr) != pid) {
		os_leaveCriticalSection();
		os_error("NOT OWNER");
		return 0;
	}

	uint16_t oldSize = 1;
	MemAddr scan = addr + 1;
	MemAddr heapEnd = heap->useStart + heap->useSize;
	while (scan < heapEnd && os_getMapEntry(heap, scan) == 0xF) {
		++oldSize;
		++scan;
	}

	if (size == oldSize) {
		os_leaveCriticalSection();
		return addr;
	}


	if (size < oldSize) {
		for (uint16_t i = size; i < oldSize && addr + i < heapEnd; ++i) {
			os_setMapEntry(heap, addr + i, 0);
			heap->driver->write(addr + i, 0);
		}

		if (addr + oldSize >= heap->allocFrameEnd[pid]) {
			frameShrinkIfNeeded(heap, pid);
		}

		os_leaveCriticalSection();
		return addr;
	}

	uint16_t difference = size - oldSize;

	uint16_t rightFree = 0;
	MemAddr p = addr + oldSize;
	while (p < heapEnd && os_getMapEntry(heap, p) == 0) {
		++rightFree;
		++p;
		if (rightFree >= difference) break;
	}

	uint16_t leftFree = 0;
	p = addr;
	while (p > heap->useStart && os_getMapEntry(heap, p - 1) == 0) {
		++leftFree;
		--p;
	}

	if (rightFree >= difference) {
		for (uint16_t i = 0; i < difference; ++i) {
			os_setMapEntry(heap, addr + oldSize + i, 0xF);
		}
		frameExtend(heap, pid, addr, size);
		os_leaveCriticalSection();
		return addr;
	}

	if (leftFree >= difference) {
		MemAddr newStart = addr - leftFree;

		move(heap, newStart, addr, oldSize);

		for (MemAddr a = newStart + size; a < addr + oldSize; ++a) {
			os_setMapEntry(heap, a, 0);
			heap->driver->write(a, 0);
		}

		for (uint16_t i = 0; i < size; ++i) {
			os_setMapEntry(heap, newStart + i, (i == 0) ? pid : 0xF);
		}

		addr = newStart;
		frameExtend(heap, pid, addr, size);
		os_leaveCriticalSection();
		return addr;
	}

	if (leftFree + rightFree >= difference) {
		MemAddr newStartAddr = addr - leftFree;

		move(heap, newStartAddr, addr, oldSize);

		for (uint16_t i = 0; i < oldSize; ++i) {
			MemAddr a = addr + i;
			if (a < newStartAddr || a >= newStartAddr + size) {
				os_setMapEntry(heap, a, 0);
				heap->driver->write(a, 0);
			}
		}

		for (uint16_t i = 0; i < size; ++i) {
			os_setMapEntry(heap, newStartAddr + i, (i == 0) ? pid : 0xF);
		}

		addr = newStartAddr;
		frameExtend(heap, pid, addr, size);
		os_leaveCriticalSection();
		return addr;
	}

	os_leaveCriticalSection();

	MemAddr newAddr = os_malloc(heap, size);
	if (!newAddr) {
		return 0;
	}

	for (uint16_t i = 0; i < oldSize && i < size; ++i) {
		heap->driver->write(newAddr + i, heap->driver->read(addr + i));
	}
	os_free(heap, addr);
	return newAddr;
}
