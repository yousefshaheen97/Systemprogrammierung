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

	// Initialwerte zurücksetzen
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
	// erste nibble in map löschen
	os_setMapEntry(heap, addr, 0);
	heap->driver->write(addr, 0);
	// folgende nibble löschen
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
	// grenze zurücksetzen
	heap->allocFrameStart[pid] = heap->useStart + heap->useSize;
	heap->allocFrameEnd  [pid] = heap->useStart;
}




MemAddr os_realloc(Heap *heap, MemAddr addr, uint16_t newSize){
	
	if (addr == 0 || newSize == 0)
	return 0;

	os_enterCriticalSection();
	uint8_t pid       = os_getCurrentProc();
	MemAddr heapStart = heap->useStart;
	MemAddr heapEnd   = heapStart + heap->useSize;

	// Chunk-Anfang finden
	MemAddr head = addr;
	while (head > heapStart && os_getMapEntry(heap, head) == 0xF)
	head--;

	// Besitzer prüfen
	if (os_getMapEntry(heap, head) != pid) {
		os_error("ERROR OWNER");
		os_leaveCriticalSection();
		return 0;
	}

	
	uint16_t oldLen = 1;
	for (MemAddr p = head + 1;
	p < heapEnd && os_getMapEntry(heap, p) == 0xF; p++)
	oldLen++;

	
	if (oldLen == newSize) {
		os_leaveCriticalSection();
		return head;
	}

	
	if (newSize < oldLen) {
		for (MemAddr p = head + newSize; p < head + oldLen; p++)
		os_setMapEntry(heap, p, 0);
		os_leaveCriticalSection();
		return head;
	}

	uint16_t need = newSize - oldLen;

	// freie mapbereich rechts zählen
	uint16_t rightFree = 0;
	for (MemAddr p = head + oldLen;
	rightFree < need && p < heapEnd && os_getMapEntry(heap, p) == 0; p++)
	rightFree++;

	
	if (rightFree >= need) {
		for (uint16_t i = 0; i < need; i++)
		os_setMapEntry(heap, head + oldLen + i, 0xF);
		os_leaveCriticalSection();
		return head;
	}

	// zähle freie map bereich nach links
	uint16_t leftFree = 0;
	for (MemAddr p = head;
	leftFree < need && p > heapStart && os_getMapEntry(heap, p - 1) == 0; )
	{
		p--;
		leftFree++;
	}

	if (leftFree + rightFree >= need) {
		
		uint16_t useLeft = leftFree;               
		uint16_t useRight = need - useLeft;        
		MemAddr newHead = head - useLeft;          

		// kopiere use bereich
		for (uint16_t i = 0; i < oldLen; i++) {
			MemValue v = heap->driver->read(head + i);
			heap->driver->write(newHead + i, v);
		}

		// alte map bereich lösche
		for (uint16_t i = 0; i < oldLen; i++)
		os_setMapEntry(heap, head + i, 0);

		// neu map setzen
		os_setMapEntry(heap, newHead, pid);
		for (uint16_t i = 1; i < newSize; i++)
		os_setMapEntry(heap, newHead + i, 0xF);

		// 0xF löschen
		for (MemAddr p = newHead + newSize;
		p < heapEnd && os_getMapEntry(heap, p) == 0xF; p++)
		os_setMapEntry(heap, p, 0);

		// rechts anhängen 
		for (uint16_t i = 0; i < useRight; i++)
		os_setMapEntry(heap, newHead + oldLen + i, 0xF);

		os_leaveCriticalSection();
		return newHead;
	}

	// Fallback – neuallokieren
	MemAddr fresh = os_malloc(heap, newSize);
	if (fresh) {
		for (uint16_t i = 0; i < oldLen; i++)
		heap->driver->write(fresh + i, heap->driver->read(head + i));
		os_free(heap, head);
	}
	os_leaveCriticalSection();
	return fresh;
}