#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
/***************************************************************************
** Fonction capture_signaux - Installer un gestionnaire de signaux
*/
void capture_signaux(int *signum,int nsig,void (*handler)(int))
/*
*Auteur : Michel Beland (RPN-SI) juin 2017
*
*Objet( capture_signaux )
*     Installer un gestionnaire de signaux pour indiquer au programme de
*     l'appeler a la reception des signaux passes en argument.
*
*Arguments
*  IN     signum   Pointeur a un tableau entier contenant le numero des signaux
*                  a intercepter
*  IN     nsig     Nombre de signaux a intercepter
*  IN     handler  Pointeur vers la fonction a executer lors de la reception des
*                  signaux.
*
*Usage typique :
#include "capture_signaux.h"
int signum[] = {SIGTERM,SIGUSR1,SIGUSR2};
capture_signaux(signum,3,handler);
*/
{
   struct sigaction act;
   int isig;
   
   /* We use sigaction() because signal90 (below) does not block several
    * SIGTERM signals in a row. Also, sigaction conforms to the POSIX.1
    * standard. */
   /* (void) signal(SIGTERM,handler); */
   act.sa_handler=handler;
   act.sa_flags= 0;
   /* Mask all signals while handler is running. */
   (void) sigfillset(&act.sa_mask);

   for (isig=0; isig < nsig ; isig++) {
      if (0 != sigaction(signum[isig],&act,NULL)) {
         perror("sigaction returned with error\n");
      }
   }

}

