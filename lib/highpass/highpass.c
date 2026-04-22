#include "highpass.h"

void highpass_init(HighPassFilter *hp, float cutoff, float Q, float fs)
{
    float w0 = 2.0f * M_PI * cutoff / fs;
    float cw = cosf(w0);
    float sw = sinf(w0);
    float alpha = sw / (2.0f * Q);

    float a0 = 1.0f + alpha;

    hp->b0 =  (1.0f + cw) * 0.5f / a0;
    hp->b1 = -(1.0f + cw)       / a0;
    hp->b2 =  (1.0f + cw) * 0.5f / a0;
    hp->a1 = (-2.0f * cw)       / a0;
    hp->a2 = (1.0f - alpha)     / a0;

    hp->x1 = 0.0f;
    hp->x2 = 0.0f;
    hp->y1 = 0.0f;
    hp->y2 = 0.0f;
}
