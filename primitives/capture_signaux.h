#include <signal.h>
void capture_signaux(int *signum,int nsig,void (*handler)(int));
