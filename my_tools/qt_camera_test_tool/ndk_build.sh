#!/bin/bash

#set -e

${ANDROID_NDK_HOME}/ndk-build \
    NDK_DEBUG=0 \
    NDK_PROJECT_PATH=. \
    NDK_APPLICATION_MK=./Application.mk \
    NDK_LIBS_OUT=./out \
    NDK_APP_OUT=./out
    
# adb forward tcp:12345 tcp:12345

# adb push /mnt/d/wsl_linux/vitality/test_sensor/out/arm64-v8a/gdbserver /data

# adb shell chmod 777 /data/gdbserver

# adb push /mnt/d/wsl_linux/vitality/test_sensor/out/local/arm64-v8a/test_demo /data

# adb shell chmod 777 /data/test_demo

# adb shell /data/gdbserver :12345 /data/test_demo
