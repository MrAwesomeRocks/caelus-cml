#include "gsl_math.h"
#include "gsl_cblas.h"
#include "cblas.h"
#include "error_cblas_l2.h"

void
cblas_strsv (const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
             const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
             const int N, const float *A, const int lda, float *X,
             const int incX)
{
#define BASE float
#include "source_trsv_r.h"
#undef BASE
}
