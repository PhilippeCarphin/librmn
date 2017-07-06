!***********************************************************************
!** Module mod_signaux - Définir l'interface ISO_C_BINDING vers la fonction
!                        capture_signaux() du fichier capture_signaux.c.
!
!Auteur : Michel Béland (RPN-SI) juin 2017
!
!Procedures accessibles publiquement : 
!         savehandler
!         capture_signaux_c
!         handlerc
!
! mais on ne devrait appeler que le sous-programme capture_signaux defini apres
! le module ci-dessous.

module mod_signaux
use, intrinsic :: ISO_C_BINDING
implicit none

private

public :: savehandler,capture_signaux_c,handlerc

! On definit un pointeur vers un gestionnaire de signal qui est un
! sous-programme Fortran traditionnel.
procedure(handler_interface), save, pointer :: handlerp => null()
abstract interface
  subroutine handler_interface(signum)
  integer, intent(IN) :: signum
  end subroutine handler_interface
end interface

! On definit ici comment est declaree la fonction C capture_signaux pour qu'on
! puisse l'appeler en Fortran (dans ce module)
interface
  subroutine capture_signaux_c(signum,nsig,handlerc) BIND(C,name='capture_signaux')
    use, intrinsic :: ISO_C_BINDING
    implicit none
    integer(C_INT), dimension(*) :: signum
    integer(C_INT), VALUE :: nsig
    interface
       subroutine handlerc(signum) BIND(C)
       use, intrinsic :: ISO_C_BINDING
       integer(C_INT), VALUE :: signum
       end subroutine handlerc
    end interface
  end subroutine capture_signaux_c
end interface

contains
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
subroutine savehandler(handler)
interface
   subroutine handler(signum)
   integer :: signum
   end subroutine handler
end interface
handlerp => handler
end subroutine savehandler
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! Le gestionnaire de signal que l'on passe au C est toujours le meme, mais il
! appelle handlerp qu'on initialise dans savehandler.
subroutine handlerc(signumc) BIND(C)
integer(C_INT), VALUE :: signumc
integer :: signum
signum=signumc
call handlerp(signum)
end subroutine handlerc
   
end module mod_signaux

!***********************************************************************
!**S/P capture_signaux  - Installer un gestionnaire de signaux
!
subroutine capture_signaux(signum,handler)
   use mod_signaux
   integer, dimension(:), intent(IN) :: signum
   interface
      subroutine handler(signum)
      integer :: signum
      end subroutine handler
   end interface
!
!Auteur : Michel Beland (RPN-SI) juin 2017
!
!Objet( capture_signaux )
!     Installer un gestionnaire de signaux pour indiquer au programme de
!     l'appeler a la reception des signaux passes en argument.
!
!Arguments
!  IN     signum    Tableau entier contenant le numero des signaux a intercepter
!  IN     handler   sous-programme que l'on doit appeler lorqu'on recoit les
!                   signaux
!
!Usage typique :
!         include 'capture_signaux.inc'
!         integer, dimension(3) :: signum = (/10,12,15/)
!         call capture_signaux(signum,handler)
!

! On sauvegarde un pointeur vers le sous-programme handler
   call savehandler(handler)
! On installe le gestionnaire de signaux handlerc, qui appelle lui-meme le
! sous-programme handler. On utilise l'interface ISO_C_BINDING de la fonction C
! capture_signaux(), définie dans capture_signaux.c.
   call capture_signaux_c(signum, size(signum),handlerc)

end subroutine capture_signaux
