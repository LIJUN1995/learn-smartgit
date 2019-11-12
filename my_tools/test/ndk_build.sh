#!/bin/bash

#set -e

${ANDROID_NDK_HOME}/ndk-build \
    -B \
    NDK_PROJECT_PATH=. \
    NDK_APPLICATION_MK=./Application.mk \
    NDK_LIBS_OUT=./out \
    NDK_APP_OUT=./out \
    NDK_DEBUG=1