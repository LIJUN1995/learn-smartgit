#########################################################################
# File Name: excute_128.sh
# Author: ma6174
# mail: ma6174@163.com
# Created Time: 2018年11月12日 星期一 14时21分00秒
#########################################################################
#!/bin/bash
adb root
adb shell stop fingerprintd
adb shell /data/test_saveImg 128 128
