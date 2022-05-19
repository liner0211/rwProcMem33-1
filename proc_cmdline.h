#ifndef PROC_CMDLINE_H_
#define PROC_CMDLINE_H_
//����
//////////////////////////////////////////////////////////////////////////
#include <linux/pid.h>
#include <linux/ksm.h>
#include "ver_control.h"
static inline struct pid * get_proc_pid_struct(int pid);
static inline int get_proc_pid(struct pid* proc_pid_struct);
static inline void release_proc_pid_struct(struct pid* proc_pid_struct);
static inline int get_proc_cmdline_addr(struct pid* proc_pid_struct, size_t * arg_start, size_t * arg_end);
static inline int get_task_proc_cmdline_addr(struct task_struct *task, size_t * arg_start, size_t * arg_end);


//ʵ��
//////////////////////////////////////////////////////////////////////////
#include "phy_mem.h"
static ssize_t g_arg_start_offset_proc_cmdline = 0; //mm_struct��arg_start��ƫ��λ��
static bool g_init_arg_start_offset_success = false; //�Ƿ��ʼ���ҵ���arg_start��ƫ��λ��


static inline struct pid * get_proc_pid_struct(int pid) {
	return find_get_pid(pid);
}


static inline int get_proc_pid(struct pid* proc_pid_struct) {
	return proc_pid_struct->numbers[0].nr;
}
static inline void release_proc_pid_struct(struct pid* proc_pid_struct) {
	put_pid(proc_pid_struct);
}
static inline int init_proc_cmdline_offset(void) {
	int is_find_cmdline_offset = 0;
	size_t size = 4096;
	char *lpOldCmdLineBuf = NULL;
	char *lpNewCmdLineBuf = NULL;

	struct file *fp_cmdline = filp_open("/proc/self/cmdline", O_RDONLY, 0);
	mm_segment_t pold_fs;
	if (IS_ERR(fp_cmdline)) {
		return -ESRCH;
	}
	pold_fs = get_fs();
	set_fs(KERNEL_DS);


	lpOldCmdLineBuf = (char*)kmalloc(size, GFP_KERNEL);
	memset(lpOldCmdLineBuf, 0, size);
	if (fp_cmdline->f_op->read(fp_cmdline, lpOldCmdLineBuf, size, &fp_cmdline->f_pos) <= 0) {
		printk_debug(KERN_INFO "Failed to do read!");
		set_fs(pold_fs);
		filp_close(fp_cmdline, NULL);
		kfree(lpOldCmdLineBuf);
		return -ESPIPE;
	}
	set_fs(pold_fs);
	filp_close(fp_cmdline, NULL);

	printk_debug(KERN_INFO "lpOldCmdLineBuf:%s\n", lpOldCmdLineBuf);


	lpNewCmdLineBuf = (char*)kmalloc(size, GFP_KERNEL);

	g_init_arg_start_offset_success = true;
	for (g_arg_start_offset_proc_cmdline = -64; g_arg_start_offset_proc_cmdline <= 64; g_arg_start_offset_proc_cmdline += 1) {
		size_t arg_start = 0, arg_end = 0;
		if (get_task_proc_cmdline_addr(current, &arg_start, &arg_end) == 0) {
			printk_debug(KERN_INFO "get_task_proc_cmdline_addr arg_start %p\n", (void*)arg_start);

			//��ȡÿ��+4��arg_start�ڴ��ַ������
			if (arg_start > 0) {

				size_t read_size = 0;

				//��ʼ��ȡ�����ڴ�
				memset(lpNewCmdLineBuf, 0, size);

				while (read_size < size) {
					//��ȡ���������ڴ��ַ��Ӧ�������ַ
					size_t phy_addr;
					size_t pfn_sz;
					size_t ret;
					char *lpOutBuf;

#ifdef CONFIG_USE_PAGEMAP_FILE
					struct file * pFile = open_pagemap(-1);
					printk_debug(KERN_INFO "open_pagemap %d\n", pFile);
					if (!pFile) { break; }

					phy_addr = get_pagemap_phy_addr(pFile, arg_start);

					close_pagemap(pFile);
#else
					pte_t *pte;
					get_task_proc_phy_addr(&phy_addr, current, arg_start + read_size, &pte);
#endif
					printk_debug(KERN_INFO "phy_addr:0x%zx\n", phy_addr);
					if (phy_addr == 0) {
						break;
					}


					pfn_sz = size_inside_page(phy_addr, ((size - read_size) > PAGE_SIZE) ? PAGE_SIZE : (size - read_size));
					printk_debug(KERN_INFO "pfn_sz:%zu\n", pfn_sz);


					ret = 0;
					lpOutBuf = (char*)(lpNewCmdLineBuf + read_size);
					read_ram_physical_addr(&ret, phy_addr, lpOutBuf, true, pfn_sz);
					read_size += pfn_sz;
				}

				printk_debug(KERN_INFO "lpNewCmdLineBuf:%s, len:%ld\n", lpNewCmdLineBuf, strlen(lpNewCmdLineBuf));

				if (strcmp(lpNewCmdLineBuf, lpOldCmdLineBuf) == 0) {
					is_find_cmdline_offset = 1;
					break;
				}


			}
		}
	}


	kfree(lpOldCmdLineBuf);
	kfree(lpNewCmdLineBuf);

	if (!is_find_cmdline_offset) {
		g_init_arg_start_offset_success = false;
		printk_debug(KERN_INFO "find cmdline offset failed\n");
		return -ESPIPE;
	}
	printk_debug(KERN_INFO "g_arg_start_offset_proc_cmdline:%zu\n", g_arg_start_offset_proc_cmdline);
	return 0;
}
static inline int get_proc_cmdline_addr(struct pid* proc_pid_struct, size_t * arg_start, size_t * arg_end) {
	int ret = 0;
	struct task_struct *task = NULL;


	if (g_init_arg_start_offset_success == false) {
		if ((ret = init_proc_cmdline_offset()) != 0) {
			return ret;
		}
	}


	task = pid_task(proc_pid_struct, PIDTYPE_PID);
	if (!task) { return -EFAULT; }
	ret = get_task_proc_cmdline_addr(task, arg_start, arg_end);
	return ret;
}
static inline int get_task_proc_cmdline_addr(struct task_struct *task, size_t * arg_start, size_t * arg_end) {
	if (g_init_arg_start_offset_success) {
		struct mm_struct *mm;
		ssize_t accurate_offset;
		mm = get_task_mm(task);

		if (!mm) { return -EFAULT; }

		//��ȷƫ��
		accurate_offset = (ssize_t)((size_t)&mm->arg_start - (size_t)mm + g_arg_start_offset_proc_cmdline);
		if (accurate_offset >= sizeof(struct mm_struct) - sizeof(ssize_t)) {
			mmput(mm);
			return -EFAULT;
		}

		down_read(&mm->mmap_sem);
		printk_debug(KERN_INFO "accurate_offset:%zd\n", accurate_offset);

		*arg_start = *(size_t*)((size_t)mm + (size_t)accurate_offset);
		*arg_end = *(size_t*)((size_t)mm + (size_t)accurate_offset + sizeof(unsigned long));

		printk_debug(KERN_INFO "arg_start addr:0x%p\n", (void*)*arg_start);

		up_read(&mm->mmap_sem);
		mmput(mm);
		return 0;
	}
	return -ESPIPE;
}

#endif /* PROC_CMDLINE_H_ */