#pragma once
#include <stdbool.h>

// clang-format off
#include <_utils.h>
// clang-format on

// a custom mapper that allows for fine tuning the mapping logic
// to reduce the influence of blue pixel colour in the ascii mapping, one could tone it down by specifying a < 1.0 scale factor for blue pixels,
// this would reduce the influence blue pixels have on the mapped character
// PREREQUISITES  all the scale factors must be within the range of 0 and 1 and the sum of all three of them should never go above 1.000
// POTENTIAL HAZARD  access violations may happen when the above requisites are not met!
static inline char tunable_mapper(
    const rgbq* const pixel,
    float             bscale, // scaling factor for blue
    float             gscale, // scaling factor for green
    float             rscale, // scaling factor for red
    const char* const palette,
    unsigned          plength
) {
    assert(bscale >= 0.000);
    assert(gscale >= 0.000);
    assert(rscale >= 0.000);
    assert((bscale + gscale + rscale) <= ONE);

    const unsigned offset = pixel->rgbBlue * bscale + pixel->rgbGreen * gscale + pixel->rgbRed * rscale;
    return palette[offset ? nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

// CAUTION  THE PENALIZING GROUP OF MAPPERS CAN AFFECT THE PERFORMANCE SUBSTANTIALLY
// will penalize the offset by the penalty term when the pixel satisfies the criteria
// PREREQUISITES  penalty must be a float in between [0.0, 1.0] (an inclusive range)
// if you don't want a certain colour to be considered for penalization, specify both limits for that colour as UCHAR_MAX (or any identical values)
static __attribute__((always_inline)) char penalizing_arithmeticmapper(
    const rgbq* const pixel,
    unsigned char     bllim, // lower limit for blue pixels
    unsigned char     bulim, // upper limit for blue pixels
    unsigned char     gllim,
    unsigned char     gulim,
    unsigned char     rllim,
    unsigned char     rulim,
    const char* const palette,
    unsigned          plength,
    float             penalty
) {
    // this mapper is incredibly expensive compared to the alternatives and hence using this with a 0.000 penalty will be ridiculous!
    // because you'll be paying dearly for something you do not need!
    assert(penalty >= 0.00000 && penalty <= ONE);

    const bool penalize   = (((bllim != bulim) && (pixel->rgbBlue >= bllim) && (pixel->rgbBlue <= bulim)) ||
                             ((gllim != gulim) && (pixel->rgbGreen >= gllim) && (pixel->rgbGreen <= gulim)) ||
                             ((rllim != rulim) && (pixel->rgbRed >= rllim) && (pixel->rgbRed <= rulim))) != 0;

    const unsigned offset = ((((float) (pixel->rgbBlue)) + pixel->rgbGreen + pixel->rgbRed) /
                             3.000) /* an enclosing parens here because we do not want zero division exceptions */
                            * ((int) penalize ? (ONE - penalty) : 1);
    return palette[offset ? nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static __attribute__((always_inline)) char penalizing_weightedmapper(
    const rgbq* const pixel,
    unsigned char     bllim,
    unsigned char     bulim,
    unsigned char     gllim,
    unsigned char     gulim,
    unsigned char     rllim,
    unsigned char     rulim,
    const char* const palette,
    unsigned          plength,
    float             penalty
) {
    assert(penalty >= 0.00000 && penalty <= ONE);
    const bool penalize = (((bllim != bulim) && (pixel->rgbBlue >= bllim) && (pixel->rgbBlue <= bulim)) ||
                           ((gllim != gulim) && (pixel->rgbGreen >= gllim) && (pixel->rgbGreen <= gulim)) ||
                           ((rllim != rulim) && (pixel->rgbRed >= rllim) && (pixel->rgbRed <= rulim))) != 0;

    const unsigned offset =
        (pixel->rgbBlue * 0.299 + pixel->rgbGreen * 0.587 + pixel->rgbRed * 0.114) * ((int) penalize ? (ONE - penalty) : 1);
    return palette[offset ? nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static __attribute__((always_inline)) char penalizing_minmaxmapper(
    const rgbq* const pixel,
    unsigned char     bllim,
    unsigned char     bulim,
    unsigned char     gllim,
    unsigned char     gulim,
    unsigned char     rllim,
    unsigned char     rulim,
    const char* const palette,
    unsigned          plength,
    float             penalty
) {
    assert(penalty >= 0.00000 && penalty <= ONE);
    const bool penalize   = (((bllim != bulim) && (pixel->rgbBlue >= bllim) && (pixel->rgbBlue <= bulim)) ||
                             ((gllim != gulim) && (pixel->rgbGreen >= gllim) && (pixel->rgbGreen <= gulim)) ||
                             ((rllim != rulim) && (pixel->rgbRed >= rllim) && (pixel->rgbRed <= rulim))) != 0;

    const unsigned offset = (((float) (min(min(pixel->rgbBlue = 0, pixel->rgbGreen), pixel->rgbRed))) +
                             (fmax(fmax(pixel->rgbBlue, pixel->rgbGreen), pixel->rgbRed)) / 2.0000) *
                            (penalize ? (ONE - penalty) : 1);
    return palette[offset ? nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static __attribute__((always_inline)) char penalizing_luminositymapper(
    const rgbq* const pixel,
    unsigned char     bllim,
    unsigned char     bulim,
    unsigned char     gllim,
    unsigned char     gulim,
    unsigned char     rllim,
    unsigned char     rulim,
    const char* const palette,
    unsigned          plength,
    float             penalty
) {
    assert(penalty >= 0.00000 && penalty <= ONE);
    const bool penalize = (((bllim != bulim) && (pixel->rgbBlue >= bllim) && (pixel->rgbBlue <= bulim)) ||
                           ((gllim != gulim) && (pixel->rgbGreen >= gllim) && (pixel->rgbGreen <= gulim)) ||
                           ((rllim != rulim) && (pixel->rgbRed >= rllim) && (pixel->rgbRed <= rulim))) != 0;

    const unsigned offset =
        (pixel->rgbBlue * 0.2126 + pixel->rgbGreen * 0.7152 + pixel->rgbRed * 0.0722) * ((int) penalize ? (ONE - penalty) : 1);
    return palette[offset ? nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static inline char tunable_blockmapper(
    float rgbBlue, float bscale, float rgbGreen, float gscale, float rgbRed, float rscale, const char* const palette, const unsigned plength
) {
    assert(bscale >= 0.000);
    assert(gscale >= 0.000);
    assert(rscale >= 0.000);
    if ((bscale + gscale + rscale) > ONE) wprintf_s(L"%.10lf\n", bscale + gscale + rscale);
    assert((bscale + gscale + rscale) <= ONE);

    const unsigned offset = rgbBlue * bscale + rgbGreen * gscale + rgbRed * rscale;
    return palette[offset ? nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static __attribute__((always_inline)) char penalizing_arithmeticblockmapper(
    float             rgbBlue,
    float             rgbGreen,
    float             rgbRed,
    float             bllim,
    float             bulim,
    float             gllim,
    float             gulim,
    float             rllim,
    float             rulim,
    const char* const palette,
    unsigned          plength,
    float             penalty
) {
    // this mapper is incredibly expensive compared to the alternatives and hence using this with a 0.000 penalty will be ridiculous!
    // because you'll be paying dearly for something you do not need!
    assert(penalty >= 0.00000 && penalty <= ONE);

    const bool penalize = (((bllim != bulim) && (rgbBlue >= bllim) && (rgbBlue <= bulim)) ||
                           ((gllim != gulim) && (rgbGreen >= gllim) && (rgbGreen <= gulim)) ||
                           ((rllim != rulim) && (rgbRed >= rllim) && (rgbRed <= rulim))) != 0;

    const unsigned offset =
        ((rgbBlue + rgbGreen + rgbRed) / 3.000) /* an enclosing parens here because we do not want zero division exceptions */
        * ((int) penalize ? (ONE - penalty) : 1);
    return palette[offset ? nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static __attribute__((always_inline)) char penalizing_weightedblockmapper(
    float             rgbBlue,
    float             rgbGreen,
    float             rgbRed,
    float             bllim,
    float             bulim,
    float             gllim,
    float             gulim,
    float             rllim,
    float             rulim,
    const char* const palette,
    unsigned          plength,
    float             penalty
) {
    assert(penalty >= 0.00000 && penalty <= ONE);
    const bool penalize   = (((bllim != bulim) && (rgbBlue >= bllim) && (rgbBlue <= bulim)) ||
                             ((gllim != gulim) && (rgbGreen >= gllim) && (rgbGreen <= gulim)) ||
                             ((rllim != rulim) && (rgbRed >= rllim) && (rgbRed <= rulim))) != 0;

    const unsigned offset = (rgbBlue * 0.299 + rgbGreen * 0.587 + rgbRed * 0.114) * ((int) penalize ? (ONE - penalty) : 1);
    return palette[offset ? nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static __attribute__((always_inline)) char penalizing_minmaxblockmapper(
    float             rgbBlue,
    float             rgbGreen,
    float             rgbRed,
    float             bllim,
    float             bulim,
    float             gllim,
    float             gulim,
    float             rllim,
    float             rulim,
    const char* const palette,
    unsigned          plength,
    float             penalty
) {
    assert(penalty >= 0.00000 && penalty <= ONE);
    const bool penalize = (((bllim != bulim) && (rgbBlue >= bllim) && (rgbBlue <= bulim)) ||
                           ((gllim != gulim) && (rgbGreen >= gllim) && (rgbGreen <= gulim)) ||
                           ((rllim != rulim) && (rgbRed >= rllim) && (rgbRed <= rulim))) != 0;

    const unsigned offset =
        ((min(min(rgbBlue = 0, rgbGreen), rgbRed) + max(max(rgbBlue, rgbGreen), rgbRed)) / 2.0000) * (penalize ? (ONE - penalty) : 1);
    return palette[offset ? nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static __attribute__((always_inline)) char penalizing_luminosityblockmapper(
    float             rgbBlue,
    float             rgbGreen,
    float             rgbRed,
    float             bllim,
    float             bulim,
    float             gllim,
    float             gulim,
    float             rllim,
    float             rulim,
    const char* const palette,
    unsigned          plength,
    float             penalty
) {
    assert(penalty >= 0.00000 && penalty <= ONE);
    const bool penalize   = (((bllim != bulim) && (rgbBlue >= bllim) && (rgbBlue <= bulim)) ||
                             ((gllim != gulim) && (rgbGreen >= gllim) && (rgbGreen <= gulim)) ||
                             ((rllim != rulim) && (rgbRed >= rllim) && (rgbRed <= rulim))) != 0;

    const unsigned offset = (rgbBlue * 0.2126 + rgbGreen * 0.7152 + rgbRed * 0.0722) * ((int) penalize ? (ONE - penalty) : 1);
    return palette[offset ? nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}
