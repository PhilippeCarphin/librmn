//! \file rpn_macros_arch.h Architecture and compiler specific definitions.  This file should never be included directly; use rpnmacros.h


//! Name mangling for Fortran functions without underscores in their name
#define f77name(a) a##_
//! Name mangling for Fortran functions with underscores in their name
#define f77_name(a) a##_

#ifndef Big_Endian
#define Big_Endian
#endif

//! \todo Replace with a definition from stdint.h like int32_t according to the actual size of an int on that platform!
#define F2Cl const int
