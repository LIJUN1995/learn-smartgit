#APP_ABI := ${CFP_APP_ABI}
#APP_STL := gnustl_static
APP_CPPFLAGS += -fno-rtti
APP_ABI := arm64-v8a

#APP_STL := stlport_shared
#APP_OPP_STL := gnustl_static
#APP_CPPFLAGS := -frtti -fexceptions  

APP_PLATFORM := android-23
#APP_MODULES := libcdfinger_sec
#APP_OPP_STL := gnustl_static
#APP_BUILD_SCRIPT := ./Android.mk
