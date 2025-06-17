/*! \file
 *  \brief Scheduling module for the OS.
 *
 * Contains everything needed to realise the scheduling between multiple processes.
 * Also contains functions to start the execution of programs.
 *
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */

#include "os_scheduler.h"

#include "lcd.h"
#include "os_core.h"
#include "os_input.h"
#include "os_scheduling_strategies.h"
#include "os_taskman.h"
#include "util.h"
#include "os_memheap_drivers.h"
#include "os_memory.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

//----------------------------------------------------------------------------
// Private Types
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

//! Array of states for every possible process
Process os_processes[MAX_NUMBER_OF_PROCESSES];
//#error IMPLEMENT STH. HERE

//! Index of process that is currently executed (default: idle)
ProcessID currentProc = 0;
//#warning IMPLEMENT STH. HERE

//----------------------------------------------------------------------------
// Private variables
//----------------------------------------------------------------------------

//! Currently active scheduling strategy
SchedulingStrategy schedulingStrategy;
//#warning IMPLEMENT STH. HERE

//! Count of currently nested critical sections
uint8_t criticalSectionCount = 0;
//#warning IMPLEMENT STH. HERE

//----------------------------------------------------------------------------
// Private function declarations
//----------------------------------------------------------------------------

//! ISR for timer compare match (scheduler)
ISR(TIMER2_COMPA_vect)
__attribute__((naked));

//----------------------------------------------------------------------------
// Function definitions
//----------------------------------------------------------------------------

/*!
 *  Timer interrupt that implements our scheduler. Execution of the running
 *  process is suspended and the context saved to the stack. Then the periphery
 *  is scanned for any input events. If everything is in order, the next process
 *  for execution is derived with an exchangeable strategy. Finally the
 *  scheduler restores the next process for execution and releases control over
 *  the processor to that process.
 */
ISR(TIMER2_COMPA_vect) {
	
	// 1–2: Kontext speeichern und stackpointer auf den stack aktu. proc
	saveContext();
	os_processes[os_getCurrentProc()].sp.as_int = SP;

	// 3–4: auf ISR-Stack wechseln
	SP = BOTTOM_OF_ISR_STACK;

	// Taskmanager Enter+ESC
	{
		uint8_t input = os_getInput();
		if ((input & ((1<<0)|(1<<3))) == ((1<<0)|(1<<3))) {
			os_taskManMain();
			os_waitForNoInput();
		}
	}

	// 5: aktuellen Prozess auf READY setzen
	if(os_processes[os_getCurrentProc()].state == OS_PS_RUNNING){
		os_processes[os_getCurrentProc()].state = OS_PS_READY;
	}
	
	
	
	os_processes[currentProc].checksum = os_getStackChecksum(currentProc);

	// 6: nächsten Prozess auswählen

	if (schedulingStrategy == OS_SS_EVEN) {
		currentProc = os_Scheduler_Even(os_processes, currentProc);
		} else if (schedulingStrategy == OS_SS_ROUND_ROBIN)
		{
			currentProc = os_Scheduler_RoundRobin(os_processes, currentProc);
		} else if (schedulingStrategy == OS_SS_INACTIVE_AGING)
		{
			currentProc = os_Scheduler_InactiveAging(os_processes, currentProc);
		}else if (schedulingStrategy == OS_SS_RUN_TO_COMPLETION)
		{
			currentProc = os_Scheduler_RunToCompletion(os_processes, currentProc);
		}
		 else {
		currentProc = os_Scheduler_Random(os_processes, currentProc);
	}
	
	if (os_processes[currentProc].checksum != os_getStackChecksum(currentProc))
	{
		os_error("ERROR: INVALID CHECKSUM");
	}
	
	// 7: ändere stand zu running
	
	os_processes[os_getCurrentProc()].state = OS_PS_RUNNING;

	// 8: auf dessen Stack zurückwechseln
	SP = os_processes[os_getCurrentProc()].sp.as_int;

	// 9–10: Kontext wiederherstellen und zurückkehren
	restoreContext();
}


