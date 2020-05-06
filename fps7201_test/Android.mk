LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
    . \

push:$(BIN)
LOCAL_SRC_FILES:= \
        ./fpsxxxx.cpp \
        ./fps7011_gcm7s0.cpp \
        ./fps6038_gc07s0.cpp \
        ./BmpClass.cpp \
        ./DevicesOps.cpp \
        $(FILE_NAME)

LOCAL_MODULE:= $(DEMO_NAME)

include $(BUILD_EXECUTABLE)