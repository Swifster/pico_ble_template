#pragma once

#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float b0, b1, b2;
    float a1, a2;
    float x1, x2;
    float y1, y2;
} BandPass2;

// f0 = center frequency (Hz)
// Q  = quality (controls bandwidth)
// fs = sampling frequency (Hz)
void bp2_init(BandPass2 *bp, float f0, float Q, float fs);

static inline float bp2_filt(BandPass2 *bp, float xn)
{
    float y = bp->b0*xn + bp->b1*bp->x1 + bp->b2*bp->x2
                        - bp->a1*bp->y1 - bp->a2*bp->y2;

    bp->x2 = bp->x1;
    bp->x1 = xn;

    bp->y2 = bp->y1;
    bp->y1 = y;

    return y;
}

#ifdef __cplusplus
}
#endif
