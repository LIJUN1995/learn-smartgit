#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
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


/*
 * This expects the new RTC class driver framework, working with
 * clocks that will often not be clones of what the PC-AT had.
 * Use the command line to specify another RTC if you need one.
 */
static const char default_rtc[] = "/dev/rtc0";

int main(int argc, char **argv)
{
	int fd, retval, irqcount = 0;
    int i = 0;
	unsigned long tmp, data;
	struct rtc_time rtc_tm;
	const char *rtc = default_rtc;
	struct timeval start, end, diff;
do{
	fd = open(rtc, O_RDONLY);
    if (fd ==  -1) {
		perror(rtc);
		exit(errno);
	}

	LOGD( "RTC Driver Test Example.");

	/* Read the RTC time/date */
            retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
            if (retval == -1) {
                perror("RTC_RD_TIME ioctl");
                exit(errno);
            }

            LOGD("Current RTC date/time is %d-%d-%d, %02d:%02d:%02d.",
            rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
            rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

        /* Set the alarm to 5 sec in the future, and check for rollover */
            rtc_tm.tm_sec += 5;
            if (rtc_tm.tm_sec >= 60) {
                rtc_tm.tm_sec %= 60;
                rtc_tm.tm_min++;
            }
            if (rtc_tm.tm_min == 60) {
                rtc_tm.tm_min = 0;
                rtc_tm.tm_hour++;
            }
            if (rtc_tm.tm_hour == 24)
                rtc_tm.tm_hour = 0;

            retval = ioctl(fd, RTC_ALM_SET, &rtc_tm);
            if (retval == -1) {
                if (errno == EINVAL) {
                    LOGD("...Alarm IRQs not supported.");
                    return 0;
                }
                perror("RTC_ALM_SET ioctl");
                exit(errno);
            }

        /* Read the current alarm settings */
            retval = ioctl(fd, RTC_ALM_READ, &rtc_tm);
            if (retval == -1) {
                perror("RTC_ALM_READ ioctl");
                exit(errno);
            }

            LOGD("Alarm time now set to %02d:%02d:%02d.",
                rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

        /* Enable alarm interrupts */
            retval = ioctl(fd, RTC_AIE_ON, 0);
            if (retval == -1) {
                if (errno == EINVAL) {
                    LOGD("...Alarm IRQs not supported.");
                    return -1;
                }

                perror("RTC_AIE_ON ioctl");
                exit(errno);
            }

             LOGD( "Waiting 5 seconds for alarm...");

        /* This blocks until the alarm ring causes an interrupt */
            retval = read(fd, &data, sizeof(unsigned long));
            if (retval == -1) {
                perror("read");
                exit(errno);
            }
            printf("time now  is  %02d:%02d:%02d.",
            rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

            i++;

             /* Disable alarm interrupts */
            retval = ioctl(fd, RTC_AIE_OFF, 0);
            if (retval == -1) {
                perror("RTC_AIE_OFF ioctl");
                exit(errno);
            }

	close(fd);
    }while(i <= 36);

	return 0;
}
