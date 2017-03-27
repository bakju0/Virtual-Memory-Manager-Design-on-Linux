#include <iostream>
#include "vm_app.h"

using namespace std;

int main()
{
    int i,j;
    char *a = (char *) vm_extend();
    char *b = (char *) vm_extend();
    char *c = (char *) vm_extend();
    char *d = (char *) vm_extend();
    char *e = (char *) vm_extend();
    a[0] = 'h';
    a[1] = 'e';
    a[2] = 'l';
    a[3] = 'l';
    a[4] = 'o';
    for(i = 0; i < 5; i++){
    	b[i] = a[i];
    	c[i] = b[i];
    }
    vm_yield();
    for(j = 0; i < 5; i++){
    	d[i] = c[i];
    	e[i] = d[i];
    }
    vm_syslog(a, 5);
    vm_syslog(b, 5);
    vm_syslog(c, 5);
    vm_syslog(d, 5);
    vm_syslog(e, 5);
    vm_syslog(a, 9000);
    vm_syslog(b-9999, 9999);
    vm_syslog(c-8192, 9000);
    vm_syslog(d-8190, 10);
    vm_syslog(e-10, 10);
    vm_syslog(a-1, 1);
}

