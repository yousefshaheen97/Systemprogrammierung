/*! \file

Scheduling strategies used by the Interrupt Service RoutineA from Timer 2 (in scheduler.c)
to determine which process may continue its execution next.

The file contains five strategies:
-even
-random
-round-robin
-inactive-aging
-run-to-completion
*/

#include "os_scheduling_strategies.h"
#include "defines.h"

#include <stdlib.h>

/*!
 *  Reset the scheduling information for a specific strategy
 *  This is only relevant for RoundRobin and InactiveAging
 *  and is done when the strategy is changed through os_setSchedulingStrategy
 *
 *  \param strategy  The strategy to reset information for
 */


extern Process os_processes[MAX_NUMBER_OF_PROCESSES];

SchedulingInformation schedulingInfo;
/////////////////////////////////////////////

extern SchedulingStrategy schedulingStrategy;


#define MLFQ_NUM_QUEUES 4
#define MLFQ_TIME_SLICES {1, 2, 4, 8}  




///////////////////

void os_resetSchedulingInformation(SchedulingStrategy strategy) {
	if (strategy == OS_SS_ROUND_ROBIN) {
		schedulingInfo.timeSlice = os_getProcessSlot(os_getCurrentProc())->priority;
		} else if (strategy == OS_SS_INACTIVE_AGING) {
		for (int i = 0; i < MAX_NUMBER_OF_PROCESSES; i++) {
			schedulingInfo.age[i] = 0;
			}
		}  else if (strategy == OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE) {
		// Alle Queues leeren
		for (int i = 0; i < MLFQ_NUM_QUEUES; i++) {
			pqueue_reset(&schedulingInfo.queues[i]);
		}
		// Zeitscheiben zurücksetzen
		for (int i = 0; i < MAX_NUMBER_OF_PROCESSES; i++) {
			schedulingInfo.timeSlices[i] = 0;
		}
		// prozesse sortieren
		static const uint8_t timeSlices[MLFQ_NUM_QUEUES] = {1, 2, 4, 8};
		
		for (ProcessID pid = 1; pid < MAX_NUMBER_OF_PROCESSES; pid++) {
			Process* process = os_getProcessSlot(pid);
			if (process->state == OS_PS_READY) {
				uint8_t queueID = (process->priority >> 6) & 0x03;
				pqueue_append(&schedulingInfo.queues[queueID], pid);
				schedulingInfo.timeSlices[pid] = timeSlices[queueID];
			}
		}
	}
}

/*!
 *  Reset the scheduling information for a specific process slot
 *  This is necessary when a new process is started to clear out any
 *  leftover data from a process that previously occupied that slot
 *
 *  \param id  The process slot to erase state for
 */
void os_resetProcessSchedulingInformation(ProcessID id) {
	schedulingInfo.age[id] = 0;
	schedulingInfo.timeSlices[id] = 0;
	
	if (schedulingStrategy == OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE) {
		Process* process = os_getProcessSlot(id);
		if (process && process->state == OS_PS_READY) {
			
			uint8_t queueID = (process->priority >> 6) & 0x03;  
			ProcessQueue* queue = MLFQ_getQueue(queueID);
			if (queue) {
				pqueue_append(queue, id);
				
				static const uint8_t timeSlices[MLFQ_NUM_QUEUES] = {1, 2, 4, 8};
				schedulingInfo.timeSlices[id] = timeSlices[queueID];
			}
		}
	}
}

///////////////////////////////////



void os_initSchedulingInformation(void) {
	// inittialisiere queue
	for (int i = 0; i < MLFQ_NUM_QUEUES; i++) {
		pqueue_init(&schedulingInfo.queues[i]);
	}
}



void pqueue_init(ProcessQueue *queue) {
	queue->size = MAX_NUMBER_OF_PROCESSES;
	queue->head = 0;
	queue->tail = 0;
}

void pqueue_reset(ProcessQueue *queue) {
	queue->head = 0;
	queue->tail = 0;
}

