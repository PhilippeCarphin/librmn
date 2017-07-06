subroutine handler(signum)
integer :: signum
   print *, "Catched signal ", signum
   call sleep(10)
   print *, "Die gracefully"
   stop
end subroutine handler

program test_capture_signauxf
include 'capture_signaux.inc'
external handler
   integer, dimension(3) ::  signaux = (/10,12,15/)
   integer :: i 

   call capture_signaux(signaux(:1),handler)
   !call capture_signaux((/10/),handler)
   call capture_signaux(signaux(2:3),handler)
   !call capture_signaux((/12,15/),handler)

   i=1
   do
      print *, "Boucle infinie ",i
      call sleep(1)
      i=i+1
   enddo
end program test_capture_signauxf
