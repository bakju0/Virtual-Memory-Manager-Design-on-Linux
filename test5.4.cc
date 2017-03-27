#include<iostream>
#include"vm_app.h"

using namespace std;
int main()
{
char *p;
p = (char*) vm_extend();
int i,j;
i= vm_syslog(p,1);
j= vm_syslog(p,10000);
if(i==j) return j;
else return i;
}