bool pqueue_hasNext(const ProcessQueue *queue) {
	return queue->head != queue->tail;
}

ProcessID pqueue_getFirst(const ProcessQueue *queue) {
	if (!pqueue_hasNext(queue)) {
		return INVALID_PROCESS;
	}
	return queue->data[queue->tail];
}

void pqueue_dropFirst(ProcessQueue *queue) {
	if (pqueue_hasNext(queue)) {
		queue->tail = (queue->tail + 1) % queue->size;
	}
}

void pqueue_append(ProcessQueue *queue, ProcessID pid) {
	queue->data[queue->head] = pid;
	queue->head = (queue->head + 1) % queue->size;
}

void pqueue_removePID(ProcessQueue *queue, ProcessID pid) {
	if (!pqueue_hasNext(queue)) {
		return;
	}
	
	
	uint8_t position = queue->tail;
	uint8_t found_position = queue->tail;
	bool found = false;
	
	while (position != queue->head) {
		if (queue->data[position] == pid) {
			found_position = position;
			found = true;
			break;
		}
		position = (position + 1) % queue->size;
	}
	
	if (!found) {
		return;
	}
	
	
	while (found_position != queue->head) {
		uint8_t next_position = (found_position + 1) % queue->size;
		queue->data[found_position] = queue->data[next_position];
		found_position = next_position;
	}
	
	
	queue->head = (queue->head - 1 + queue->size) % queue->size;
}

ProcessQueue* MLFQ_getQueue(uint8_t queueID) {
	if (queueID >= MLFQ_NUM_QUEUES) {
		return NULL;
	}
	return &schedulingInfo.queues[queueID];
}


////////////////////////////

/*!
 *  This function implements the even strategy. Every process gets the same
 *  amount of processing time and is rescheduled after each scheduler call
 *  if there are other processes running other than the idle process.
 *  The idle process is executed if no other process is ready for execution
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the even strategy.
 */
ProcessID os_Scheduler_Even(const Process processes[], ProcessID current) {
	
	// zähle ready proc
	uint8_t totalReady = 0;
	for (ProcessID pid = 0; pid < MAX_NUMBER_OF_PROCESSES; pid++) {
		if (processes[pid].state == OS_PS_READY || processes[pid].state == OS_PS_BLOCKED) {
			totalReady++;
		} 
	}
	// Wenn nur idle ready, wähle idle
	if (totalReady == 1 && processes[0].state == OS_PS_READY) {
		return 0;
	}
	ProcessID  pid = current;
	// suche nächste ready proc und proc!=0
	for (uint8_t offset = 0; offset < MAX_NUMBER_OF_PROCESSES; offset++) {
		pid = (pid + 1) % MAX_NUMBER_OF_PROCESSES;
		if (pid != 0 && processes[pid].state == OS_PS_READY) {
			return pid;
		} if (processes[pid].state == OS_PS_BLOCKED){
			return pid;
		}
	}
	return 0;
//#warning IMPLEMENT STH. HERE
}

/*!
 *  This function implements the random strategy. The next process is chosen based on
 *  the result of a pseudo random number generator.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the random strategy.
 */
ProcessID os_Scheduler_Random(const Process processes[], ProcessID current) {
	
	// Zähle READY > 0
    uint8_t ready_count = 0;
    for (ProcessID pid = 1; pid < MAX_NUMBER_OF_PROCESSES; pid++) {
        if (processes[pid].state == OS_PS_READY || processes[pid].state == OS_PS_BLOCKED) {
            ready_count++;
        } 
    }
    if (ready_count == 0) {
        return 0;
    }
    // choosee rand num
    uint16_t r = rand() % ready_count;
    // Finde r-tes READY > 0
    for (ProcessID pid = 1; pid < MAX_NUMBER_OF_PROCESSES; pid++) {
        if (processes[pid].state == OS_PS_READY) {
            if (r == 0) {
				if (os_processes[pid].state == OS_PS_BLOCKED)
				{
					os_processes[pid].state = OS_PS_READY;
				}
                return pid;
            }
            r--;
        } 
    }
	for (uint8_t i = 1; i<MAX_NUMBER_OF_PROCESSES ; i++)
	{
		if (os_processes[i].state == OS_PS_BLOCKED)
		{
			os_processes[i].state = OS_PS_READY;
		}
	}
    
    return 0;
	
	
//#warning IMPLEMENT STH. HERE
    
}

