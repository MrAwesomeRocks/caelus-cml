#include "gsl_math.h"
#include "gsl_cblas.h"
#include "cblas.h"
#include "error_cblas_l3.h"

void
cblas_strsm (const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
             const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
             const enum CBLAS_DIAG Diag, const int M, const int N,
             const float alpha, const float *A, const int lda, float *B,
             const int ldb)
{
#define BASE float
#include "source_trsm_r.h"
#undef BASE
}