/*!
 *  This is the idle program. The idle process owns all the memory
 *  and processor time no other process wants to have.
 */
void idle(void){
	
	while (1) {
		lcd_writeProgString(PSTR("....."));
		delayMs(DEFAULT_OUTPUT_DELAY);
	}
	
//#warning IMPLEMENT STH. HERE
}

/*!
 *  This function is used to register the given program for execution.
 *  A stack will be provided if the process limit has not yet been reached.
 *  This function is multitasking safe. That means that programs can repost
 *  themselves, simulating TinyOS 2 scheduling (just kick off interrupts ;) ).
 *
 *  \param program  The function of the program to start.
 *  \param priority A priority ranging 0..255 for the new process:
 *                   - 0 means least favourable
 *                   - 255 means most favourable
 *                  Note that the priority may be ignored by certain scheduling
 *                  strategies.
 *  \return The index of the new process or INVALID_PROCESS as specified in
 *          defines.h on failure
 */
ProcessID os_exec(Program *program, Priority priority) {
	
	os_enterCriticalSection();
	
	// Freien Slot finden
	ProcessID pid;
	for (pid = 0; pid < MAX_NUMBER_OF_PROCESSES; pid++) {
		if (os_processes[pid].state == OS_PS_UNUSED) {
			break;
		}
	}
	if (pid >= MAX_NUMBER_OF_PROCESSES || program == NULL) {
		os_leaveCriticalSection();
		return INVALID_PROCESS;
	}

	// Programmzeiger, Zustand und Priorität speichern
	os_processes[pid].program  = program;
	os_processes[pid].state    = OS_PS_READY;
	os_processes[pid].priority = priority;
	
	
	 os_resetProcessSchedulingInformation(pid);
	
	//  stack vorbereiten
	os_processes[pid].sp.as_ptr = (uint8_t*) PROCESS_STACK_BOTTOM(pid); 
	*(os_processes[pid].sp.as_ptr) = 0b0000000011111111 & (uint16_t) os_dispatcher;     
	os_processes[pid].sp.as_int = os_processes[pid].sp.as_int - 1;    
	*(os_processes[pid].sp.as_ptr) = (0b1111111100000000 & (uint16_t) os_dispatcher) >> 8; 
	os_processes[pid].sp.as_int = os_processes[pid].sp.as_int - 1; 
	
	
	
	for (int i=0; i<33; i++) 
	{
		*(os_processes[pid].sp.as_ptr) = 0;
		os_processes[pid].sp.as_int = os_processes[pid].sp.as_int - 1;
	}
	
	
	os_processes[pid].checksum = os_getStackChecksum(pid);
	os_leaveCriticalSection();
	
	return pid;
	
//#error IMPLEMENT STH. HERE
}

	

/*!
 *  If all processes have been registered for execution, the OS calls this
 *  function to start the idle program and the concurrent execution of the
 *  applications.
 */
void os_startScheduler(void) {

	
	currentProc = 0;
	os_processes[currentProc].state = OS_PS_RUNNING;
	SP = os_processes[currentProc].sp.as_int;
	restoreContext();

//#warning IMPLEMENT STH. HERE
}

/*!
 *  In order for the Scheduler to work properly, it must have the chance to
 *  initialize its internal data-structures and register.
 */
void os_initScheduler(void){
	
	// alles auf unesed setzen
	for (ProcessID i = 0; i < MAX_NUMBER_OF_PROCESSES; i++) {
		os_processes[i].state = OS_PS_UNUSED;
	}
	// idle = proc 0
	os_exec(idle, DEFAULT_PRIORITY);

	while(autostart_head != NULL){
		
		os_exec(autostart_head->program, DEFAULT_PRIORITY);
		
		autostart_head = autostart_head->next;
	}
	
//#warning IMPLEMENT STH. HERE
}

/*!
 *  A simple getter for the slot of a specific process.
 *
 *  \param pid The processID of the process to be handled
 *  \return A pointer to the memory of the process at position pid in the os_processes array.
 */
Process *os_getProcessSlot(ProcessID pid) {
    return os_processes + pid;
}

