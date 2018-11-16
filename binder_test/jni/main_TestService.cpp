#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include "TestService.h"

#define EASY_START_BINDER_SERVICE 1

using namespace android;

int main(int argc,char** argv)
{
    #if EASY_START_BINDER_SERVICE
        TestService::publishAndJoinThreadPool();
    #else
        sp<ProcessState> Proc(ProcessState::self());
        sp<IServiceManager> sm = defaultServiceManager();
        sm->addService(String16(TestService::getServiceName()),new TestService());
        ProcessState::self()->startThreadPool();
        IPCThreadState::self()->joinThreadPool();
    #endif

    return 0;
}
