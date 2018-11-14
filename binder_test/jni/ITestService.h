#ifndef ITestService_H
#define ITestService_H

#include <binder/IInterface.h>
#include <android/log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "testbinder"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG ,LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  ,LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN  ,LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  ,LOG_TAG, __VA_ARGS__)

namespace android {
    class ITestService: public IInterface {
        public:
            DECLARE_META_INTERFACE(TestService);
            virtual int setSomething(int a) = 0;
            virtual int getSomething() = 0;
    };
    
    class BnTestService: public BnInterface<ITestService> {
        public:
            virtual status_t onTransact(uint32_t cmd,
                                        const Parcel& data,
                                        Parcel* reply,
                                        uint32_t flags = 0);
    };
}

#endif