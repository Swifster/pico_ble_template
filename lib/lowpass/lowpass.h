#pragma once

#include <stdint.h>

typedef struct {
    float b0, b1, b2;
    float a1, a2;
    float x1, x2;
    float y1, y2;
} LowPassFilter;


// Common Q values this is quality factor
#define Q_BUTTERWORTH     .707f   // maximally flat, smooth, default choice
#define Q_CRITICALLY_DAMPED 1.0f  // sharper rolloff, slight resonance
#define Q_UNDERDAMPED     1.414f  // peaks a bit, faster, more "ringy"
#define Q_OVERDAMPED      .5f     // slower response, extra smoothing


// cutoff in Hz, Q = 0.707 for Butterworth, fs = sample rate
void lowpass_init(LowPassFilter *lp, float cutoff, float signal_quality, float sampling_frequency);

// Filter one sample
static inline float lowpass_filt(LowPassFilter *lp, float xn) {
    float y = lp->b0*xn + lp->b1*lp->x1 + lp->b2*lp->x2
                        - lp->a1*lp->y1 - lp->a2*lp->y2;

    lp->x2 = lp->x1;
    lp->x1 = xn;

    lp->y2 = lp->y1;
    lp->y1 = y;

    return y;
}