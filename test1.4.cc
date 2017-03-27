#include<iostream>
#include"vm_app.h"

using namespace std;
int main()
{
char*p;
int i=0;
while(vm_extend()!=NULL)
i++;
p = (char*)vm_extend();
p[i]= 'h';
}
