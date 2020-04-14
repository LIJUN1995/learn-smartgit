
#添加平台支持
APP_ABI := arm64-v8a

#添加C++ STL库支持
APP_STL := c++_static

#C++代码的编译选项。在android-ndk-1.5_r1版本中，此变量只适用于C++，但是现在可以同时适用于C和C＋＋。 
#APP_CPPFLAGS的别名，将来此变量将会被抛弃。
APP_CXXFLAGS += -fno-rtti
#打开异常控制支持
APP_CXXFLAGS += -fexceptions

#使用的ndk库函数版本号。一般和SDK的版本相对应，各个版本在NDK目录下的platforms文件夹中
APP_PLATFORM := android-27

APP_BUILD_SCRIPT := ./Android.mk