/*!
 *  This function implements the round-robin strategy. In this strategy, process priorities
 *  are considered when choosing the next process. A process stays active as long its time slice
 *  does not reach zero. This time slice is initialized with the priority of each specific process
 *  and decremented each time this function is called. If the time slice reaches zero, the even
 *  strategy is used to determine the next process to run.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the round robin strategy.
 */
ProcessID os_Scheduler_RoundRobin(const Process processes[], ProcessID current) {
    // run curr bis ende der timeSlice
    if (processes[current].state == OS_PS_READY && current != 0 && schedulingInfo.timeSlice > 1) {
	    schedulingInfo.timeSlice--;
	    return current;
    }

    // sonst suche nächte ready process
    ProcessID next = current;
    uint8_t steps = MAX_NUMBER_OF_PROCESSES;
    while (steps--) {
	    next = (next + 1) % MAX_NUMBER_OF_PROCESSES;
	    if (next != 0 && processes[next].state == OS_PS_READY) {
		    // reset timeSlice
		    schedulingInfo.timeSlice = processes[next].priority;
		    return next;
	    } else if (processes[next].state == OS_PS_BLOCKED) {
	    continue;
    }
    }
	for (uint8_t i = 1; i<MAX_NUMBER_OF_PROCESSES ; i++)
	{
		if (os_processes[i].state == OS_PS_BLOCKED)
		{
			os_processes[i].state = OS_PS_READY;
		}
	}
    return 0;
}

/*!
 *  This function realizes the inactive-aging strategy. In this strategy a process specific integer ("the age") is used to determine
 *  which process will be chosen. At first, the age of every waiting process is increased by its priority. After that the oldest
 *  process is chosen. If the oldest process is not distinct, the one with the highest priority is chosen. If this is not distinct
 *  as well, the one with the lower ProcessID is chosen. Before actually returning the ProcessID, the age of the process who
 *  is to be returned is reset to 0.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the inactive-aging strategy.
 */
ProcessID os_Scheduler_InactiveAging(const Process processes[], ProcessID current) {
    // Alter ready procces erhöhen
    for (ProcessID i = 1; i < MAX_NUMBER_OF_PROCESSES; i++) {
	    if (processes[i].state == OS_PS_READY) {
		    schedulingInfo.age[i] += processes[i].priority;
	    } 
    }

    // Ältesten Prozess suchen
    ProcessID selected    = INVALID_PROCESS;
    Age      highestAge   = 0;
    Priority highestPrio  = 0;
    for (ProcessID i = 1; i < MAX_NUMBER_OF_PROCESSES; i++) {
	    if (processes[i].state == OS_PS_BLOCKED)
	    continue;
	    if (processes[i].state != OS_PS_READY)
	    continue;
		Age      a = schedulingInfo.age[i];
	    Priority p = processes[i].priority;

	    if (selected == INVALID_PROCESS) {
		    // erster Kandidat
		    selected    = i;
		    highestAge  = a;
		    highestPrio = p;
	    }
	    else if (a > highestAge) {
		    selected    = i;
		    highestAge  = a;
		    highestPrio = p;
	    }
	    else if (a == highestAge) {
		    if (p > highestPrio) {
			    selected    = i;
			    highestPrio = p;
		    }
		    else if (p == highestPrio && i < selected) {
			    selected = i;
		    }
	    }
    }
	if (processes[current].state == OS_PS_BLOCKED && highestAge == 0){
		schedulingInfo.age[current] = 0;
		return current;
	}
	

    // Kein ready proc dann Idle
    if (selected == INVALID_PROCESS) {
	    return 0;
    }

    // Alter zurücksetzen 
    schedulingInfo.age[selected] = 0;
    return selected;
}

