#include "gsl_math.h"
#include "gsl_cblas.h"
#include "cblas.h"
#include "error_cblas_l2.h"

void
cblas_zhpr (const enum CBLAS_ORDER order, const enum CBLAS_UPLO Uplo,
            const int N, const double alpha, const void *X, const int incX,
            void *Ap)
{
#define BASE double
#include "source_hpr.h"
#undef BASE
}
