#ifndef _RNBO_CMATH_H
#define _RNBO_CMATH_H

#ifdef RNBO_NOSTDLIB
extern "C" {

double         acos (double x);
float          acosf(float x);
long double    acosl(long double x);

double         asin (double x);
float          asinf(float x);
long double    asinl(long double x);

double         atan (double x);
float          atanf(float x);
long double    atanl(long double x);

double         atan2 (double y, double x);
float          atan2f(float y, float x);
long double    atan2l(long double y, long double x);

double         ceil (double x);
float          ceilf(float x);
long double    ceill(long double x);

double         cos (double x);
float          cosf(float x);
long double    cosl(long double x);

double         cosh (double x);
float          coshf(float x);
long double    coshl(long double x);

double         exp (double x);
float          expf(float x);
long double    expl(long double x);

float          exp2f(float);
double         exp2(double);
long double    exp2l(long double);

int            abs(int x);
double         fabs (double x);
float          fabsf(float x);
long double    fabsl(long double x);

double         floor (double x);
float          floorf(float x);
long double    floorl(long double x);

float          fmaxf(float x, float y);
double         fmax(double x, double y);
long double    fmaxl(long double x, long double y);

double         fmod (double x, double y);
float          fmodf(float x, float y);
long double    fmodl(long double x, long double y);

double         frexp (double value, int* exp);
float          frexpf(float value, int* exp);
long double    frexpl(long double value, int* exp);

double         ldexp (double value, int exp);
float          ldexpf(float value, int exp);
long double    ldexpl(long double value, int exp);

double         log (double x);
float          logf(float x);
long double    logl(long double x);

double         log2 (double x);
float          log2f(float x);
long double    log2l(long double x);

double         log10 (double x);
float          log10f(float x);
long double    log10l(long double x);

double         log1p(double x);
float          log1pf(float x);
long double    log1pl(long double x);

double         modf (double value, double* iptr);
float          modff(float value, float* iptr);
long double    modfl(long double value, long double* iptr);

double         pow (double x, double y);
float          powf(float x, float y);
long double    powl(long double x, long double y);

double         sin (double x);
float          sinf(float x);
long double    sinl(long double x);

double         sinh (double x);
float          sinhf(float x);
long double    sinhl(long double x);

double         sqrt (double x);
float          sqrtf(float x);
long double    sqrtl(long double x);

double         cbrt(double x);
float          cbrtf(float x);
long double    cbrtl(long double x);

double         expm1(double x);
float          expm1f(float x);
long double    expm1l(long double x);

double         tan (double x);
float          tanf(float x);
long double    tanl(long double x);

double         tanh (double x);
float          tanhf(float x);
long double    tanhl(long double x);

double         trunc (double x);
float          truncf(float x);
long double    truncl(long double x);

double         asinh (double x);
float          asinhf(float x);
long double    asinhl(long double x);

double         acosh (double x);
float          acoshf(float x);
long double    acoshl(long double x);

double         atanh (double x);
float          atanhf(float x);
long double    atanhl(long double x);

}
#endif // RNBO_NOSTDLIB

#endif // #ifndef _RNBO_CMATH_H
