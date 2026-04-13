#ifndef SCHEDULER_H
#define SCHEDULER_H

/*
 * AstraOS Process Scheduler
 *
 * Stub interface for process/thread creation and scheduling.
 *
 * Future implementation will provide:
 *   - Process Control Block (PCB) data structure
 *   - Process creation (fork / exec stubs)
 *   - Round-robin scheduler driven by the timer IRQ
 *   - Context switch via TSS and stack manipulation
 *   - Process states: RUNNING, READY, BLOCKED, ZOMBIE
 */

#include <stdint.h>

/* Process identifier type */
typedef uint32_t pid_t;

/* Initialise the scheduler (no-op in v0.1 stub) */
void scheduler_init(void);

/*
 * Yield the CPU to the next ready process.
 * No-op until the scheduler is implemented.
 */
void scheduler_yield(void);

#endif /* SCHEDULER_H */
