#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <semaphore.h>
#include "fpsxxxx.h"
#include "fps7011_gcm7s0.h"
#include "BmpClass.h"
#include "DevicesOps.h"

using namespace std;

int main(int argc, char *argv[])
{
    uint16_t *p;
    int ret = 0;
    int width = 240;
    int height = 160;
    DevicesOps dev;
    BmpOps bmpOps;
    vector<uint16_t> vec;
    vector<uint8_t> vec1;

    Fps7011 *fps = NULL;
    try {
        fps = new Fps7011;
    }catch (const char* msg) {
        cerr << msg << endl;
        goto OUT;
    }

    fps->sensor_test_otp();

/*    fps->sensor_init();
    fps->sensor_pre_image();
    dev.WaitSemCall(3);
    fps->sensor_setImgWH(0,0,width,height);
    fps->sensor_get_img_buffer(vec);
    
    p = (uint16_t *)&vec[0];
    for (size_t i = 0; i < vec.size(); i++)
    {
        // cout<<hex<<int(p[i])<<endl;
        vec1.push_back(uint8_t(p[i]>>4));
    }

    bmpOps.saveGrayBitmap("0.bmp",&vec1[0],height,width);
 */   
OUT:
    delete fps;

    return 0;
}