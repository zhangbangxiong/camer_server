#ifndef CPUMEM_H
#define CPUMEM_H

typedef struct PACKED_CPU         	//定义一个cpu occupy的结构体
{
	char name[20];      		//定义一个char类型的数组名name有20个元素
	unsigned int user; 			//定义一个无符号的int类型的user
	unsigned int nice; 			//定义一个无符号的int类型的nice
	unsigned int system;		//定义一个无符号的int类型的system
	unsigned int idle; 			//定义一个无符号的int类型的idle
}CPU_OCCUPY;

typedef struct PACKED_MEM         	//定义一个mem occupy的结构体
{
	char name[20];      			//定义一个char类型的数组名name有20个元素
	unsigned long count; 
	char unit[20];     				//单位               
}MEM_OCCUPY;

int get_memoccupy();
int get_cpuoccupy (CPU_OCCUPY *cpust);
int cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n);

#endif
