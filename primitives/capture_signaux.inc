      interface
         subroutine capture_signaux(signum,handler)
         integer, dimension(:), intent(IN) :: signum
         interface
            subroutine handler(signum)
            integer :: signum
            end subroutine handler
         end interface
         end subroutine capture_signaux
      end interface
