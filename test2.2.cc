#include <iostream>
#include "vm_app.h"

using namespace std;

int main(){
    int i;
    char *a;
    a = (char *) vm_extend();
    a[0] = 'h';
    a[1] = 'e';
    a[2] = 'l';
    a[3] = 'l';
    a[4] = 'o';
    char *b;
    b = (char *) vm_extend();
    char *c;
    c = (char *) vm_extend();
    c[0] = 'l';
    char *d;
    d = (char *) vm_extend();
    d[0] = 'l';
    char *e;
    e = (char *) vm_extend();
    e[0] = 'o';
    a[0] = a[0];
      for(i = 0; i < 5; i++){
    	b[i] = a[i];
    }
    char *f;
    f = (char *) vm_extend();
    f[0] = 'a';
    vm_syslog(a, 5);
    vm_syslog(b, 5);
    vm_syslog(c, 1);
    vm_syslog(d, 1);
    vm_syslog(e, 1);
    vm_syslog(f, 1);
    a[0] = 'x';
    b[0] = 'o';
    c[0] = 'l';
    d[0] = 'l';
    e[0] = 'e';
    f[0] = 'h';
    char *g;
    g = (char *) vm_extend();
    vm_syslog(g, 1);
    g[0] = 'H';
    char *h;
    h = (char *) vm_extend();
    vm_syslog(h, 1);
    h[0] = 'E';
    char *l;
    l = (char *) vm_extend();
    vm_syslog(l, 1);
    l[0] = 'O';
}
