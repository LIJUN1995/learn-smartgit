/* ************************************************************************
 *       Filename:  rtc.c
 *    Description:  
 *        Version:  1.0
 *        Created:  2018年09月01日 16时38分09秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *        Company:  
 * ************************************************************************/



/*************************************************************************
	> File Name: rtc.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2018年09月01日 星期六 16时38分09秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/time.h>
#include<time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<linux/fb.h>
#include<linux/rtc.h>
#include <android/log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "test_rtc"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG ,LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  ,LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN  ,LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  ,LOG_TAG, __VA_ARGS__)

static int fd;
static struct rtc_time rtime;
void test_func(void)
{
	ioctl(fd,RTC_RD_TIME,&rtime);
	LOGD("%02d:%02d:%02d",rtime.tm_hour,rtime.tm_min,rtime.tm_sec);
}

void init_sigaction(void)
{
	struct sigaction act;
	
	act.sa_handler = (__sighandler_t)test_func;
	act.sa_flags = 0;

	//sigemptyset(&act.sa_mask);
    sigaction(SIGALRM, &act, NULL);
}

void init_time()
{
	struct itimerval val;
	val.it_value.tv_sec = 10;
	val.it_value.tv_usec = 0;

	val.it_interval = val.it_value;
	setitimer(ITIMER_REAL,&val,NULL);
}

// void get_framebuffer(void)
// {
// 	fd = open("/dev/graphics/fb0", O_RDWR);
// 	printf("fd = %d\n",fd);
// }

int main(int argc,char **argv)
{
	//get_framebuffer();
	fd = open("/dev/rtc0",O_RDWR);
	init_sigaction();
	init_time();

	while(1);
	// timer_t timerid;
	// sigevent_t sevp;
	// struct itimerspec time_count;
	// sigval_t val;
	// int ret = 0;

	// val.sival_int = 0;
	// sevp.sigev_value = val;
	// sevp.sigev_notify = SIGEV_THREAD;
	// sevp._sigev_un._sigev_thread._function = test_func;

	// time_count.it_value.tv_sec = 5;
	// time_count.it_interval = time_count.it_value;

	// ret = timer_create(CLOCK_REALTIME,&sevp,&timerid);
	// if(ret != 0) {
	// 	printf("create timer failed!\n");
	// 	return ret;
	// }
	// ret = timer_settime(timerid,0,&time_count,NULL);
	// if(ret != 0) {
	// 	printf("set timer failed!\n");
	// 	return ret;
	// }

	// while(1)
	// {
	// 	if(val.sival_int > 10)
	// 		break;
	// 	sleep(100);
	// }
	
	// ret = timer_delete(timerid);
	// if(ret != 0) {
	// 	printf("delete timer failed!\n");
	// 	return ret;
	// }

	return 0;
}

