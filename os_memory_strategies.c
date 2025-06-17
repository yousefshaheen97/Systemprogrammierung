/*
 * os_memory_strategies.c
 *
 * Created: 27/05/2025 16:15:21
 *  Author: MSI GF72 8RE
 */ 


#include "os_memory_strategies.h"
#include "os_memory.h"  
#include "os_process.h"
#include "os_scheduler.h"
#include "os_core.h"
#include "lcd.h"
#include "os_input.h"






MemAddr os_Memory_FirstFit(Heap *heap, size_t size) {
	if (size == 0 || size > heap->useSize) {
		return 0;
	}

	MemAddr startAllocAddr = heap->useStart;
	MemAddr endAllocAddr   = heap->useStart + heap->useSize;
	MemAddr limit;
	if (endAllocAddr >= size) {
		
		limit = endAllocAddr - size;
	}
	else {
		
		limit = heap->useStart;
	}
	for (MemAddr addr = startAllocAddr; addr <= limit; ++addr) {
		bool block_free = true;
		for (size_t offset = 0; offset < size; ++offset) {
			if (os_getMapEntry(heap, addr + offset) != 0) {
				block_free = false;
				break;
			}
		}
		if (block_free) {
			return addr;
		}
	}
	return 0;
}




MemAddr os_Memory_NextFit(Heap *heap, size_t size) {
	if (size == 0 || size > heap->useSize) {
		return 0;
	}
	static MemAddr lastPosition = 0;
	MemAddr startAllocAddr = heap->useStart;
	MemAddr endAllocAddr   = startAllocAddr + heap->useSize;

	if (lastPosition < startAllocAddr || lastPosition >= endAllocAddr) {
		lastPosition = startAllocAddr;
	}
	MemAddr limit = endAllocAddr - size;
	MemAddr cursor;

	for (cursor = lastPosition; cursor <= limit; ++cursor) {
		int ok = 1;
		for (size_t i = 0; i < size; ++i) {
			if (os_getMapEntry(heap, cursor + i) != 0) {
				ok = 0;
				break;
			}
		}
		if (ok) {
			lastPosition = cursor + size;
			return cursor;
		}
	}
	for (cursor = startAllocAddr; cursor < lastPosition && cursor <= limit; ++cursor) {
		int ok = 1;
		for (size_t i = 0; i < size; ++i) {
			if (os_getMapEntry(heap, cursor + i) != 0) {
				ok = 0;
				break;
			}
		}
		if (ok) {
			lastPosition = cursor + size;
			return cursor;
		}
	}
	return 0;
}




MemAddr os_Memory_BestFit(Heap *heap, size_t size) {
	if (size == 0 || size > heap->useSize) {
		return 0;
	}

	MemAddr start   = heap->useStart;
	MemAddr end     = heap->useStart + heap->useSize;
	MemAddr bestAdr = 0;
	size_t  bestLen = heap->useSize + 1; 

	MemAddr pos = start;
	while (pos < end) {
		// ignoriere bestzte segmente
		if (os_getMapEntry(heap, pos) != 0) {
			pos++;
			continue;
		}
		
		MemAddr freeStart = pos;
		size_t  freeLen   = 0;
		// berchne groesse passende segmente
		while (pos < end && os_getMapEntry(heap, pos) == 0) {
			freeLen++;
			pos++;
		}
		// suche kleineste segment
		if (freeLen >= size && freeLen < bestLen) {
			bestLen = freeLen;
			bestAdr = freeStart;
		}
	}

	return bestAdr;
}




MemAddr os_Memory_WorstFit(Heap *heap, size_t size) {
	
	if (size == 0 || size > heap->useSize) {
		return 0;
	}

	MemAddr heapStart = heap->useStart;
	MemAddr heapEnd   = heap->useStart + heap->useSize;
	MemAddr worstStart = 0;
	size_t  worstSize  = 0; 

	MemAddr position = heapStart;
	// suche ganzen heap
	while (position < heapEnd) {
		// freie segmente nur betrachten
		if (os_getMapEntry(heap, position) == 0) {
			// merke anfang des freien segment
			MemAddr segmentStart = position;
			size_t segmentlaenge = 0;
			// länge des segment bestimmen
			while (position < heapEnd && os_getMapEntry(heap, position) == 0) {
				segmentlaenge++;
				position++;
			}
			// vergleiche mit dem alten segment
			if (segmentlaenge >= size && segmentlaenge > worstSize) {
				worstSize  = segmentlaenge;
				worstStart = segmentStart;
			}
			} else {
			// bestzte segment ignorieren
			position++;
		}
	}

	return worstStart; 
}


