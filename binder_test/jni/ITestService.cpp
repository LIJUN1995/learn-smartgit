#include "ITestService.h"
#include <binder/Parcel.h>
#include <binder/IInterface.h>
#include <utils/Log.h>

#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif
#define LOG_NDEBUG 0

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "testbinder"

using namespace android;
enum {
    SET_SOMETHING = IBinder::FIRST_CALL_TRANSACTION,
    GET_SOMETHING,
};

//----------------proxy side-----------------------------

class BpTestService: public BpInterface<ITestService> {
    public:
        BpTestService(const sp<IBinder>& impl): BpInterface<ITestService>(impl) {
            /*构造函数*/
        }
        virtual int setSomething(int a) {
            LOGD("BpTestService::setSomething a = %d",a);
            Parcel data,reply;
            data.writeInt32(a);
            remote()->transact(SET_SOMETHING,data,&reply);
            return reply.readInt32();
        }
        virtual int getSomething() {
            LOGD("BpTestService::getSomething");
            Parcel data,reply;
            data.writeInterfaceToken(ITestService::getInterfaceDescriptor());
            remote()->transact(GET_SOMETHING,data,&reply);
            return reply.readInt32();
        }
};
//----------------------interface---------------------------

IMPLEMENT_META_INTERFACE(TestService,"chenxf.binder.ITestService");
//IMPLEMENT_META_INTERFACE(TestService,"ITestService");

//------------------------service side-----------------------

status_t BnTestService::onTransact (uint32_t cmd,const Parcel& data,
                                    Parcel* reply,uint32_t flags) {
    switch(cmd) {
        case SET_SOMETHING: {
            LOGD("BnTestService::onTransact SET_SOMETHING");
            reply->writeInt32(setSomething((int)data.readInt32()));
            return NO_ERROR;
        } break;
        case GET_SOMETHING: {
            LOGD("BnTestService::onTransact GET_SOMETHING");
            reply->writeInt32(getSomething());
            return NO_ERROR;
        } break;
    }
    return BBinder::onTransact(cmd,data,reply,flags);
}