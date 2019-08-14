#ifndef CDFINGER_SIGNAL_H
#define CDFINGER_SIGNAL_H
#include <semaphore.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

extern sem_t g_down_sem;
void sig_handler(int signo);
int cfp_handler_register(void (*func)(int));
int64_t getTimeMsec();
int func_sem_timedwait(sem_t *g_down_sem);
unsigned long long cfp_get_uptime();

#endif