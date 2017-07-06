#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "capture_signaux.h"

void handler(int signum)
{
   printf("Catched signal %d\n", signum);
   sleep(10);
   printf("Die gracefully\n");
   exit(0);
}

int main(void)
{
   static int signaux[2] = {SIGTERM,SIGUSR2};
   int i; 

   capture_signaux(signaux,2,handler);
   /* Si on appelle capture_signaux sur chaque signal, cela fonctionne aussi :
   capture_signaux(&signaux[0],1,handler);
   capture_signaux(&signaux[1],1,handler);
   */

   for (i=1;;i++) {
      printf("Boucle infinie %d\n",i);
      sleep(1);
   }
}

