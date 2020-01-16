LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
    . \
    ./camera_ctl_file \

push:$(BIN)
LOCAL_SRC_FILES:= \
		./camera_ctl_file/fps6037_isf1001.c \
		./camera_ctl_file/fps6038_gc07s0.c \
        ./camera_ctl_file/fps7001_gc0403.c \
        ./camera_ctl_file/fps7001_gc0403_double.c \
        ./camera_ctl_file/fps7011_gcm7s0.c \
        cfp_mem.c \
		fb_test.c \
        $(FILE_NAME)
        # camera_qt.c \
		# # camera_qt.c \

LOCAL_MODULE:= $(DEMO_NAME)
# LOCAL_MODULE:= test_camera

include $(BUILD_EXECUTABLE)