/*!
 *  A simple getter to retrieve the currently active process.
 *
 *  \return The process id of the currently active process.
 */
ProcessID os_getCurrentProc(void) {
//#warning IMPLEMENT STH. HERE
    return currentProc;
}

/*!
 *  Sets the current scheduling strategy.
 *
 *  \param strategy The strategy that will be used after the function finishes.
 */
void os_setSchedulingStrategy(SchedulingStrategy strategy){
//#warning IMPLEMENT STH. HERE
	schedulingStrategy = strategy;
	os_resetSchedulingInformation(strategy);

}

/*!
 *  This is a getter for retrieving the current scheduling strategy.
 *
 *  \return The current scheduling strategy.
 */
SchedulingStrategy os_getSchedulingStrategy(void) {
//#warning IMPLEMENT STH. HERE
    return schedulingStrategy;
}

/*!
 *  Enters a critical code section by disabling the scheduler if needed.
 *  This function stores the nesting depth of critical sections of the current
 *  process (e.g. if a function with a critical section is called from another
 *  critical section) to ensure correct behaviour when leaving the section.
 *  This function supports up to 255 nested critical sections.
 */
void os_enterCriticalSection(void) {
	
	uint8_t gie = SREG & 0b10000000;
	cli();
	if (criticalSectionCount == UINT8_MAX) {
		SREG |= gie;
		os_errorPStr(PSTR("CritSec overflow"));
		return;
	}
	if (criticalSectionCount++ == 0) {
		TIMSK2 &= 0b111111101;
	}
	SREG |= gie;
	
//#warning IMPLEMENT STH. HERE
}

/*!
 *  Leaves a critical code section by enabling the scheduler if needed.
 *  This function utilizes the nesting depth of critical sections
 *  stored by os_enterCriticalSection to check if the scheduler
 *  has to be reactivated.
 */
void os_leaveCriticalSection(void){
	
	uint8_t gie = SREG & 0b10000000;
	cli();
	if (criticalSectionCount == 0) {
		SREG |= gie;
		os_errorPStr(PSTR("CritSec underflow"));
		return;
	}
	if (--criticalSectionCount == 0) {
		TIMSK2 |= 0b00000010;
	}
	SREG |= gie;


//#warning IMPLEMENT STH. HERE
}

/*!
 *  Calculates the checksum of the stack for a certain process.
 *
 *  \param pid The ID of the process for which the stack's checksum has to be calculated.
 *  \return The checksum of the pid'th stack.
 */
StackChecksum os_getStackChecksum(ProcessID pid) {
	uint8_t result = 0;
	
	uint8_t *lo = (uint8_t *)(os_processes[pid].sp.as_int + 1);
	uint8_t *hi = (uint8_t *)PROCESS_STACK_BOTTOM(pid);

	for (uint8_t *p = lo; p <= hi; ++p) {
		result ^= *p;
	}
	return (StackChecksum)result;
	
//#warning IMPLEMENT STH. HERE
    
}




void os_dispatcher(void) {
	
	ProcessID pid = os_getCurrentProc();
	
	os_getProcessSlot(pid) ->program();

	
	os_kill(pid);

	
	while (1) {
		
	}
}






bool os_kill(ProcessID pid){
	 os_enterCriticalSection();

	 if(pid == 0){
		 
		 os_leaveCriticalSection();
		 return false;
	 }
	 if(os_processes[pid].state == OS_PS_UNUSED || pid >= MAX_NUMBER_OF_PROCESSES){
		 //os_error("Error: os_kill() pid wrong");
		 os_leaveCriticalSection();
		 return false;
	 }
	 os_processes[pid].state = OS_PS_UNUSED;
	 if(pid == os_getCurrentProc()){
		 while (criticalSectionCount != 1){
			 os_leaveCriticalSection();
		 }
	 }
	 os_freeProcessMemory(intHeap, pid);
	 os_freeProcessMemory(extHeap, pid);
	 os_leaveCriticalSection();
	 while (currentProc == pid){
	 }
	 return true;
}