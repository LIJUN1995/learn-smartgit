LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
MY_LIB_DIR := $(LOCAL_PATH)/../../finger_code/lib64_so
# LOCAL_SHARED_LIBRARIES := \
# 	libcutils \
# 	libbinder \
# 	libutils \
# 	libhardware \
# 	libc++ \
# 	libbacktrace \
# 	libbase \
# 	libunwind

#this variable is that should be linked to this module's static library list
#when include $(BUILD_STATIC_LIBRARY)
#which only makes sense for shared library module


LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/platform/inc/native/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/platform/inc/system/core/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/platform/inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/platform/hardware/libhardware/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/algoandroid/common/inc
LOCAL_LDFLAGS := $(MY_LIB_DIR)/libbinder.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libcutils.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libutils.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libhardware.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libc++.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libbacktrace.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libbase.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libunwind.so


#LOCAL_SRC_FILES:= ITestService.cpp \
#				  main_client.cpp
LOCAL_SRC_FILES:= rtc.c
LOCAL_MODULE_TAGS = eng tests
#LOCAL_MODULE:= testClient
LOCAL_MODULE:= rtc
include $(BUILD_EXECUTABLE)


#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


include $(CLEAR_VARS)
MY_LIB_DIR := $(LOCAL_PATH)/../../finger_code/lib64_so
# LOCAL_SHARED_LIBRARIES := \
# 	libcutils \
# 	libbinder \
# 	libutils \
# 	libhardware \
# 	libc++ \
# 	libbacktrace \
# 	libbase \
# 	libunwind

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/platform/inc/native/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/platform/inc/system/core/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/platform/inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/platform/hardware/libhardware/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../finger_code/algoandroid/common/inc
LOCAL_LDFLAGS := $(MY_LIB_DIR)/libbinder.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libcutils.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libutils.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libhardware.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libc++.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libbacktrace.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libbase.so
LOCAL_LDFLAGS += $(MY_LIB_DIR)/libunwind.so


LOCAL_SRC_FILES:= ITestService.cpp \
				  TestService.cpp \
				  main_TestService.cpp
				  
LOCAL_MODULE:= testServer
LOCAL_MODULE_TAGS = eng tests
include $(BUILD_EXECUTABLE) 
