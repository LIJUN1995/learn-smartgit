#ifndef FINGERTECH_EXIT_H_
#define FINGERTECH_EXIT_H_

//中断组设置
//NVIC_PRIORITYGROUP_0: 0 位抢占优先级
//                      4 位子优先级
//NVIC_PRIORITYGROUP_1: 1 位抢占优先级
//                      3 位子优先级
//NVIC_PRIORITYGROUP_2: 2 位抢占优先级
//                      2 位子优先级
//NVIC_PRIORITYGROUP_3: 3 位抢占优先级
//                      1 位子优先级
//NVIC_PRIORITYGROUP_4: 4 位抢占优先级
//                      0 位子优先级
#define NVIC_PRIORITYGROUP_0   	((uint8_t)0x7)                                                               
#define NVIC_PRIORITYGROUP_1	((uint8_t)0x6)
#define NVIC_PRIORITYGROUP_2   	((uint8_t)0x5)
#define NVIC_PRIORITYGROUP_3   	((uint8_t)0x4) 
#define NVIC_PRIORITYGROUP_4   	((uint8_t)0x3) 

extern volatile bool CSI_intflag;

void FingerTech_NVIC_SetGrouping(uint32_t group);
void FingerTech_NVIC_SetPriority(IRQn_Type IRQn,uint32_t prep,uint32_t subp);
#endif
