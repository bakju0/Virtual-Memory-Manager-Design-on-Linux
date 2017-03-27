#include<iostream>
#include"vm_app.h"

using namespace std;
int main()
{
char *p;
p = (char*) vm_extend();
p[0]='1';
vm_syslog(p,2000);
p = (char*) vm_extend();
p[0]='2';
p[10000]=3;
vm_syslog(p,10000);
}
