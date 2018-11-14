#########################################################################
# File Name: chang_bg.sh
# Author: ma6174
# mail: ma6174@163.com
# Created Time: 2018年11月12日 星期一 14时15分28秒
#########################################################################
#!/bin/bash
adb root
adb shell rm /data/system/cdfinger/bg /data/system/cdfinger/caliPara
adb shell cp /data/picture/0.bin /data/system/cdfinger/bg
adb reboot
