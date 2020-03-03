/**
*****************************************************************************
**
**  File        : syscalls.c
**
**  Abstract    : Atollic TrueSTUDIO Minimal System calls file
**
** 		          For more information about which c-functions
**                need which of these lowlevel functions
**                please consult the Newlib libc-manual
**
**  Environment : Atollic TrueSTUDIO
**
**  Distribution: The file is distributed as is,without any warranty
**                of any kind.
**
**  (c)Copyright Atollic AB.
**  You may use this file as-is or modify it according to the needs of your
**  project. Distribution of this file (unmodified or modified) is not
**  permitted. Atollic AB permit registered Atollic TrueSTUDIO(R) users the
**  rights to distribute the assembled, compiled & linked contents of this
**  file as part of an application binary file, provided that it is built
**  using the Atollic TrueSTUDIO(R) Pro toolchain.
**
*****************************************************************************
*/

/* Includes */
#include "main.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
register char * stack_ptr asm("sp");

void initialise_monitor_handles()
{

}

int _getpid(void)
{
	return 1;
}

int _kill(int pid, int sig)
{
	return -1;
}

int _read (int file, char *ptr, int len)
{

 return len;
}

int _write(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
    usart_data_transmit(UART3, (uint8_t)ptr[DataIdx]);
    while(RESET == usart_flag_get(UART3, USART_FLAG_TBE));
	}
	return len;
}

caddr_t _sbrk(int incr)
{
	extern char end asm("end");
	static char *heap_end;
	char *prev_heap_end;
	if (heap_end == 0)
		heap_end = &end;
	prev_heap_end = heap_end;
	if (heap_end + incr > stack_ptr)
	{
	errno = ENOMEM;
	return (caddr_t) -1;
	}
	heap_end += incr;
	return (caddr_t) prev_heap_end;
}
#if 0
caddr_t _sbrk(int incr)
{
	//extern char end asm("end");
	extern char end asm("_end");
	static char *heap_end;
	char *prev_heap_end;
	if (heap_end == 0)
		heap_end = &end;
	prev_heap_end = heap_end;
	if (heap_end + incr > stack_ptr)
	{
	errno = ENOMEM;
	return (caddr_t) -1;
	}
	heap_end += incr;
	return (caddr_t) prev_heap_end;
}
#endif

int _close(int file)
{
	return -1;
}


int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}

int _open(char *path, int flags, ...)
{
	/* Pretend like we always fail */
	return -1;
}

int _wait(int *status)
{
	return -1;
}
#if 0
#pragma import(__use_no_semihosting) 
void _exit(int status)
{

}
struct __FILE
{
  int handle;
};
FILE __stdout;
#endif

int _unlink(char *name)
{
	return -1;
}

int _times(struct tms *buf)
{
	return -1;
}

int _stat(char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _link(char *old, char *new)
{
	return -1;
}

int _fork(void)
{
	return -1;
}

int _execve(char *name, char **argv, char **env)
{
	return -1;
}

