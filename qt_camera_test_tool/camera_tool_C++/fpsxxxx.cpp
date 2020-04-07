#include <iostream>

#include <sys/ioctl.h>

// #ifdef __cplusplus
// extern "C"{
// #endif
// 　　#include <sys/ioctl.h>
// #ifdef __cplusplus
// };
// #endif

#include "fpsxxxx.h"

using namespace std;

int  Cdfinger_fops::spi_send_data(unsigned char *tx, unsigned char *rx, int len)
{
    int ret = -1;
    Cdfinger_spi_data *data = new Cdfinger_spi_data;

    data->rx = rx;
    data->tx = tx;
    data->length = len;
    data->tx_length = len;
    ret = ioctl(private_fd, CDFINGER_SPI_WRITE_AND_READ, data);
    if (ret != 0) {
        cout << "spi send data is error!!! ret = " << ret << endl;
    }

    delete data;
    return ret;
}

int Cdfinger_fops::get_private_fd(void)
{
    return private_fd;
}