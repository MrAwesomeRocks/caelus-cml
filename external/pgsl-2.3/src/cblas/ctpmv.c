#include "gsl_math.h"
#include "gsl_cblas.h"
#include "cblas.h"
#include "error_cblas_l2.h"

void
cblas_ctpmv (const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
             const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_DIAG Diag,
             const int N, const void *Ap, void *X, const int incX)
{
#define BASE float
#include "source_tpmv_c.h"
#undef BASE
}
