#ifndef TestService_h
#define TestService_h

#include "ITestService.h"
#include <binder/BinderService.h>

namespace android {
    class TestService: public BinderService<TestService>,public BnTestService {
        public:
            TestService();
            static const char* getServiceName() { return "TestService"; }
            virtual int setSomething(int a);
            virtual int getSomething();
        protected:
            int myParam;
    };
}

#endif