#include <stdio.h>
#include <binder/IServiceManager.h>
#include "ITestService.h"

#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif

#define LOG_NDEBUG 0

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "testbinder"

using namespace android;
sp<ITestService> mTestService;
void initTestServiceClient() {
    int count = 10;
    if(mTestService == 0) {
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;
        do {
            binder = sm->getService(String16("TestService"));
            if(binder != 0)
                break;
            LOGW("TestService not published,wait...");
            sleep(1);
            count++;
        }while(count < 20);
        mTestService = interface_cast<ITestService>(binder);
    }
}

int main(int argc,char* argv[]) {
    initTestServiceClient();
    if(mTestService == NULL) {
        LOGW("cannot find TestService");
        return 0;
    }
    while(1) {
        mTestService->setSomething(1);
        sleep(1);
        LOGD("getSomething %d",mTestService->getSomething());
    }

    return 0;
}