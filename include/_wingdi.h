#pragma once

#include <assert.h>

// putting these here as this header is universally included in all the sources
#ifdef __TEST__
    #define _testing(...) (__VA_ARGS__)
    #if !defined(_DEBUG) || !defined(DEBUG) // when testing define _DEBUG & DEBUG for assertions to work!
        #define _DEBUG
        #define DEBUG
    #endif
#else
    #define _testing(...)
#endif // _DEBUG

// a header providing proxy definitions for some essential Win32 structs related to bitmaps

// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbquad
typedef struct {
        unsigned char b; // blue
        unsigned char g; // green
        unsigned char r; // red
        unsigned char _; // reserved
} rgbq;                  // rgb quad pixel
static_assert(sizeof(rgbq) == 4);

// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
typedef struct {
        unsigned       size;
        int            width;
        int            height;
        unsigned short planes;
        unsigned short nbits;
        unsigned       compression;
        unsigned       imagesize;
        int            ppm_x;
        int            ppm_y;
        unsigned       used_clrs;
        unsigned       imp_clrs;
} infhead; // bitmap info header
static_assert(sizeof(infhead) == 40);

// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
#pragma pack(push, 2)
typedef struct {
        unsigned short type;
        unsigned       size;
        unsigned short _reserved_0;
        unsigned short _reserved_1;
        unsigned       offbits;
} fhead; // bitmap file header
#pragma pack(pop)
static_assert(sizeof(fhead) == 14);
