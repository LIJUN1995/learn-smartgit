#include <stdio.h>
#include "cmos.h"
#include <sys/time.h>
       #include <sys/resource.h>
#include <sched.h>
#include "common.h"
int main(void)
{
    int ret = 0;
    unsigned char order[16] = {0};
    struct sched_param param;

    ret = OpenDeviceFile("/dev/fpsdev0");
    if(ret<0){
        return ret;
    }
    DeviceInit();

    while(1){
        printf("\n\n\n");
        printf("**************************************************\n");
        printf("* Update background image please enter:        1 *\n");
        printf("* get pictures please enter:                   2 *\n");
        printf("* cpld_test please enter:                      3 *\n");
        printf("* change exposure time please enter:           4 *\n");
        printf("* download cpld bin please enter:              6 *\n");
        printf("* check sensor id and sensor init please enter:7 *\n");
        printf("* end programmer please enter:                 q *\n");
        printf("* program flash programmer please enter:       p *\n");
        printf("**************************************************\n");
        printf("\n\n\n");
        printf("please intput the order:");
        
        do{
            ret = scanf("%s", order);
            if(ret < 0){
                printf("scanf printf error!\n");
            }
        }while(ret < 0);
        
        printf("==================%s\n",order);

        if(strcmp(order,"q") == 0){
            break;
        }else{
            switch(atoi(order)){
                case 1:{
                    checkUpdate();
                    break;
                }
                case 2:{
                    get_pictures();
                    break;
                }
                case 3:{
                    cpld_test();
                    break;
                }
                case 4:{
                    close_cpld1();
                    break;
                }
                case 5:{
                    close_cpld2();
                    break;
                }
                case 6:{
                    ProgrammingRawCode();
                    break;
                }
                case 7:{
                    cmos_init();
                    break;
                }
                case 8:{
                    // set_reg();
                    break;
                }
                case 9:{
                    programming_NVCM();
                    break;
                }
                default:{
                    printf("wrong input!please enter 1,2or3 to choose the function you want!\n");
                    break;
                }
            }
        }
    }
    CloseDeviceFile();
    return 0;
}
