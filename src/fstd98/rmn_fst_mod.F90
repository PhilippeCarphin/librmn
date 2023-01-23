
module rmn_fst
    use iso_c_binding
    use f_c_strings_mod
    implicit none
    private

    public :: fstouv, fstecr

    interface
        function c_fstouv(iun, options) result(status) BIND(C, name='c_fstouv')
            import C_INT32_T, C_CHAR
            implicit none
            integer(C_INT32_T), intent(in), value :: iun
            character(C_CHAR), dimension(*), intent(in) :: options
            integer(C_INT32_T) :: status
        end function c_fstouv

        function c_fstecr(field, work, npak, iun, date, deet, npas, ni, nj, nk, &
                        ip1, ip2, ip3, typvar, nomvar, etiket, grtyp, &
                        ig1, ig2, ig3, ig4, datyp, rewrite) &
                        result(status) bind(C,name='c_fstecr')
            import :: C_INT, C_CHAR
            implicit none
#define IgnoreTypeKindRank field, work
#define ExtraAttributes 
#include "rmn/IgnoreTypeKindRank.hf"
            integer(C_INT), intent(IN), value :: iun, npak, date, deet, npas, ni, nj, nk, datyp, rewrite
            integer(C_INT), intent(IN), value :: ip1, ip2, ip3, ig1, ig2, ig3, ig4
            character(C_CHAR), dimension(*), intent(IN) :: typvar, nomvar, etiket, grtyp
            integer(C_INT) :: status
            end function c_fstecr
    end interface

contains

    function fstouv(iun, options) result(status)
        implicit none
        integer(C_INT32_T), intent(in), value :: iun
        character(len=*),   intent(in)        :: options
        integer(C_INT32_T) :: status

        print*, 'Calling fstouv from module! Options = ', options, ', iun = ', iun

        status = c_fstouv(iun, f_c_string(options))

        print*, 'iun = ', iun
    end function fstouv

    subroutine fstecr(field, work, npak, iun, date, deet, npas, ni, nj, nk, &
                        ip1, ip2, ip3, typvar, nomvar, etiket, &
                        grtyp, ig1, ig2, ig3, ig4, datyp, rewrite)
        implicit none
#define IgnoreTypeKindRank field, work
#define ExtraAttributes 
#include "rmn/IgnoreTypeKindRank.hf"
        integer(C_INT), intent(IN), value :: iun
        integer(C_INT), intent(IN) :: npak, date, deet, npas, ni, nj, nk, datyp
        integer(C_INT), intent(IN) :: ip1, ip2, ip3, ig1, ig2, ig3, ig4
        character(len=*), intent(IN) :: typvar, nomvar, etiket, grtyp
        logical, intent(IN) :: rewrite
        integer(C_INT) :: status
        character(len=4)  :: nom
        character(len=2)  :: typ
        character(len=1)  :: gty
        character(len=12) :: eti

        integer(C_INT32_T) :: c_rewrite

        nom = nomvar
        typ = typvar
        gty = grtyp
        eti = etiket

        c_rewrite = 0
        if (rewrite) c_rewrite = 1

        print *, 'Calling fstecr from module!'
        status = c_fstecr(field, work, npak, iun, date, deet, npas, ni, nj, nk,                                     &
                        ip1, ip2, ip3, f_c_string(typ), f_c_string(nom), f_c_string(eti), f_c_string(gty),          &
                        ig1, ig2, ig3, ig4, datyp, c_rewrite)
    end subroutine fstecr
end module rmn_fst
