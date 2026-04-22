#include <math.h>
#include "lowpass.h"

void lowpass_init(LowPassFilter *lp, float cutoff, float signal_quality, float sampling_frequency)
{
    lp->x1 = lp->x2 = 0.0f;
    lp->y1 = lp->y2 = 0.0f;

    float w0 = 2.0f * 3.14159265359f * cutoff / sampling_frequency;
    float cw = cosf(w0);
    float sw = sinf(w0);
    float alpha = sw / (2.0f * signal_quality);

    float a0 = 1.0f + alpha;

    lp->b0 = 0.5f * (1.0f - cw) / a0;
    lp->b1 =       (1.0f - cw) / a0;
    lp->b2 = 0.5f * (1.0f - cw) / a0;
    lp->a1 = (-2.0f * cw) / a0;
    lp->a2 = (1.0f - alpha) / a0;
}