LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
		common.c \
		spi.c \
		test_cpld.c \
		cmos.c \
		i2c.c \
		cdfinger_signal.c \
		fb_test.c
#LOCAL_CFLAGS =-DRESIZE
LOCAL_MODULE:= test_cpld

include $(BUILD_EXECUTABLE)
