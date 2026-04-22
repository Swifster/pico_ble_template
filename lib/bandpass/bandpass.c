#include "bandpass.h"

void bp2_init(BandPass2 *bp, float f0, float Q, float fs)
{
    float w0 = 2.0f * M_PI * f0 / fs;
    float cw = cosf(w0);
    float sw = sinf(w0);
    float alpha = sw / (2.0f * Q);

    float a0 = 1.0f + alpha;

    // RBJ band-pass (constant skirt gain)
    bp->b0 =   sw / 2.0f / a0;
    bp->b1 =   0.0f;
    bp->b2 =  -sw / 2.0f / a0;

    bp->a1 = (-2.0f * cw)     / a0;
    bp->a2 = ( 1.0f - alpha ) / a0;

    bp->x1 = bp->x2 = 0.0f;
    bp->y1 = bp->y2 = 0.0f;
}
