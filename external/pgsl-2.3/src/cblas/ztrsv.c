#include "gsl_math.h"
#include "gsl_cblas.h"
#include "cblas.h"
#include "error_cblas_l2.h"

#include "hypot.c"

void
cblas_ztrsv (const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
             const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
             const int N, const void *A, const int lda, void *X,
             const int incX)
{
#define BASE double
#include "source_trsv_c.h"
#undef BASE
}
