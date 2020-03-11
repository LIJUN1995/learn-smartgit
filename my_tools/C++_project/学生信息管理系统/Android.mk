LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

push:$(BIN)
LOCAL_SRC_FILES:= main.cpp \
					Student.cpp \
					StudentManage.cpp \
					StudentView.cpp

LOCAL_MODULE:= main

include $(BUILD_EXECUTABLE)