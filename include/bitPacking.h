#if !defined (_BITPACKING_H_)
#define _BITPACKING_H_

#include <stdint.h>

#define FLOAT_PACK 1
#define FLOAT_UNPACK 2

#if defined(Little_Endian)
typedef struct {
    uint32_t mantis3:29, mantis2:3, mantis1:20, expo:11, sign:1;
} IEEE_FLOAT_DOUBLE;

typedef struct {
    uint32_t mantis:23, expo:8 , sign:1;
} IEEE_FLOAT;
#else
typedef struct {
    uint32_t sign:1, expo:11 , mantis1:20,mantis2:3,mantis3:29;
} IEEE_FLOAT_DOUBLE;

typedef struct {
    uint32_t sign:1, expo:8 , mantis:23;
} IEEE_FLOAT;
#endif

typedef struct {
    uint32_t sign:1, expo:7 , mantis:24;
} IBM_FLOAT;

typedef struct {
    uint32_t sign:1, expo:7 , mantis1:24 , mantis2:32;
} IBM_FLOAT_DOUBLE;

typedef union {
    float X;
    double XD;
    uint32_t U;
    IEEE_FLOAT_DOUBLE MD;
    IEEE_FLOAT M;
    IBM_FLOAT I;
    IBM_FLOAT_DOUBLE ID;
} ALL_FLOAT;


//! Extract a token from a steam of packed tokens
inline void extract(
    //! [out] Extracted token
    uint32_t * const unpackedToken,
    //! [in,out] Pointer to the token to extract
    const uint32_t ** packedWordPtr,
    //! [in] Word size in bits
    const int wordSize,
    //! [in] Packed token size in bits
    const int bitSizeOfPackedToken,
    //! [in,out] Word holding the desired token
    uint32_t * packedWord,
    //! [in,out] Number of packed bits remaining packedWord
    int * const bitPackInWord
) {
    if ( *bitPackInWord >= bitSizeOfPackedToken ) {
        *unpackedToken = (packedWord >> ( wordSize - bitSizeOfPackedToken ) );
        packedWord <<= bitSizeOfPackedToken;
        *bitPackInWord -= bitSizeOfPackedToken;
    } else {
        *unpackedToken = (packedWord >> ( wordSize - bitSizeOfPackedToken ));
        (*packedWordPtr)++;
        packedWord = **packedWordPtr;
        *unpackedToken |= ( packedWord >> ( wordSize - (bitSizeOfPackedToken - *bitPackInWord)));
        packedWord <<= ( bitSizeOfPackedToken - *bitPackInWord );
        bitPackInWord = wordSize - (bitSizeOfPackedToken - *bitPackInWord);
    }
    if ( bitPackInWord == 0 ) {
        // Special case
        (*packedWordPtr)++;
        packedWord = *packedWordPtr;
        *bitPackInWord = wordSize;
    }
}


//! Discard unwanted bits
inline void discard(
    //! [in,out] Pointer to the token to extract
    uint32_t ** packedWordPtr,
    //! [in] Word size in bits
    const int wordSize,
    //! [in] Bits to discard
    const int discardBit,
    //! [out] Word holding the desired token
    uint32_t * const packedWord,
    //! [in,out] Number of packed bits remaining packedWord
    int * const bitPackInWord
) {
    if ( *bitPackInWord > discardBit ) {
        *packedWord <<= discardBit;
        *bitPackInWord -= discardBit;
    } else {
        (*packedWordPtr)++;
        *packedWord = **packedWordPtr;
        *packedWord <<= (discardBit - *bitPackInWord);
        *bitPackInWord = wordSize - (discardBit - *bitPackInWord);
    }
    if ( *bitPackInWord == 0 ) {
        // Special case
        (*packedWordPtr)++;
        *packedWord = **packedWordPtr;
        *bitPackInWord = wordSize;
    }
}


//! Packe a token in a steam of packed tokens
inline void stuff(
    //! [in] Token to pack
    const unit32_t token,
    //! [out] Pointer to the word position available for packing
    uint32_t ** availableWordPtr,
    //! [in] Word size in bits
    const int wordSize,
    //! [in] Size of the token in bits
    const int bitSizeOfPackedToken,
    //! [in,out] Word to hold the token
    uint32_t * const lastWordShifted,
    //! [in,out] Number of bits available for packing in the last word
    uint32_t * const spaceInLastWord
) {
    if ( *spaceInLastWord >= bitSizeOfPackedToken ) {
        // integer token fits into space left
        *lastWordShifted = ( *lastWordShifted << bitSizeOfPackedToken ) | token;
        *spaceInLastWord = *spaceInLastWord - bitSizeOfPackedToken;
    } else {
        // integer token can not fit into space left
        **availableWordPtr = ((*lastWordShifted << *spaceInLastWord) | ( token >> ( bitSizeOfPackedToken - *spaceInLastWord)));
        *lastWordShifted = token & ( -1 >> ( wordSize - ( bitSizeOfPackedToken - *spaceInLastWord)));
        *spaceInLastWord = wordSize - ( bitSizeOfPackedToken - *spaceInLastWord );
        (*availableWordPtr)++;
        lastSlot++;
    }
}

#endif