/*!
 *  This function realizes the run-to-completion strategy.
 *  As long as the process that has run before is still ready, it is returned again.
 *  If  it is not ready, the even strategy is used to determine the process to be returned
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the run-to-completion strategy.
 */
ProcessID os_Scheduler_RunToCompletion(const Process processes[], ProcessID current) {
    if (processes[current].state == OS_PS_READY) {
	    return current;
    }
    return os_Scheduler_Even(processes, current);
}



//////////////////////////////////////////////////////////////
ProcessID os_Scheduler_MLFQ(Process const  processes[], ProcessID current) {
	static const uint8_t timeSlices[MLFQ_NUM_QUEUES] = {1, 2, 4, 8};
	
	
	if (current != 0 && processes[current].state == OS_PS_READY && schedulingInfo.timeSlices[current] > 0) {
		schedulingInfo.timeSlices[current]--;
		return current;
	}
	
	if (current != 0 && processes[current].state == OS_PS_READY) {
		uint8_t curQ = 0;
		bool found = false;
		for (uint8_t q = 0; q < MLFQ_NUM_QUEUES; q++) {
			ProcessQueue *queue = MLFQ_getQueue(q);
			if (!queue) continue;
			uint8_t pos = queue->tail;
			while (pos != queue->head) {
				if (queue->data[pos] == current) {
					curQ = q;
					found = true;
					break;
				}
				pos = (pos + 1) % queue->size;
			}
			if (found) break;
		}
		if (found) {
			ProcessQueue *oldQ = MLFQ_getQueue(curQ);
			pqueue_removePID(oldQ, current);
			if (schedulingInfo.timeSlices[current] > 0) {
				pqueue_append(oldQ, current);
				} else {
				uint8_t nextQ = (curQ + 1) % MLFQ_NUM_QUEUES;
				ProcessQueue *newQ = MLFQ_getQueue(nextQ);
				pqueue_append(newQ, current);
				schedulingInfo.timeSlices[current] = timeSlices[nextQ];
			}
		}
	}
	
	for (ProcessID pid = 1; pid < MAX_NUMBER_OF_PROCESSES; pid++) {
		if (processes[pid].state != OS_PS_READY) continue;
		bool inQ = false;
		for (uint8_t q = 0; q < MLFQ_NUM_QUEUES; q++) {
			ProcessQueue *queue = MLFQ_getQueue(q);
			if (!queue) continue;
			uint8_t pos = queue->tail;
			while (pos != queue->head) {
				if (queue->data[pos] == pid) {
					inQ = true;
					break;
				}
				pos = (pos + 1) % queue->size;
			}
			if (inQ) break;
		}
		if (!inQ) {
			uint8_t qid = (processes[pid].priority >> 6) & 0x03;
			ProcessQueue *queue = MLFQ_getQueue(qid);
			pqueue_append(queue, pid);
			schedulingInfo.timeSlices[pid] = timeSlices[qid];
		}
	}
	
	ProcessID chosen = INVALID_PROCESS;
	uint8_t chosenQ = 0;
	for (uint8_t q = 0; q < MLFQ_NUM_QUEUES; q++) {
		ProcessQueue *queue = MLFQ_getQueue(q);
		if (queue && pqueue_hasNext(queue)) {
			chosen = pqueue_getFirst(queue);
			chosenQ = q;
			break;
		}
	}
	
	if (chosen == INVALID_PROCESS) {
		for (ProcessID pid = 1; pid < MAX_NUMBER_OF_PROCESSES; pid++) {
			if (processes[pid].state == OS_PS_READY) {
				return pid;
			}
		}
		return 0;
	}
	
	ProcessQueue *queue = MLFQ_getQueue(chosenQ);
	pqueue_dropFirst(queue);
	schedulingInfo.timeSlices[chosen] = timeSlices[chosenQ];
	
	return chosen;
}