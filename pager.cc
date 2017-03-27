
#include "vm_pager.h"
#include <fstream>
#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <cstddef>

using namespace std;

struct Page
{
	page_table_entry_t* pte; 
	int valid;
	int resident;
	int dirty;
	int reference; 
	int disk_block_num;
};

struct Process
{
	page_table_t* pt;  
	Page** pages;	   
	int next_available_vp;	
};

queue<unsigned int> freePageQueue;
queue<unsigned int> freeDiskQueue;

Process *processNow;
pid_t pidNow;
queue<Page*> clockQueue;
map<pid_t,Process*> processMap;
int disk_blocks_left;

extern void vm_init(unsigned int memory_pages, unsigned int disk_blocks)
{
	disk_blocks_left=0;

	for(int i=(memory_pages-1);i>=0;i--)
		freePageQueue.push(i);	

	for(int j=0;j<disk_blocks;j++)
	{
		freeDiskQueue.push(j);
		disk_blocks_left++;
	}
	
	page_table_base_register=NULL;
	processNow=NULL;
	
	
}

extern void vm_create(pid_t pid)
{
	Process *p=new Process;

	p->pt=new page_table_t;
	p->pages=new Page*[VM_ARENA_SIZE/VM_PAGESIZE];

	p->next_available_vp=0;
	
	processMap[pid]=p;	
}

extern void vm_switch(pid_t pid)
{
	if(processMap.find(pid)!=processMap.end())
	{
		pidNow=pid;
		processNow=processMap[pid];
		page_table_base_register=processNow->pt;
	}
}

extern void* vm_extend()
{
	if((processNow->next_available_vp)>=(VM_ARENA_SIZE/VM_PAGESIZE))
		return NULL;
	if(disk_blocks_left<1||processNow==NULL)
		return NULL;

	
	disk_blocks_left--;	

	Page* p=new Page;
	p->pte=&(page_table_base_register->ptes[processNow->next_available_vp]);
	
	
	
	p->pte->read_enable=0;
	p->pte->write_enable=0;

	p->valid=1;
	p->reference=0;  
	p->dirty=0;	
	p->resident=0;
	p->disk_block_num=-1;
		


	processNow->pages[processNow->next_available_vp]=p;
	processNow->next_available_vp++;

	
	return (void*)((unsigned long)VM_ARENA_BASEADDR+(processNow->next_available_vp-1)*VM_PAGESIZE);
}


void evict_createFreePage()
{	
	Page* p=clockQueue.front();
	
	while(p->reference)
	{
		p->reference=0;
		p->pte->read_enable=0;
		p->pte->write_enable=0;
		
		clockQueue.pop();
		clockQueue.push(p);
		p=clockQueue.front();		
	}
	
	//cout<<"evicted p->pte: "<<&(p->pte)<<endl;

	p->resident=0;	
	
	if(p->dirty)
	{
		if(p->disk_block_num==-1)
		{
			p->disk_block_num=freeDiskQueue.front();
			freeDiskQueue.pop();
		}
		disk_write(p->disk_block_num,p->pte->ppage);		
	}
		

	p->pte->read_enable=0;
	p->pte->write_enable=0;
	  
	p->dirty=0;  
	
	freePageQueue.push(p->pte->ppage);
	clockQueue.pop();
}


extern int vm_fault(void *addr, bool write_flag)
{
	intptr_t address=(intptr_t)addr-(intptr_t)VM_ARENA_BASEADDR;
	if(address<0)	return -1;
	
	if(processNow->pages[address/VM_PAGESIZE]->valid==0) 
	return -1;
	
	if(address>=((processNow->next_available_vp)*VM_PAGESIZE))
	return -1;

	Page* p=processNow->pages[address/VM_PAGESIZE];
	p->reference=1;

	if(!p->resident)
	{
		if(freePageQueue.empty())
			evict_createFreePage();
		
		p->pte->ppage=freePageQueue.front();
		
		freePageQueue.pop();
		clockQueue.push(p);
		
		p->resident=1;	
		
		
		if(p->disk_block_num==-1)
		{
			for(int i=0;i<VM_PAGESIZE;i++)
			{
				((char*)pm_physmem)[(p->pte->ppage*VM_PAGESIZE)+i]=0;
				
			}
		}
		
		else
			disk_read(p->disk_block_num,p->pte->ppage);
	}

	//cout<<"following is the  page state"<<endl;
	//cout<<"p->pte: "<<&(p->pte)<<endl;
	//cout<<"p->resident: "<<p->resident<<endl;
	//cout<<"p->valid: "<<p->valid<<endl;
	//cout<<"p->dirty: "<<p->dirty<<endl;
	//cout<<"p->reference: "<<p->reference<<endl;
	//cout<<"p->disk_block_num: "<<p->disk_block_num<<endl;
	

	if(write_flag)
	{
		p->pte->write_enable=1;
		p->pte->read_enable=1;
		p->dirty=1;
	}
	else
	{
		p->pte->read_enable=1;
		if(p->dirty==1)
			p->pte->write_enable=1;
		else
			p->pte->write_enable=0;
	}
	return 0;	
}


extern int vm_syslog(void *message,unsigned int len)
{
	if(len<=0)	return -1;
	intptr_t address=(intptr_t)message-(intptr_t)VM_ARENA_BASEADDR;
	if(address<0)	return -1;
	if(address >= (processNow->next_available_vp)*VM_PAGESIZE)   return -1;
	
	string s="";
	for(int i=0;i<len;i++)
	{
		int pageNum=(address+i)/VM_PAGESIZE;
		int pageOffset=(address+i)%VM_PAGESIZE;
		int phyPage=page_table_base_register->ptes[pageNum].ppage;
		if(page_table_base_register->ptes[pageNum].read_enable==0||!processNow->pages[pageNum]->resident)
		{
			if(vm_fault((void*)((unsigned long)message+i),false))
			{
				return -1;
			}
			phyPage=page_table_base_register->ptes[pageNum].ppage;
		}
		processNow->pages[pageNum]->reference=1;
		s.append((char*)pm_physmem+phyPage*VM_PAGESIZE+pageOffset,1);  //!!!
	}
	cout<<"syslog \t\t\t" << s <<endl;
	return 0;
}




extern void vm_destroy()
{
	processMap.erase(pidNow);
	for(int i=0;i<processNow->next_available_vp;i++)
	{
		Page* p=processNow->pages[i];
		if(p->resident)
		{
			p->resident=0;
			
			freePageQueue.push(p->pte->ppage);
			
			Page* clockPage=clockQueue.front();
			int u=clockQueue.size();
			while(clockPage!=p)
			{
				clockQueue.pop();
				clockQueue.push(clockPage);
				clockPage=clockQueue.front();
			}
			clockQueue.pop();			
		}
		
		freeDiskQueue.push(p->disk_block_num);

		disk_blocks_left++;//  !!!!
		//processNow->next_available_vp=-1;// !!!!
		
		p->valid=0;
		p->pte->read_enable=0;
		p->pte->write_enable=0;
		delete p;
	}

	delete processNow->pt;
	delete[] processNow->pages;
	delete processNow;
}
