/* ************************************************************************
 *       Filename:  service.cpp
 *    Description:  
 *        Version:  1.0
 *        Created:  2018年08月21日 10时30分32秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *        Company:  
 * ************************************************************************/



/*************************************************************************
	> File Name: service.cpp
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2018年08月21日 星期二 10时30分32秒
 ************************************************************************/
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <utils/threads.h>
#include "TestService.h"

namespace android {
	TestService::TestService() {
		myParam = 0;
	}
	int TestService::setSomething(int a) {
		LOGD("TestService::setSomething a = %d myParam %d",a,myParam);
		myParam += a;
		return 0;
	}
	int TestService::getSomething() {
		LOGD("TestService::getSomething myParam = %d",myParam);
		return myParam;
	}
}