/***************************************************************
***************************************************************/
#include <stdio.h>
#include <string.h>
#include "cpumem.h"

int get_memoccupy () 
{
    FILE *fd;          
    int n; 
	MEM_OCCUPY	m_total = {0};
	MEM_OCCUPY	m_cache = {0};
	MEM_OCCUPY	m_free = {0};
	
    char buff[256];   
                                                                                                             
    fd = fopen ("/proc/meminfo", "r"); 
	if(NULL == fd)
		return -1;
	 MemTotal:         134408 kB
	MemFree:            6440 kB
	Buffers:            3336 kB
	Cached:            44188 kB
	*/
    fgets (buff, sizeof(buff), fd); 	//line  	
	sscanf (buff, "%s %u %s", m_total.name, &m_total.count, m_total.unit); 

	fgets (buff, sizeof(buff), fd); 
	sscanf (buff, "%s %u %s", m_free.name, &m_free.count, m_free.unit); 
    fgets (buff, sizeof(buff), fd); 
    fgets (buff, sizeof(buff), fd); 
    sscanf (buff, "%s %u %s", m_cache.name, &m_cache.count, m_cache.unit); 
	
    fclose(fd);    
	
	return 100- (100*(m_cache.count + m_free.count))/m_total.count;
}

int cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n) 
{   
    unsigned long od, nd;    
    unsigned long id, sd;
    int cpu_use = 0;   
    
    od = (unsigned long) (o->user + o->nice + o->system +o->idle);//第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (unsigned long) (n->user + n->nice + n->system +n->idle);//第二次(用户+优先级+系统+空闲)的时间再赋给od
      
    id = (unsigned long) (n->user - o->user);    //用户第一次和第二次的时间之差再赋给id
    sd = (unsigned long) (n->system - o->system);//系统第一次和第二次的时间之差再赋给sd
    if((nd-od) != 0)
    cpu_use = (int)((sd+id)*100)/(nd-od); //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
    //cpu_use = (int)((sd+id)*10000)/(nd-od); //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
    else cpu_use = 0;
    //printf("cpu: %u\n",cpu_use);
    return cpu_use;
}

int get_cpuoccupy (CPU_OCCUPY *cpust) //对无类型get函数含有一个形参结构体类弄的指针O
{   
    FILE *fd;         
    int n;            
    char buff[256]; 
    CPU_OCCUPY *cpu_occupy;
    cpu_occupy=cpust;
                                                                                                              
    fd = fopen ("/proc/stat", "r"); 
	if(NULL == fd)	
	{
		return -1;
	}
	
    fgets (buff, sizeof(buff), fd);
    sscanf (buff, "%s %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle);
    fclose(fd);  

	return 0;
}

#if 0
int main()
{
    CPU_OCCUPY cpu_stat1;
    CPU_OCCUPY cpu_stat2;
    MEM_OCCUPY mem_stat;
    int cpu;
    
    //获取内存
    printf("mem use:%d\n", get_memoccupy ());
    
    //第一次获取cpu使用情况
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);
    usleep(100000);
    
    //第二次获取cpu使用情况
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);
    
    //计算cpu使用率
    cpu = cal_cpuoccupy ((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);
	printf("cpu use:%d\n", cpu);
    
    return 0;
}
#endif
