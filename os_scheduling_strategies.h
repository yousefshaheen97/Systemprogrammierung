/*! \file
 *  \brief Scheduling library for the OS.
 *
 *  Contains the scheduling strategies.
 *
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */

#ifndef _OS_SCHEDULING_STRATEGIES_H
#define _OS_SCHEDULING_STRATEGIES_H

#include "defines.h"
#include "os_scheduler.h"
#include <stdbool.h>

//! Structure used to store specific scheduling informations such as a time slice
// This is a presence task

///////////////////////////////////////////////

typedef struct ProcessQueue {
	ProcessID data[MAX_NUMBER_OF_PROCESSES];
	uint8_t   size;
	uint8_t   head;
	uint8_t   tail;
} ProcessQueue;



typedef struct SchedulingInformation {
	uint8_t  timeSlice;
	Age      age[MAX_NUMBER_OF_PROCESSES];
	// MLFQ specific fields
	uint8_t timeSlices[MAX_NUMBER_OF_PROCESSES];  // Individual time slices for each process
	ProcessQueue queues[4];  // Four priority classes (0-3)
} SchedulingInformation;
//////////////////////////////////////////////


//! Used to reset the SchedulingInfo for one process
void os_resetProcessSchedulingInformation(ProcessID id);

//! Used to reset the SchedulingInfo for a strategy
void os_resetSchedulingInformation(SchedulingStrategy strategy);

//! Even strategy
ProcessID os_Scheduler_Even(const Process processes[], ProcessID current);

//! Random strategy
ProcessID os_Scheduler_Random(const Process processes[], ProcessID current);

//! RoundRobin strategy
ProcessID os_Scheduler_RoundRobin(const Process processes[], ProcessID current);

//! InactiveAging strategy
ProcessID os_Scheduler_InactiveAging(const Process processes[], ProcessID current);

//! RunToCompletion strategy
ProcessID os_Scheduler_RunToCompletion(const Process processes[], ProcessID current);

/////////////////////////////////////////////////////
void os_initSchedulingInformation(void);

//! Multilevel-Feedback-Queue strategy
ProcessID os_Scheduler_MLFQ(const Process processes[], ProcessID current);

//! ProcessQueue management functions
void pqueue_init(ProcessQueue *queue);
void pqueue_reset(ProcessQueue *queue);
bool pqueue_hasNext(const ProcessQueue *queue);
ProcessID pqueue_getFirst(const ProcessQueue *queue);
void pqueue_dropFirst(ProcessQueue *queue);
void pqueue_append(ProcessQueue *queue, ProcessID pid);
void pqueue_removePID(ProcessQueue *queue, ProcessID pid);

//! MLFQ helper functions
ProcessQueue* MLFQ_getQueue(uint8_t queueID);





#endif
