/* RMNLIB - Library of useful routines for C and FORTRAN programming
 * Copyright (C) 1975-2001  Division de Recherche en Prevision Numerique
 *                          Environnement Canada
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation,
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <bitPacking.h>


typedef struct {
#if defined(Little_Endian)
    uint32_t numOfBitsPerToken : 6, SHIFT : 6, unused : 12, ID : 8;
#else
    uint32_t ID : 8, unused : 12, SHIFT : 6, numOfBitsPerToken : 6;
#endif
    uint32_t numOfPackedToken : 32;
    uint32_t minValue         : 32;
    uint32_t maxValue         : 32;
} integer_header;



//! Pack or unpack array
int compact_integer(
    //! [in,out] Unpacked array of integer
    void *unpackedArrayOfInt,
    //! [in] Pack header
    void *packedHeader,
    //! [in,out] Packed array of integer
    void *packedArrayOfInt,
    //! [in] Number of elements in the unpacked array
    int nbElems,
    //! [in] Packed integer size in bits.  Set to -1 to autodetect
    int bitSizeOfPackedToken,
    //! [in] When packing, last bit of integer packed inside array.  When unpacking, first bit of integer packed inside array
    int offset,
    //! [in] Unpacked integer spacing
    int stride,
    //! [in] Operation mode
    //! | Value | Operation       |
    //! | ----: | :-------------- |
    //! |     1 | Unsigned pack   |
    //! |     2 | Unsigned unpack |
    //! |     3 | Signed pack     |
    //! |     4 | Signed unpack   |
    int opCode
) {
    if ( bitSizeOfPackedToken == 0 ) return 0;

    uint32_t *arrayOfUnsignedUnpacked = (uint32_t *)unpackedArrayOfInt;
    int32_t * arrayOfSignedUnpacked = (int *)unpackedArrayOfInt;

    int intCount = nbElems;
    int minSignedInteger = 0;
    int maxSignedInteger = 0;
    uint32_t minUnsignedInteger = 0;
    uint32_t maxUnsignedInteger = 0;
    if ( (opCode==1) || (opCode==3) ) {
        // collect info for the packing
        if ( packedHeader != NULL ) {
            // pack header is required, (X - Xmin) is used as packInt
            if ( opCode == 1 ) {
                constructHeader(arrayOfUnsignedUnpacked, minUnsignedInteger, maxUnsignedInteger);
            } else {
                constructHeader(arrayOfSignedUnpacked, minSignedInteger, maxSignedInteger);
            }
        } else {
            // pack header not required, X itself is used as packInt, determines bitSizeOfPackedToken, if not available
            if ( bitSizeOfPackedToken == -1 ) {
                // obtain minimum, maximun, span
                uint32_t maxSpan;
                if ( opCode == 3 ) {
                    // signed integer number
                    findMinMax(arrayOfSignedUnpacked, minSignedInteger, maxSignedInteger);
                    maxSpan = ( abs(minSignedInteger) > maxSignedInteger ) ? abs(minSignedInteger) : maxSignedInteger;
                } else if (  opCode == 1 ) {
                    // unsigned integer number
                    maxSpan = arrayOfUnsignedUnpacked[0];
                    for (int i = stride; i < intCount * stride ; i += stride) {
                        maxSpan |= arrayOfUnsignedUnpacked[i];
                    }
                }

                // derive bitSizeOfPackedToken
                bitSizeOfPackedToken = 0;
                while ( maxSpan != 0 ) {
                    maxSpan = maxSpan >> 1;
                    bitSizeOfPackedToken++;
                }
                if ( opCode == 3 ) {
                    // accomodate the signed bit
                    bitSizeOfPackedToken++;
                }
                int wordSize = 8 * sizeof(uint32_t);
                cleanupMask = ((uint32_t)(~0) >> (wordSize - bitSizeOfPackedToken));
            }
        }
    } else {
        // collect info for the unpacking
        int tokenSize;
        int ShiftIntended;
        int  minSigned;
        uint32_t minUnsigned;
        integer_header * theHeader = (integer_header *)packedHeader;
        if ( packedHeader != NULL ) {
            theHeader = (integer_header *)packedHeader;
            tokenSize = theHeader->numOfBitsPerToken;
            ShiftIntended = theHeader->SHIFT;
            intCount = theHeader->numOfPackedToken;
            minSigned = theHeader->minValue;
            minUnsigned = theHeader->minValue;
        } else {
            tokenSize = bitSizeOfPackedToken;
            ShiftIntended = 0;
            intCount = intCount;
            minSigned = minSignedInteger;
            minUnsigned = minUnsignedInteger;
        }
    }

    // compute signed int adjustment, since 1,2,3,4 without header all needs it
    int positiveMask = ( opCode < 3 ) ? 0 : ( 1 << ( bitSizeOfPackedToken - 1 ));

    if ( opCode == 1 ) {
        Pack(arrayOfUnsignedUnpacked, minUnsignedInteger);
    } else if ( opCode == 3 ) {
        Pack(arrayOfSignedUnpacked, minSignedInteger);
    } else if ( opCode == 2 ) {
        Unpack(arrayOfUnsignedUnpacked, (uint32_t  *)packedArrayOfInt, ShiftIntended, tokenSize, minUnsigned, intCount, offset);
    } else if ( opCode == 4 ) {
        Unpack(arrayOfSignedUnpacked, (uint32_t  *)packedArrayOfInt, ShiftIntended, tokenSize, minSigned, intCount, offset);
    } else {
        printf("\n opCode:%d is not defined \n", opCode);
        return 0;
    }

    // unused, function must return something
    return intCount;
}


/***********************************************************************************************
 *                                                                                             *
 * Author   : M. Lepine, sept 2005                                                             *
 *                                                                                             *
 * Objective: transform array of short integer between pack and unpacked format                *
 *                                                                                             *
 * Arguments:                                                                                  *
 *    IN/OUT  unpackedArrayOfShort   unpacked array of short integer                           *
 *    IN/OUT  packedArrayOfInt       packed integer array                                      *
 *    IN      nbElems           total count of element in unpacked integer array          *
 *    IN      bitSizeOfPackedToken   packed integer size in bit                                *
 *                                   when equal "-1", autodetection of bit length is on        *
 *    IN      offset                 in packing   : the last bit of integer packed inside array*
 *                                   in unpacking : the first bit of integer packed inside array*
 *    IN      stride                 unpacked integer spacing indicator                        *
 *    IN      opCode                 5: unsigned short pack                                    *
 *                                   6: unsigned short unpack                                  *
 *                                   7: signed short pack    (not implemented)                 *
 *                                   8: signed short unpack  (not implemented)                 *
 **********************************************************************************************/
