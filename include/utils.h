#pragma once

#define ONE 1.000000000

#include <limits.h>
#include <math.h>

// clang-format off
#include <_wingdi.h>
// clang-format on

#ifdef __VERBOSE_OUTPUTS
    #define __printf_verbose(...) fprintf(stderr, __VA_ARGS__)
    #define _verbose(...)         (__VA_ARGS__)
#else
    #define __printf_verbose(...)
    #define _verbose(...)
#endif // _DEBUG

//-----------------------------------------------
// characters in ascending order of luminance
//-----------------------------------------------

static const char PALETTE_MINIMAL[]  = { '_', '.', ',', '-', '=', '+', ':', ';', 'c', 'b', 'a', '!', '?', '1',
                                         '2', '3', '4', '5', '6', '7', '8', '9', '$', 'W', '#', '@', 'N' };

static const char PALETTE_BASE[]     = { ' ', '.', '-', ',', ':', '+', '~', ';', '(', '%', 'x', '1', '*', 'n', 'u',
                                         'T', '3', 'J', '5', '$', 'S', '4', 'F', 'P', 'G', 'O', 'V', 'X', 'E', 'Z',
                                         '8', 'A', 'U', 'D', 'H', 'K', 'W', '@', 'B', 'Q', '#', '0', 'M', 'N' };

static const char PALETTE_EXTENDED[] = { ' ', '.', '\'', '`', '^', '"', ',', ':', ';', 'I',  'l',  '!', 'i', '>', '<', '~', '+', '_',
                                         '-', '?', ']',  '[', '}', '{', '1', ')', '(', '|',  '\\', '/', 't', 'f', 'j', 'r', 'x', 'n',
                                         'u', 'v', 'c',  'z', 'X', 'Y', 'U', 'J', 'C', '\'', 'Q',  '0', 'O', 'Z', 'm', 'w', 'q', 'p',
                                         'd', 'b', 'k',  'h', 'a', 'o', '*', '#', 'M', 'W',  '&',  '8', '%', 'B', '@', '$' };

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// transformers that map an RGB pixel to a representative ascii character, using the provided palette //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// one concern about these mappers is that the logic (offset / (float) UCHAR_MAX) could be any value between 0.0000 and 1.000
// because offset can be anywhere between 0 and 255!
// when it comes to mapping this pixel value to a character, multiplication by palette length can yield very small values, even for the longest palette
// the longest palette we have is the PALETTE_EXTENDED, which is 70 characters long
// consider when offset is non-zero but still very small, e.g. offset = 1,
// ((offset / (float) (UCHAR_MAX)) evaluates to 1 / 255.0 = 0.00392156862745098
// that multiplied by palette length, 0.00392156862745098 * 70 = 0.274509803921569
// when we cast this to an unsigned, we get 0 due to truncation,
// subtract 1 from this, we end up with -1, a guranteed ticket to access violation!
// this logic return palette[offset ? (unsigned) ((offset / (float) (UCHAR_MAX)) * plength) - 1 : 0]; is dangerous!
// ceiling the subexpression ((offset / (float) (UCHAR_MAX)) * plength) can help here!

// but ceil() will send a call to a ucrt dll every time we use the mapper in a loop, againt will make the performance phenomenally bad
// we really do not need a ceilf() call, all we need is a function that can return 1.000 when the input is between 0.00000 and 1.0000

//--------------------------------
// PIXEL TO CHARACTER MAPPERS
//--------------------------------

static inline unsigned __attribute__((always_inline)) _nudge(float _value) {
    // taking it for granted that the input will never be a negative value
    return _value < 1.000000 ? 1 : (unsigned) _value;
}

static inline char __attribute__((always_inline)) arithmetic(const rgbq* const pixel, const char* const palette, unsigned plength) {
    const unsigned offset = ((double) pixel->b + pixel->g + pixel->r) / 3.000; // can range from 0 to 255
    // hence, offset / (float)(UCHAR_MAX) can range from 0.0 to 1.0
    return palette[offset ? _nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static inline char __attribute__((always_inline)) weighted(const rgbq* const pixel, const char* const palette, unsigned plength) {
    const unsigned offset = pixel->b * 0.299 + pixel->g * 0.587 + pixel->r * 0.114;
    return palette[offset ? _nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static inline char __attribute__((always_inline)) minmax(const rgbq* const pixel, const char* const palette, unsigned plength) {
    const unsigned offset = (unsigned) ((fmin(fmin(pixel->b, pixel->g), pixel->r) + fmax(fmax(pixel->b, pixel->g), pixel->r)) / 2.0000);
    return palette[offset ? _nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static inline char __attribute__((always_inline)) luminosity(const rgbq* const pixel, const char* const palette, unsigned plength) {
    const unsigned offset = pixel->b * 0.2126 + pixel->g * 0.7152 + pixel->r * 0.0722;
    return palette[offset ? _nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

//---------------------------------------
// PIXEL BLOCKS TO CHARACTER MAPPERS
//---------------------------------------

static inline char __attribute__((always_inline)) arithmetic_blockmapper(
    float b, float g, float r, const char* const palette, unsigned plength
) {
    const unsigned offset = (b + g + r) / 3.000; // can range from 0 to 255
    // hence, offset / (float)(UCHAR_MAX) can range from 0.0 to 1.0
    return palette[offset ? _nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static inline char __attribute__((always_inline)) weighted_blockmapper(
    float b, float g, float r, const char* const palette, unsigned plength
) {
    const unsigned offset = b * 0.299 + g * 0.587 + r * 0.114;
    return palette[offset ? _nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static inline char __attribute__((always_inline)) minmax_blockmapper(
    float b, float g, float r, const char* const palette, unsigned plength
) {
    const unsigned offset = (fminf(fminf(b, g), r) + fmaxf(fmaxf(b, g), r)) / 2.0000;
    return palette[offset ? _nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}

static inline char __attribute__((always_inline)) luminosity_blockmapper(
    float b, float g, float r, const char* const palette, unsigned plength
) {
    const unsigned offset = b * 0.2126 + g * 0.7152 + r * 0.0722;
    return palette[offset ? _nudge(offset / (float) (UCHAR_MAX) *plength) - 1 : 0];
}
