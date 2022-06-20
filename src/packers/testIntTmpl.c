#include <stdio.h>
#include <stdint.h>

#define PASTE(a,b) a##b
#define CAT(a,b) PASTE(a,b)

#define TYPE int32_t
#include "compact_integer.tmplc"
#undef TYPE
#define TYPE float
#include "compact_integer.tmplc"
#undef TYPE

#define NB_ELEMS 5

int main() {
    const int32_t iArr[NB_ELEMS] = {42, 28, 14, 56, 2};
    int32_t iMin;
    int32_t iMax;
    findMinMax_int32_t(iArr, NB_ELEMS, 1, &iMin, &iMax);
    printf("int32_t - Min = %d, Max = %d\n", iMin, iMax);

    const float fArr[NB_ELEMS] = {42.0, 28.8, 14.4, 56.6, 2.2};
    float fMin;
    float fMax;
    findMinMax_float(fArr, NB_ELEMS, 1, &fMin, &fMax);
    printf("float - Min = %f, Max = %f\n", fMin, fMax);

    return 0;
}
