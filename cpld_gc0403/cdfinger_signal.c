#include "cdfinger_signal.h"
#include <signal.h>
#include <fcntl.h>
#include <linux/in.h>
#include <errno.h>
#include "test_cpld.h"
#include <stdlib.h>
#include <string.h>
sem_t g_down_sem;

void sig_handler(int signo) {
	if (signo == SIGIO) {
		sem_post(&g_down_sem);
	}
	if(signo == SIGINT){
		exit(0);
	}
}

int cfp_handler_register(void (*func)(int)) {
	struct sigaction action;
	if (func == NULL) {
		printf("Func to register is NULL.\n");
		return -1;
	}
	memset(&action, 0, sizeof(action));
	action.sa_handler = func;
	action.sa_flags = 0;
	sigaction(SIGIO, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	fcntl(m_fd, F_SETOWN, getpid());
	fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL) | FASYNC);
	return 0;
}

int64_t getTimeMsec() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int func_sem_timedwait(sem_t *g_down_sem) {
	struct timespec timewait;
	timewait.tv_sec = getTimeMsec() / 1000 + 3;
	timewait.tv_nsec = 0;
	int ret = sem_timedwait(g_down_sem, &timewait);
	return ret;
}

unsigned long long cfp_get_uptime() {
	struct timeval cur;
	long long int timems = 0;
	gettimeofday(&cur, NULL);
	timems = (long long int) (cur.tv_sec * 1000 + (cur.tv_usec) / 1000);
	return timems;
}

