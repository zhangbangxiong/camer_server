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
    
    od = (unsigned long) (o->user + o->nice + o->system +o->idle);//��һ��(�û�+���ȼ�+ϵͳ+����)��ʱ���ٸ���od
    nd = (unsigned long) (n->user + n->nice + n->system +n->idle);//�ڶ���(�û�+���ȼ�+ϵͳ+����)��ʱ���ٸ���od
      
    id = (unsigned long) (n->user - o->user);    //�û���һ�κ͵ڶ��ε�ʱ��֮���ٸ���id
    sd = (unsigned long) (n->system - o->system);//ϵͳ��һ�κ͵ڶ��ε�ʱ��֮���ٸ���sd
    if((nd-od) != 0)
    cpu_use = (int)((sd+id)*100)/(nd-od); //((�û�+ϵͳ)��100)��(��һ�κ͵ڶ��ε�ʱ���)�ٸ���g_cpu_used
    //cpu_use = (int)((sd+id)*10000)/(nd-od); //((�û�+ϵͳ)��100)��(��һ�κ͵ڶ��ε�ʱ���)�ٸ���g_cpu_used
    else cpu_use = 0;
    //printf("cpu: %u\n",cpu_use);
    return cpu_use;
}

int get_cpuoccupy (CPU_OCCUPY *cpust) //��������get��������һ���βνṹ����Ū��ָ��O
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
    
    //��ȡ�ڴ�
    printf("mem use:%d\n", get_memoccupy ());
    
    //��һ�λ�ȡcpuʹ�����
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);
    usleep(100000);
    
    //�ڶ��λ�ȡcpuʹ�����
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);
    
    //����cpuʹ����
    cpu = cal_cpuoccupy ((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);
	printf("cpu use:%d\n", cpu);
    
    return 0;
}
#endif
