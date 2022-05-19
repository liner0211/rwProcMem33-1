#ifndef VERSION_CONTROL_H_
#define VERSION_CONTROL_H_
#include <linux/version.h>
#define DEV_FILENAME "rwProcMem37" //��ǰ����DEV�ļ���

//�����ں�ģ�����ģʽ
#define CONFIG_MODULE_GUIDE_ENTRY

//�Ƿ����ö�ȡpagemap�ļ������������ڴ�ĵ�ַ
//#define CONFIG_USE_PAGEMAP_FILE

//�Ƿ��ӡ�ں˵�����־
//#define CONFIG_DEBUG_PRINTK

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif
#ifndef LINUX_VERSION_CODE 
//#define LINUX_VERSION_CODE KERNEL_VERSION(3,10,0)
//#define LINUX_VERSION_CODE KERNEL_VERSION(3,10,84)
//#define LINUX_VERSION_CODE KERNEL_VERSION(3,18,71)
//#define LINUX_VERSION_CODE KERNEL_VERSION(3,18,140)
//#define LINUX_VERSION_CODE KERNEL_VERSION(4,4,21)
//#define LINUX_VERSION_CODE KERNEL_VERSION(4,4,78)
//#define LINUX_VERSION_CODE KERNEL_VERSION(4,4,153)
//#define LINUX_VERSION_CODE KERNEL_VERSION(4,4,192)
//#define LINUX_VERSION_CODE KERNEL_VERSION(4,9,112)
//#define LINUX_VERSION_CODE KERNEL_VERSION(4,9,186)
//#define LINUX_VERSION_CODE KERNEL_VERSION(4,14,83)
//#define LINUX_VERSION_CODE KERNEL_VERSION(4,14,117)
//#define LINUX_VERSION_CODE KERNEL_VERSION(4,14,141)
//#define LINUX_VERSION_CODE KERNEL_VERSION(4,19,81)
//#define LINUX_VERSION_CODE KERNEL_VERSION(5,4,61)
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,83)
#include <linux/sched/task.h>
#include <linux/sched/mm.h>
#include <linux/sched/signal.h>
#endif



#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,4,192)
#define FILE_OP_DIR_ITER iterate
#endif

#ifndef FILE_OP_DIR_ITER
#define FILE_OP_DIR_ITER iterate_shared
#endif



#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,84)
#define KUID_T_VALUE
#define KGID_T_VALUE
#else
#define KUID_T_VALUE .val
#define KGID_T_VALUE .val
#endif




#ifdef CONFIG_DEBUG_PRINTK
#define printk_debug printk
#else
static inline void printk_debug(char *fmt, ...) {}
#endif

#endif /* VERSION_CONTROL_H_ */