int  compact_short(
    void *unpackedArrayOfShort,
    void *packedHeader,
    void *packedArrayOfInt,
    int nbElems,
    int bitSizeOfPackedToken,
    int offset,
    int stride,
    int opCode
) {
    if ( bitSizeOfPackedToken == 0 ) return 0;

    uint32_t minUnsignedInteger = 0;
    uint32_t maxUnsignedInteger = 0;

    // variables used by the unpacker
    int firstPackBit, bitPackInFirstWord, currentSlot;
    uint32_t currentWord, packInt;
    int significantBit, inSignificantBit;
    uint32_t *arrayPtr;
    int tokenSize, ShiftIntended, nbElemsFromHeader;
    int  minSigned;
    uint32_t minUnsigned;


    int wordSize = 8 * sizeof(uint32_t);
    uint16_t arrayOfUnsignedShort = (uint16_t *)unpackedArrayOfShort;
    integer_header *theHeader = (integer_header *)packedHeader;
    uint32_t * packHeader = (uint32_t *)packedHeader;
    uint32_t * arrayOfPacked = (uint32_t  *)packedArrayOfInt;
    int intCount = nbElems;
    uint32_t cleanupMask = ((uint32_t)(~0) >> (wordSize - bitSizeOfPackedToken));

    if (opCode == 5) {
       // collect info for the packing
        if ( packedHeader != NULL ) {
            // pack header is required, (X - Xmin) is used as packInt
            constructHeader(arrayOfUnsignedShort, minUnsignedInteger, maxUnsignedInteger);
        } else {
            // pack header not required, X itself is used as packInt, determines bitSizeOfPackedToken, if not available
            if ( bitSizeOfPackedToken == -1 ) {
                // obtain minimum, maximun, span
                uint32_t maxSpan;
                if ( opCode == 5 ) {
                    // unsigned integer number
                    maxSpan = arrayOfUnsignedShort[0];

                    for(int i = stride; i < intCount * stride ; i += stride) {
                        maxSpan |= arrayOfUnsignedShort[i];
                    }
                }
                // derive bitSizeOfPackedToken
                bitSizeOfPackedToken = 0;
                while ( maxSpan != 0 ) {
                    maxSpan = maxSpan >> 1;
                    bitSizeOfPackedToken++;
                }
                if ( opCode == 3 ) {
                    // accomodate the sign bit
                    bitSizeOfPackedToken++;
                }
                cleanupMask = ((uint32_t)(~0) >> (wordSize - bitSizeOfPackedToken));
            }
        }
    } else {
        // collect info for the unpacking
        if ( packHeader != NULL ) {
            theHeader = (integer_header *)packedHeader;
            tokenSize = theHeader->numOfBitsPerToken;
            ShiftIntended = theHeader->SHIFT;
            intCount = theHeader->numOfPackedToken;
            minSigned = theHeader->minValue;
            minUnsigned = theHeader->minValue;
        } else {
            tokenSize = bitSizeOfPackedToken;
            ShiftIntended = 0;
            intCount = intCount;
            minSigned = 0;
            minUnsigned = minUnsignedInteger;
        }
    }

    // compute signed int adjustment, since 1, 2, 3, 4 without header all needs it
    int positiveMask = ( opCode < 7 ) ? 0 : ( 1 << ( bitSizeOfPackedToken - 1 ));

    if ( opCode == 5 ) {
        Pack(arrayOfUnsignedShort, 0);
    } else if ( opCode == 6 ) {
        Unpack(arrayOfUnsignedShort, ShiftIntended, tokenSize, 0, intCount);
    } else {
        printf("\n opCode:%d is not defined \n", opCode);
        return 0;
    }

    // unused, function must return something
    return intCount;
}

