#ifndef CPUMEM_H
#define CPUMEM_H

typedef struct PACKED_CPU         	//����һ��cpu occupy�Ľṹ��
{
	char name[20];      		//����һ��char���͵�������name��20��Ԫ��
	unsigned int user; 			//����һ���޷��ŵ�int���͵�user
	unsigned int nice; 			//����һ���޷��ŵ�int���͵�nice
	unsigned int system;		//����һ���޷��ŵ�int���͵�system
	unsigned int idle; 			//����һ���޷��ŵ�int���͵�idle
}CPU_OCCUPY;

typedef struct PACKED_MEM         	//����һ��mem occupy�Ľṹ��
{
	char name[20];      			//����һ��char���͵�������name��20��Ԫ��
	unsigned long count; 
	char unit[20];     				//��λ               
}MEM_OCCUPY;

int get_memoccupy();
int get_cpuoccupy (CPU_OCCUPY *cpust);
int cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n);

#endif
