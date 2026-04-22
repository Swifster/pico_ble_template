#pragma once

#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    float b0, b1, b2;
    float a1, a2;
    float x1, x2;
    float y1, y2;
} HighPassFilter;

// cutoff in Hz, Q=0.707 for Butterworth, fs = sample rate
void highpass_init(HighPassFilter *hp, float cutoff, float Q, float fs);

// Filter one sample
static inline float highpass_filt(HighPassFilter *hp, float xn) {
    float y = hp->b0*xn + hp->b1*hp->x1 + hp->b2*hp->x2
                        - hp->a1*hp->y1 - hp->a2*hp->y2;

    hp->x2 = hp->x1;
    hp->x1 = xn;

    hp->y2 = hp->y1;
    hp->y1 = y;

    return y;
}