/***********************************************************************************************
 *                                                                                             *
 * Author   : M. Lepine, sept 2005                                                             *
 *                                                                                             *
 * Objective: transform array of short integer between pack and unpacked format                *
 *                                                                                             *
 * Arguments:                                                                                  *
 *    IN/OUT  unpackedArrayOfBytes   unpacked array of bytes                                   *
 *    IN/OUT  packedArrayOfInt       packed integer array                                      *
 *    IN      nbElems           total count of element in unpacked integer array          *
 *    IN      bitSizeOfPackedToken   packed integer size in bit                                *
 *                                   when equal "-1", autodetection of bit length is on        *
 *    IN      offset                 in packing   : the last bit of integer packed inside array*
 *                                   in unpacking : the first bit of integer packed inside array*
 *    IN      stride                 unpacked integer spacing indicator                        *
 *    IN      opCode                 9: unsigned char pack                                     *
 *                                  10: unsigned char unpack                                   *
 **********************************************************************************************/
int  compact_char( void *unpackedArrayOfBytes, void *packedHeader, void *packedArrayOfInt,
                       int nbElems, int bitSizeOfPackedToken, int offset,
                       int stride, int opCode)
{
    int wordSize;
    uint32_t cleanupMask;
    int minSignedInteger = 0;
    uint32_t minUnsignedInteger=0, maxUnsignedInteger=0;
    uint32_t maxRange;
    uint32_t maxSpan;


    int positiveMask;
    unsigned char *arrayOfUnsignedChar;
    uint32_t *arrayOfPacked;
    int i, k;
    int intCount;
    int bitRequiredForRange, shiftRequired = 0;



    /****************************************
     *                                      *
     *     variables used by the packer     *
     *                                      *
     ***************************************/
    int lastPackBit, spaceInLastWord, lastSlot;
    uint32_t lastWordShifted, tempInt;
    uint32_t *packHeader;

    /***************************************
     *                                     *
     *    variables used by the unpacker   *
     *                                     *
     **************************************/
    int firstPackBit, bitPackInFirstWord, currentSlot;
    uint32_t currentWord, packInt;
    int significantBit, inSignificantBit;
    uint32_t *arrayPtr;
    int tokenSize, ShiftIntended, nbElemsFromHeader;
    int  minSigned;
    uint32_t minUnsigned;
    integer_header *theHeader;


/*  printf("minSignedInteger=%d minUnsignedInteger=%d \n",minSignedInteger,minUnsignedInteger); */

    /********************************
     *                              *
     *   handle abnormal condition  *
     *                              *
     ********************************/
    /* token size is 0 */
    if ( bitSizeOfPackedToken == 0 )
      {
        return 0;
      };






    /********************************************************
     *                                                      *
     *    determine wordsize and others                     *
     *                                                      *
     ********************************************************/
    wordSize                 = 8 * sizeof(uint32_t);
    arrayOfUnsignedChar      = (unsigned char *)unpackedArrayOfBytes;
    theHeader                = (integer_header *)packedHeader;
    packHeader               = (uint32_t *)packedHeader;
    arrayOfPacked            = (uint32_t  *)packedArrayOfInt;
    intCount                 = nbElems;
    cleanupMask              = ((uint32_t)(~0)>>(wordSize-bitSizeOfPackedToken));



   if (opCode==9)
    /************************************************
     *                                              *
     *         collect info for the packing         *
     *                                              *
     ***********************************************/
   {
     if ( packedHeader != NULL )
       /*******************************************************************
        *                                                                  *
        *  pack header is required, (X - Xmin) is used as packInt          *
        *                                                                  *
        *******************************************************************/
       {
             constructHeader(arrayOfUnsignedChar, minUnsignedInteger, maxUnsignedInteger);
       }
    else
      /*************************************************************
       *                                                           *
       *   pack header not required, X itself is used as packInt,  *
       *   determines bitSizeOfPackedToken, if not available       *
       *                                                           *
       ************************************************************/
      {
        if ( bitSizeOfPackedToken == -1 )
          {
            /**********************************************************
             *                                                        *
             *   obtain minimum, maximun, span                        *
             *                                                        *
             *********************************************************/
            if ( opCode == 9 )
              {
                /* unsigned integer number */
                maxSpan = arrayOfUnsignedChar[0];

                for(i=stride; i < intCount*stride ; i+=stride)
                  {
                    maxSpan |= arrayOfUnsignedChar[i];
                  };
              };

            /************************************************************
             *                                                          *
             *           derive bitSizeOfPackedToken                    *
             *                                                          *
             ***********************************************************/
            bitSizeOfPackedToken = 0;
            while ( maxSpan != 0 )
              {
                maxSpan = maxSpan >> 1;
                bitSizeOfPackedToken++;
              };
            if ( opCode == 3 )
              {/* accomodate the signed bit */
                bitSizeOfPackedToken++;
              };
            cleanupMask = ((uint32_t)(~0)>>(wordSize-bitSizeOfPackedToken));
          };/* if */



      };/* else */
   }
   else/* opCode == 10 */
    /************************************************
     *                                              *
     *         collect info for the unpacking       *
     *                                              *
     ***********************************************/
     {
       if ( packHeader != NULL )
         {
           theHeader     = (integer_header *)packedHeader;
           tokenSize     = theHeader->numOfBitsPerToken;
           ShiftIntended = theHeader->SHIFT;
           intCount      = theHeader->numOfPackedToken;
           minSigned     = theHeader->minValue;
           minUnsigned   = theHeader->minValue;

         }
       else
         {
           tokenSize     = bitSizeOfPackedToken;
           ShiftIntended = 0;
           intCount      = intCount;
           minSigned     = minSignedInteger;
           minUnsigned   = minUnsignedInteger;
         };
     };



   /**********************************************
    *                                            *
    *  compute signed int adjustment,            *
    * since 1,2,3,4 without header all needs it  *
    *                                            *
    *********************************************/
   positiveMask = ( opCode < 11 ) ? 0 : ( 1 << ( bitSizeOfPackedToken - 1 ));


   /***********************************************
    *                                             *
    *   pack                                      *
    *                                             *
    **********************************************/
   if ( opCode == 9 )
     {
       Pack(arrayOfUnsignedChar, 0);
     }
   /***********************************************
    *                                             *
    *   unpack                                    *
    *                                             *
    **********************************************/
   else if ( opCode == 10 )
     {
       Unpack(arrayOfUnsignedChar, ShiftIntended, tokenSize,
              0, intCount);
     }
   else
     {
       printf("\n opCode:%d is not defined \n", opCode);
       return 0;
     };/* if */

  return intCount;  /* unused, function must return something */


} /* end compact_char */

